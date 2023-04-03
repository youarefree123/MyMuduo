#pragma once

#include "storage_block.h"
#define TOKEN_SIZE 4048

// 无限制字符流
class Buffer {
  private:
    BlockList _stream_buffer{};
    char _token[TOKEN_SIZE] ;
    size_t _size;
    size_t _nwritten;
    size_t _nread;
    bool _input_ended;
    bool _error{};  //!< Flag indicating that the stream suffered an error.

  public:
    Buffer();
    size_t HasWrite(const std::string &data);
    // size_t remaining_capacity() const;
    // 设置写结束
    void EndInput();
    void set_error() { _error = true; }
  
    // 取前len个字符
    std::string PeekOutput(const size_t len) const;
    void PopOutput(const size_t len);

    std::string HasRead(const size_t len) {
        const auto ret = PeekOutput(len);
        PopOutput(len);
        return ret;
    }

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

    size_t ReadFd( int fd ) ;
    size_t WriteFd( int fd ) ;
};
