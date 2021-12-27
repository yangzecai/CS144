#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32(static_cast<uint32_t>(n & 0xffffffff) + isn.raw_value());
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t base = n.raw_value() - isn.raw_value();
    uint64_t first = (checkpoint >> 32) << 32 | base;
    uint64_t second = max(((checkpoint >> 32) + 1) << 32 | base, first);
    uint64_t third = min(((checkpoint >> 32) - 1) << 32 | base, first);
    uint64_t first_dist = min(checkpoint - first, first - checkpoint);
    uint64_t second_dist = min(checkpoint - second, second - checkpoint);
    uint64_t third_dist = min(checkpoint - third, third - checkpoint);
    uint64_t min_dist = min(first_dist, min(second_dist, third_dist));
    if (min_dist == first_dist) {
        return first;
    } else if (min_dist == second_dist) {
        return second;
    } else {
        return third;
    }
}
