#include "unlimited_buffer.h"
#include <unistd.h>
#include <stdexcept>
#define TOKEN_SIZE 4048
using namespace std;


UnlimitedBuffer::UnlimitedBuffer()
    : _size(0), _nwritten(0), _nread(0), _input_ended(0) {}

size_t UnlimitedBuffer::HasWrite(const string &data) {
    size_t len = data.size();
    _stream_buffer.Append(BlockList(std::move( const_cast<string&>(data) )));
    _size += len;
    _nwritten += len;
    return len;
}

string UnlimitedBuffer::PeekOutput(const size_t len) const { return _stream_buffer.Concatenate(min(len, _size)); }

// 要么取len个，要么取完
void UnlimitedBuffer::PopOutput(const size_t len) {
    size_t l = min(len, _size);
    _stream_buffer.RemovePrefix(l);
    _size -= l;
    _nread += l;
}

void UnlimitedBuffer::EndInput() { _input_ended = 1; }

bool UnlimitedBuffer::InputEnded() const { return _input_ended; }

size_t UnlimitedBuffer::size() const { return _size; }

bool UnlimitedBuffer::BufferEmpty() const { return _size == 0; }

bool UnlimitedBuffer::eof() const { return InputEnded() && BufferEmpty(); }

size_t UnlimitedBuffer::BytesWritten() const { return _nwritten; }

size_t UnlimitedBuffer::BytesRead() const { return _nread; }

size_t UnlimitedBuffer::ReadFd( int fd ) {
    char _token[TOKEN_SIZE] ; 
    size_t len = read( fd, &_token, TOKEN_SIZE );
    if( len < 0 ) {
        // log 后面换
        perror( "Buffer::ReadFd" );
        exit(1);
    } 
    if( len > 0 ) HasWrite( {_token,len} ); // buff每次写len个字符
    return len;
}

size_t UnlimitedBuffer::WriteFd( int fd ) {
    vector<iovec> iovecs = _stream_buffer.as_iovecs();
    size_t len = writev( fd, &iovecs[0], iovecs.size() );
    if( len < 0 ) {
        perror("Buffer::WriteFd");
        exit(1);
    }
    PopOutput(len); // 已读len个字节 
    return len;
}