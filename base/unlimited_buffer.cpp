#include <unistd.h>
#include <stdexcept>

#include "log.h"
#include "unlimited_buffer.h"

#define TOKEN_SIZE 65536

using namespace std;

UnlimitedBuffer::UnlimitedBuffer()
    : _size(0), _nwritten(0), _nread(0), _input_ended(0) {}

size_t UnlimitedBuffer::HasWrite(string&& data) {
    size_t len = data.size();
    // _stream_buffer.Append(BlockList(std::move( const_cast<string&>(data) )));
    _stream_buffer.Append(BlockList(std::forward<string>( data )));
    _size += len;
    _nwritten += len;
    return len;
}


 // 为了适配muduo
void UnlimitedBuffer::Append(const char *data, size_t len)
{
    size_t nwrote = HasWrite( std::string( data,len ) );
    if( nwrote != len ) {
        CRITICAL( "UnlimitedBuffer::append" );
    }
}

std::string UnlimitedBuffer::HasRead(const size_t len) {
    const auto ret = PeekOutput(len);
    PopOutput(len);
    return ret;
}


std::string UnlimitedBuffer::PeekOutput(const size_t len) const { return _stream_buffer.Concatenate(min(len, _size)); }

// 要么取len个，要么取完
void UnlimitedBuffer::PopOutput(const size_t len) {
    size_t l = min(len, _size);
    _stream_buffer.RemovePrefix(l);
    _size -= l;
    _nread += l;
}



// void UnlimitedBuffer::EndInput() { _input_ended = 1; }

// bool UnlimitedBuffer::InputEnded() const { return _input_ended; }

// size_t UnlimitedBuffer::size() const { return _size; }

// bool UnlimitedBuffer::BufferEmpty() const { return _size == 0; }

// bool UnlimitedBuffer::eof() const { return InputEnded() && BufferEmpty(); }

// size_t UnlimitedBuffer::BytesWritten() const { return _nwritten; }

// size_t UnlimitedBuffer::ReadableBytes() const { return _nread; }

/* 从fd读取数据，Poller默认工作在LT模式下 */
ssize_t UnlimitedBuffer::ReadFd( int fd ) {
    char _token[TOKEN_SIZE]; 
    // 最多每次读TOKEN_SIZE大小的数据
    ssize_t len = read( fd, &_token, TOKEN_SIZE );
    if( len < 0 ) {
        // log 后面换
        CRITICAL( "Buffer::ReadFd" );
    } 
    if( len > 0 ) HasWrite( std::string( _token,len ) ); // buff每次写len个字符,这里有问题，api不对
    return len;
}

// Todo: 可能一次性没读完，如何判断每个iovecs的offset
ssize_t UnlimitedBuffer::WriteFd( int fd ) {
    vector<iovec> iovecs = _stream_buffer.as_iovecs();
    ssize_t len = writev( fd, &iovecs[0], iovecs.size() ); /* 不一定一次读完，要如何处理 */
    if( len < 0 ) {
        perror("Buffer::WriteFd");
        exit(1);
    }
    if( len != size() ) {
        CRITICAL( "UnlimitedBuffer::WriteFd Failed, writev()" );
    }
    PopOutput(len); // 已读len个字节 
    return len;
}