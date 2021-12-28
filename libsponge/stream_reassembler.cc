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
    , _segment_begins()
    , _segment_ends()
    , _next_segment_begin(0)
    , _unassembled_bytes(0)
    , _eof_index(numeric_limits<size_t>::max()) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    size_t begin_of_data = 0;
    size_t end_of_data = data.size();
    if (index < _next_segment_begin) {
        begin_of_data += _next_segment_begin - index;
    }
    if (index + data.size() > _capacity + _next_segment_begin - _output.buffer_size()) {
        end_of_data = _capacity + _next_segment_begin - index - _output.buffer_size();
    }

    vector<size_t> begins;
    vector<size_t> ends;
    get_nonoverlapping_segment(index + begin_of_data, index + end_of_data, begins, ends);
    for (size_t i = 0; i < begins.size(); ++i) {
        push_nonoverlapping_substring(data, index, begins[i] - index, ends[i] - index);
    }

    if (!_segment_begins.empty() && *_segment_begins.begin() == _next_segment_begin) {
        uint64_t begin_of_contiguous = *_segment_begins.begin();
        uint64_t end_of_contiguous = *_segment_ends.begin();
        uint64_t contiguous_length = end_of_contiguous - begin_of_contiguous;
        // size_t written_length = _output.write(_buffer.substr(begin_of_contiguous - _offset, contiguous_length));
        size_t written_length = _output.write(_buffer.peek(contiguous_length));
        _buffer.pop(written_length);
        _next_segment_begin += written_length;
        if (contiguous_length == written_length) {
            _segment_begins.erase(_segment_begins.begin());
            _segment_ends.erase(_segment_ends.begin());
        } else {
            _segment_begins.erase(_segment_begins.begin());
            _segment_begins.insert(_next_segment_begin);
        }
        _unassembled_bytes -= written_length;
    }

    if (eof) {
        _eof_index = index + data.size();
    }
    if (_next_segment_begin == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }

void StreamReassembler::get_nonoverlapping_segment(size_t begin_of_segment,
                                                   size_t end_of_segment,
                                                   std::vector<size_t> &begins,
                                                   std::vector<size_t> &ends) const {
    if (begin_of_segment >= end_of_segment) {
        return;
    }
    while (begin_of_segment != end_of_segment) {
        size_t tmp_begin = begin_of_segment;
        size_t tmp_end = end_of_segment;
        auto begin_iter = _segment_begins.upper_bound(begin_of_segment);
        if (begin_iter == _segment_begins.begin()) {  // * [<==begin_iter [
            tmp_begin = begin_of_segment;
            tmp_end = min(begin_iter != _segment_begins.end() ? *begin_iter : end_of_segment, end_of_segment);
        } else if (begin_iter != _segment_begins.end()) {  // [ * [<==begin_iter
            auto end_iter = _segment_ends.upper_bound(*(--begin_iter));
            tmp_begin = max(*end_iter, begin_of_segment);
            tmp_end = min(*(++begin_iter), end_of_segment);
        } else {  // [ [ * <==begin_iter
            auto end_iter = _segment_ends.upper_bound(*(--begin_iter));
            tmp_begin = max(*end_iter, begin_of_segment);
            tmp_end = end_of_segment;
        }
        begin_of_segment = tmp_end;
        if (tmp_begin < tmp_end) {
            begins.push_back(tmp_begin);
            ends.push_back(tmp_end);
        }
    }
}

void StreamReassembler::push_nonoverlapping_substring(const string &data,
                                                      const size_t index,
                                                      const size_t begin_of_data,
                                                      const size_t end_of_data) {
    assert(end_of_data <= data.size());

    size_t length = end_of_data - begin_of_data;
    size_t begin_of_segment = index + begin_of_data;
    size_t end_of_segment = index + end_of_data;

    _buffer.insert(data.data() + begin_of_data, length, begin_of_segment - _next_segment_begin);
    _unassembled_bytes += length;

    if (_segment_begins.find(end_of_segment) == _segment_begins.end()) {
        _segment_ends.insert(end_of_segment);
    } else {
        _segment_begins.erase(end_of_segment);
    }
    if (_segment_ends.find(begin_of_segment) == _segment_ends.end()) {
        _segment_begins.insert(begin_of_segment);
    } else {
        _segment_ends.erase(begin_of_segment);
    }
}
