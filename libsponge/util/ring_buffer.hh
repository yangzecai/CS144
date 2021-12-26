#ifndef SPONGE_LIBSPONGE_RING_BUFFER_HH
#define SPONGE_LIBSPONGE_RING_BUFFER_HH

#include <string>

class RingBuffer {
  public:
    RingBuffer(size_t capacity);

    size_t append(const std::string &data);

    size_t append(const char *data, size_t len);

    size_t insert(const std::string &data, size_t pos);

    size_t insert(const char *data, size_t len, size_t pos);

    std::string peek(const size_t len) const;

    size_t pop(const size_t len);

    std::string read(const size_t len);

    size_t remaining_capacity() const;

    size_t size() const;

    bool empty() const;

    void reset();

  private:
    void move_pointer(size_t &pointer, size_t size) const;

    size_t local_remaining_capacity(size_t from) const;
    
    size_t local_buffer_size() const;

    const size_t _capacity;
    const size_t _buffer_capacity;
    std::string _buffer;
    size_t _head;
    size_t _tail;
};

#endif