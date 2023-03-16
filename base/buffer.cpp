#include "buffer.h"
#include <unistd.h>
#include <stdexcept>

using namespace std;


Buffer::Buffer()
    : _size(0), _nwritten(0), _nread(0), _input_ended(0) {}

size_t Buffer::has_write(const string &data) {
    size_t len = data.size();
    _stream_buffer.append(BlockList(std::move( const_cast<string&>(data) )));
    _size += len;
    _nwritten += len;
    return len;
}

string Buffer::peek_output(const size_t len) const { return _stream_buffer.concatenate(min(len, _size)); }

// 要么取len个，要么取完
void Buffer::pop_output(const size_t len) {
    size_t l = min(len, _size);
    _stream_buffer.remove_prefix(l);
    _size -= l;
    _nread += l;
}

void Buffer::end_input() { _input_ended = 1; }

bool Buffer::input_ended() const { return _input_ended; }

size_t Buffer::size() const { return _size; }

bool Buffer::buffer_empty() const { return _size == 0; }

bool Buffer::eof() const { return input_ended() && buffer_empty(); }

size_t Buffer::bytes_written() const { return _nwritten; }

size_t Buffer::bytes_read() const { return _nread; }

size_t Buffer::read_fd( int fd ) {
  size_t len = read( fd, &_token, TOKEN_SIZE );
  if( len < 0 ) {
    // log 后面换
    perror( "Buffer::read_fd" );
    exit(1);
  } 
  if( len > 0 ) has_write( {_token,len} ); // buff每次写len个字符
  return len;
}

size_t Buffer::write_fd( int fd ) {
  vector<iovec> iovecs = _stream_buffer.as_iovecs();
  size_t len = writev( fd, &iovecs[0], iovecs.size() );
  if( len < 0 ) {
    perror("Buffer::write_fd");
    exit(1);
  }
  pop_output(len); // 已读len个字节 
  return len;
}