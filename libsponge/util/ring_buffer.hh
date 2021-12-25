#ifndef SPONGE_LIBSPONGE_RING_BUFFER_HH
#define SPONGE_LIBSPONGE_RING_BUFFER_HH

#include <string>

class RingBuffer {
  public:
    RingBuffer(size_t capacity);

  private:
    size_t _capacity;
    size_t _buffer_capacity;
    std::string _buffer;
    size_t _head;
    size_t _tail;
    size_t _offset;
};

#endif