#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

TCPReceiver::TCPReceiver(const size_t capacity)
    : _reassembler(capacity)
    , _capacity(capacity)
    , _isn()
    , _begin_of_unassembled_index(0) {}

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!_isn.has_value()) {
        if (seg.header().syn) {
            _isn = seg.header().seqno;
        } else {
            return;
        }
    }
    uint64_t absolute_seqno = unwrap(seg.header().seqno, _isn.value(), _begin_of_unassembled_index);
    if (absolute_seqno + static_cast<uint64_t>(seg.length_in_sequence_space()) <= _begin_of_unassembled_index) {
        return;
    }
    size_t pre_received = stream_out().buffer_size();
    int64_t index = unwrap(WrappingInt32(seg.header().seqno.raw_value() + (seg.header().syn ? 1 : 0)),
                           _isn.value(),
                           _begin_of_unassembled_index) - 1;
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);
    size_t post_received = stream_out().buffer_size();
    _begin_of_unassembled_index += post_received - pre_received;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    optional<WrappingInt32> ackno;
    if (_isn.has_value()) {
        ackno = wrap(_begin_of_unassembled_index + 1 + (stream_out().input_ended() ? 1 : 0), _isn.value());
    }
    return ackno;
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
