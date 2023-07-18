#include "base/storage_block.h"
#include <stdexcept>
using namespace std;

void Block::RemovePrefix(const size_t n) {
    if (n > str().size()) {
        throw out_of_range("Block::RemovePrefix");
    }
    _starting_offset += n;
    // 如果这个block 已经用完，将该指针置空,引用计数-1(其实就是释放了)
    if (_storage and _starting_offset == _storage->size()) {
        _storage.reset();
    }
}

void BlockList::Append(const BlockList &other) {
    for (const auto &blk : other._blocks) {
        _blocks.push_back(blk);
    }
}

// 
BlockList::operator Block() const {
    switch (_blocks.size()) {
        case 0:
            return {};
        case 1:
            return _blocks[0];
        default: {
            return Block( Concatenate() );
            // throw runtime_error(
            //     "BlockList: please use Concatenate() to combine a multi-Block BlockList into one Block");
        }
    }
}

string BlockList::Concatenate() const {
    std::string ret;
    ret.reserve(size());
    for (const auto &blk : _blocks) {
        ret.append(blk);
    }
    return ret;
}

// blk.str() 是string_view,只有在合并的时候才会拷贝
string BlockList::Concatenate(size_t n) const {
    std::string ret;
    ret.reserve(n);
    for (const auto &blk : _blocks) {
        if (ret.size() + blk.size() <= n) {
            ret.append(blk);
        } else {
            auto tmp_blk = std::string_view( blk );
            ret.append(tmp_blk.begin(), tmp_blk.begin() + n - ret.size());
        }
    }
    return ret;
}

size_t BlockList::size() const {
    size_t ret = 0;
    for (const auto &blk : _blocks) {
        ret += blk.size();
    }
    return ret;
}

void BlockList::RemovePrefix(size_t n) {
    while (n > 0) {
        if (_blocks.empty()) {
            throw std::out_of_range("BlockList::RemovePrefix");
        }

        if (n < _blocks.front().str().size()) {
            _blocks.front().RemovePrefix(n);
            n = 0;
        } else {
            n -= _blocks.front().str().size();
            _blocks.pop_front();
        }
    }
}


vector<iovec> BlockList::as_iovecs() const{
    BlockViewList views{ *this };
    return views.as_iovecs();
}


BlockViewList::BlockViewList(const BlockList &blocks) {
    for (const auto &x : blocks.blocks()) {
        _views.push_back(x);
    }
}

void BlockViewList::RemovePrefix(size_t n) {
    while (n > 0) {
        if (_views.empty()) {
            throw std::out_of_range("BlockListView::RemovePrefix");
        }

        if (n < _views.front().size()) {
            _views.front().remove_prefix(n);
            n = 0;
        } else {
            n -= _views.front().size();
            _views.pop_front();
        }
    }
}

size_t BlockViewList::size() const {
    size_t ret = 0;
    for (const auto &blk : _views) {
        ret += blk.size();
    }
    return ret;
}

vector<iovec> BlockViewList::as_iovecs() const {
    vector<iovec> ret;
    ret.reserve(_views.size());
    for (const auto &x : _views) {
        ret.push_back({const_cast<char *>(x.data()), x.size()});
    }
    return ret;
}