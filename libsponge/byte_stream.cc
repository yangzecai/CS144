#include "byte_stream.hh"

#include <cstring>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _error(false)
    , _capacity(capacity)
    , _buffer_capacity(capacity + 1)
    , _buffer(new char[capacity + 1])
    , _head(0)
    , _tail(0)
    , _bytes_read(0)
    , _bytes_written(0)
    , _input_ended(false) {}

void ByteStream::move_pointer(size_t &pointer, size_t size) const { pointer = (pointer + size) % _buffer_capacity; }

size_t ByteStream::write(const string &data) {
    size_t bytes_written = 0;
    while (size_t bytes_to_write = min(data.size() - bytes_written, local_remaining_capacity()) != 0) {
        strncpy(_buffer + _tail, data.data() + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        move_pointer(_tail, bytes_to_write);
    }
    _bytes_written += bytes_written;
    return bytes_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t bytes_peeked = 0;
    string ret;
    ret.reserve(len);
    size_t shadow_head = _head;
    while (size_t bytes_to_peek = min(len - bytes_peeked, local_buffer_size()) != 0) {
        ret.append(_buffer + shadow_head, bytes_to_peek);
        bytes_peeked += bytes_to_peek;
        move_pointer(shadow_head, bytes_to_peek);
    }
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t bytes_to_pop = min(len, buffer_size());
    move_pointer(_head, bytes_to_pop);
    _bytes_read += bytes_to_pop;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret = peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() { _input_ended = true; }

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const {
    if (_tail >= _head) {
        return _tail - _head;
    } else {
        return _buffer_capacity - _head + _tail;
    }
}

bool ByteStream::buffer_empty() const { return _head == _tail; }

bool ByteStream::eof() const { return _input_ended && _tail == _head; }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }

size_t ByteStream::local_remaining_capacity() const {
    return _head <= _tail ? (_buffer_capacity - _tail - (_head == 0 ? 1 : 0)) : _head - _tail - 1;
}

size_t ByteStream::local_buffer_size() const { return _head <= _tail ? _tail - _head : _buffer_capacity - _head; }