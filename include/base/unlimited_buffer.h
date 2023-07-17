#pragma once

#include "base/storage_block.h"
// #define TOKEN_SIZE 4048



// 无限制字符流
class UnlimitedBuffer {
private:
    BlockList _stream_buffer{};
    size_t _size;
    size_t _nwritten;
    size_t _nread;
    bool _input_ended;
    bool _error{};  //!< Flag indicating that the stream suffered an error.

public:
    UnlimitedBuffer();
    size_t HasWrite(std::string&& data);

    // 为了适配muduo
    void Append(const char *data, size_t len);


    std::string HasRead(const size_t len);
    // size_t remaining_capacity() const;
    
  
    void set_error() { _error = true; }
  
    
    // 去除前len个或者所有的数据
    void PopOutput(const size_t len);
    // 为了兼容muduo
    void Retrieve( const size_t len ) { PopOutput(len); }
    std::string RetrieveAll() { return HasRead( ReadableBytes() ); }
    
    // 取前len个字符,不删除
    std::string PeekOutput(const size_t len) const;

    void EndInput() { _input_ended = true; }  // 设置写结束
    bool error() const { return _error; }
    size_t size() const { return _size; }
    bool BufferEmpty() const { return _size == 0; }
    bool eof() const { return InputEnded() && BufferEmpty(); }
    bool InputEnded() const { return _input_ended; } /* 判断是否还能读 */

    
    size_t BytesWritten() const { return _nwritten; } /* 已写数  */

    size_t ReadableBytes() const { return _size; } /*  可读数 */

    ssize_t ReadFd( int fd) ;
    ssize_t WriteFd( int fd, size_t len ) ;
};
