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
    size_t has_write(const std::string &data);
    // size_t remaining_capacity() const;
    // 设置写结束
    void end_input();
    void set_error() { _error = true; }
  
    // 取前len个字符
    std::string peek_output(const size_t len) const;
    void pop_output(const size_t len);

    std::string has_read(const size_t len) {
        const auto ret = peek_output(len);
        pop_output(len);
        return ret;
    }

    // 判断时候还能读
    bool input_ended() const;
    bool error() const { return _error; }
    size_t size() const;
    bool buffer_empty() const;
    bool eof() const;
    // 已写数
    size_t bytes_written() const;
    // 已读数
    size_t bytes_read() const;

    size_t read_fd( int fd ) ;
    size_t write_fd( int fd ) ;
};
