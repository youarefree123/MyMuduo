#pragma once

#include "storage_block.h"
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
    std::string HasRead(const size_t len);
    // size_t remaining_capacity() const;
    // 设置写结束
    void EndInput();
    void set_error() { _error = true; }
  
    // 取前len个字符
    std::string PeekOutput(const size_t len) const;
    void PopOutput(const size_t len);

    

    // 判断时候还能读
    bool InputEnded() const;
    bool error() const { return _error; }
    size_t size() const;
    bool BufferEmpty() const;
    bool eof() const;
    // 已写数
    size_t BytesWritten() const;
    // 已读数
    size_t BytesRead() const;

    size_t ReadFd( int fd) ;
    size_t WriteFd( int fd ) ;
};
