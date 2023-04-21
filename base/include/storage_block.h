#pragma once


// #include <algorithm>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <sys/uio.h>
#include <vector>

// string的封装类，为了实现0拷贝
class Block {
private:
    std::shared_ptr<std::string> _storage{}; 
    size_t _starting_offset{};

public:
    Block() = default;

    // 移动构造
    Block(std::string &&str) noexcept : _storage(std::make_shared<std::string>(std::move(str))) {}

    
    // 辅助函数，返回截断后的string_view
    std::string_view str() const {
        if (not _storage) {
            return {};
        }
        return {_storage->data() + _starting_offset, _storage->size() - _starting_offset};
    }

    // 重载的类型转换，返回的是截断string_view
    operator std::string_view() const { return str(); }
  
    // 目前没用到
    // uint8_t at(const size_t n) const { return str().at(n); }

    // 返回目前还有多少字符没有被读
    size_t size() const { return str().size(); }

    // 拷贝
    std::string copy() const { return std::string(str()); }

    //! \brief Discard the first `n` bytes of the string (does not require a copy or move)
    //! \note Doesn't free any memory until the whole string has been discarded in all copies of the Block.
    void RemovePrefix(const size_t n);
};


// 组织block的List，对外提供接口实现Append block和remove n个字符
class BlockList {
private:
    std::deque<Block> _blocks{};
public:
    BlockList() = default;

    BlockList(const Block& block) : _blocks{block} {}

    // BlockList 可以由block 构造出来
    // 所以下面的Append参数列表只需要有BlockList
    BlockList(std::string &&str) noexcept {
        Block blk{std::move(str)};
        Append(blk);
    }

    const std::deque<Block> &blocks() const { return _blocks; }
    void Append(const BlockList &other);
  
    // 可以直接将一个BlockList转成Block??
    operator Block() const;

    // 同样是移除前n个字符
    void RemovePrefix(size_t n);

    size_t size() const;

    // 这个强转成Block的时候会用到吧
    std::string Concatenate() const;

    // 只合并前n个字节
    std::string Concatenate(size_t n) const;

    // 转成iovece 数组用于分散读
    std::vector<iovec> as_iovecs() const;
};

// 用来分散读的视图
class BlockViewList {
    std::deque<std::string_view> _views{};

public:
  // 构造
    BlockViewList(const std::string &str) : BlockViewList(std::string_view(str)) {}

    BlockViewList(const char *s) : BlockViewList(std::string_view(s)) {}

    BlockViewList(const BlockList &blocks);

    BlockViewList(std::string_view str) { _views.push_back({const_cast<char *>(str.data()), str.size()}); }
    
    void RemovePrefix(size_t n);

    size_t size() const;

    // 转成iovece 数组用于分散读
    std::vector<iovec> as_iovecs() const;
};

