#include "sliding_window.hh"

#include <cassert>

SlidingWindow::SlidingWindow(size_t init_window_size)
    : _window_size(init_window_size), _left_edge(0), _block_begins(), _block_ends() {}

size_t SlidingWindow::get_slidable_size() const {
    if (_block_begins.empty() || *_block_begins.begin() != _left_edge) {
        return 0;
    }
    return *_block_ends.begin() - *_block_begins.begin();
}

size_t SlidingWindow::slide_window(size_t slide_size) {
    size_t slidable = get_slidable_size();
    size_t actual_slide = std::min(slide_size, slidable);
    _left_edge += actual_slide;
    if (actual_slide == slidable) {
        _block_begins.erase(_block_begins.begin());
        _block_ends.erase(_block_ends.begin());
    } else if (actual_slide < slidable) {
        _block_begins.erase(_block_begins.begin());
        _block_begins.insert(_left_edge);
    } else {
        assert(false);
    }
    return actual_slide;
}

std::vector<std::pair<uint64_t, uint64_t>> SlidingWindow::fill_into(uint64_t block_begin, uint64_t block_end) {
    std::vector<std::pair<uint64_t, uint64_t>> subblocks = get_uncovered_subblocks(block_begin, block_end);
    for (const std::pair<uint64_t, uint64_t> &subblock : subblocks) {
        uint64_t begin = subblock.first, end = subblock.second;
        if (_block_begins.find(end) == _block_begins.end()) {
            _block_ends.insert(end);
        } else {
            _block_begins.erase(end);
        }
        if (_block_ends.find(begin) == _block_ends.end()) {
            _block_begins.insert(begin);
        } else {
            _block_ends.erase(begin);
        }
    }
    return subblocks;
}

void SlidingWindow::reset_window_size(size_t window_size) {
    assert(_block_begins.empty() || (*_block_ends.rbegin() - _left_edge <= window_size));
    _window_size = window_size;
}

std::vector<std::pair<uint64_t, uint64_t>> SlidingWindow::get_uncovered_subblocks(uint64_t block_begin,
                                                                                  uint64_t block_end) const {
    std::vector<std::pair<uint64_t, uint64_t>> subblocks;
    block_begin = std::max(block_begin, _left_edge);
    block_end = std::min(block_end, _left_edge + _window_size);
    while (block_begin < block_end) {
        size_t tmp_begin, tmp_end;
        auto begin_iter = _block_begins.upper_bound(block_begin);
        if (begin_iter == _block_begins.begin()) {  // * [<==begin_iter [
            tmp_begin = block_begin;
            tmp_end = std::min(begin_iter != _block_begins.end() ? *begin_iter : block_end, block_end);
        } else if (begin_iter != _block_begins.end()) {  // [ * [<==begin_iter
            auto end_iter = _block_ends.upper_bound(*(--begin_iter));
            tmp_begin = std::max(*end_iter, block_begin);
            tmp_end = std::min(*(++begin_iter), block_end);
        } else {  // [ [ * <==begin_iter
            auto end_iter = _block_ends.upper_bound(*(--begin_iter));
            tmp_begin = std::max(*end_iter, block_begin);
            tmp_end = block_end;
        }
        if (tmp_begin < tmp_end) {
            subblocks.push_back(std::make_pair(tmp_begin, tmp_end));
        }
        block_begin = tmp_end;
    }
    return subblocks;
}