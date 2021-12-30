#include "stream_reassembler.hh"

#include <cassert>
#include <cstring>
#include <iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _buffer(capacity)
    , _window(capacity)
    , _unassembled_bytes(0)
    , _eof_index(numeric_limits<size_t>::max()) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _window.reset_window_size(_capacity - _output.buffer_size());
    std::vector<std::pair<uint64_t, uint64_t>> strings = _window.fill_into(index, index + data.size());
    for (std::pair<uint64_t, uint64_t>& begin_end : strings) {
        uint64_t begin = begin_end.first, end = begin_end.second;
        size_t begin_in_data = begin - index;
        size_t length = end - begin;
        size_t begin_in_window = begin - _window.left_edge();
        _buffer.insert(data.data() + begin_in_data, length, begin_in_window);
        _unassembled_bytes += length;
    }

    size_t slidable_bytes = _window.get_slidable_size();
    if (slidable_bytes != 0) {
        _window.slide_window(slidable_bytes);
        _output.write(_buffer.read(slidable_bytes));
        _unassembled_bytes -= slidable_bytes;
    }

    if (eof) {
        _eof_index = index + data.size();
    }
    if (_window.left_edge() == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
