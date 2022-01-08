#include "ring_buffer.hh"

#include <cstring>
#include <string>

RingBuffer::RingBuffer(size_t capacity)
    : _capacity(capacity), _buffer_capacity(capacity + 1), _buffer(_buffer_capacity, 0), _head(0), _tail(0) {}

void RingBuffer::move_pointer(size_t &pointer, size_t size) const { pointer = (pointer + size) % _buffer_capacity; }

size_t RingBuffer::insert(const char *data, size_t len, size_t pos) {
    size_t bytes_written = 0;
    size_t pseudo_tail = _head;
    move_pointer(pseudo_tail, pos);
    while (size_t bytes_to_write = std::min(len - bytes_written, local_remaining_capacity(pseudo_tail)) != 0) {
        std::memcpy(_buffer.data() + pseudo_tail, data + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
        move_pointer(pseudo_tail, bytes_to_write);
    }
    if (pseudo_tail - _head > _tail - _head) {
        _tail = pseudo_tail;
    }
    return bytes_written;
}

size_t RingBuffer::insert(const std::string &data, size_t pos) { return insert(data.data(), data.size(), pos); }

size_t RingBuffer::append(const std::string &data) { return insert(data.data(), data.size(), size()); }

size_t RingBuffer::append(const char *data, size_t len) { return insert(data, len, size()); }

std::string RingBuffer::peek(const size_t len) const {
    size_t bytes_peeked = 0;
    std::string ret;
    ret.reserve(len);
    size_t shadow_head = _head;
    while (size_t bytes_to_peek = std::min(len - bytes_peeked, local_buffer_size()) != 0) {
        ret.append(_buffer.data() + shadow_head, bytes_to_peek);
        bytes_peeked += bytes_to_peek;
        move_pointer(shadow_head, bytes_to_peek);
    }
    return ret;
}

size_t RingBuffer::pop(const size_t len) {
    size_t bytes_to_pop = std::min(len, size());
    move_pointer(_head, bytes_to_pop);
    return bytes_to_pop;
}

std::string RingBuffer::read(const size_t len) {
    std::string ret = peek(len);
    pop(ret.size());
    return ret;
}

size_t RingBuffer::remaining_capacity() const { return _capacity - size(); }

size_t RingBuffer::size() const {
    if (_tail >= _head) {
        return _tail - _head;
    } else {
        return _buffer_capacity - _head + _tail;
    }
}

bool RingBuffer::empty() const { return _tail == _head; }

void RingBuffer::reset() {
    _head = 0;
    _tail = 0;
}

size_t RingBuffer::local_remaining_capacity(size_t from) const {
    return _head <= from ? (_buffer_capacity - from - (_head == 0 ? 1 : 0)) : _head - from - 1;
}

size_t RingBuffer::local_buffer_size() const { return _head <= _tail ? _tail - _head : _buffer_capacity - _head; }