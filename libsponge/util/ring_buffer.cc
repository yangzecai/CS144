#include "ring_buffer.hh"

RingBuffer::RingBuffer(size_t capacity)
    : _capacity(capacity)
    , _buffer_capacity(capacity + 1)
    , _buffer(_buffer_capacity, 0)
    , _head(0)
    , _tail(0)
    , _offset(0) {}


