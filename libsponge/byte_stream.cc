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
    : _error(false), _capacity(capacity), _buffer(capacity), _bytes_read(0), _bytes_written(0), _input_ended(false) {}

size_t ByteStream::write(const string &data) {
    size_t bytes_written = _buffer.append(data);
    _bytes_written += bytes_written;
    return bytes_written;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { return _buffer.peek(len); }

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t bytes_to_pop = _buffer.pop(len);
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

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return _input_ended && _buffer.empty(); }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }