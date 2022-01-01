#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <cassert>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _window(1)
    , _bytes_in_flight(0)
    , _consecutive_retransmissions(0)
    , _cur_time(0)
    , _retransmission_timeout(_initial_retransmission_timeout)
    , _outstanding_segements()
    , _fin_sent(false)
    , _zero_window_size(false) {}

void TCPSender::fill_window() {
    while (1) {
        if (_fin_sent) {
            return;
        }

        if (_zero_window_size) {
            _window.reset_window_size(1);
        }
        TCPSegment segment;
        size_t syn_bytes = (_next_seqno == 0 ? 1 : 0);
        _bytes_in_flight += syn_bytes;
        size_t payload_bytes = std::min(TCPConfig::MAX_PAYLOAD_SIZE,
                                        std::min(_stream.buffer_size(), _window.window_size() - _bytes_in_flight));
        segment.payload() = _stream.read(payload_bytes);
        _bytes_in_flight += payload_bytes;
        size_t fin_bytes = 0;
        if (_stream.eof() && _window.window_size() > _bytes_in_flight) {
            fin_bytes = 1;
            _fin_sent = true;
        }
        _bytes_in_flight += fin_bytes;
        size_t total_bytes = syn_bytes + payload_bytes + fin_bytes;
        if (total_bytes == 0) {
            return;
        }
        segment.header().syn = (syn_bytes == 1);
        segment.header().fin = (fin_bytes == 1);
        segment.header().seqno = wrap(_next_seqno, _isn);
        _next_seqno += total_bytes;
        _outstanding_segements[_next_seqno] = segment;
        _segments_out.push(segment);
        _window.fill_into(_window.left_edge(), _next_seqno);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t absolute_ackno = unwrap(ackno, _isn, _window.left_edge());
    if (absolute_ackno < _window.left_edge() ||
        (absolute_ackno == _window.left_edge() && window_size <= _window.window_size()) ||
        (absolute_ackno > _next_seqno)) {
        return;
    }
    if (window_size > 0) {
        _zero_window_size = false;
    } else {
        _zero_window_size = true;
    }
    size_t slide_size = absolute_ackno - _window.left_edge();
    _window.slide_window(slide_size);
    _window.reset_window_size(window_size);
    _bytes_in_flight -= slide_size;
    _consecutive_retransmissions = 0;
    _cur_time = 0;
    _retransmission_timeout = _initial_retransmission_timeout;

    while (!_outstanding_segements.empty() && _outstanding_segements.begin()->first <= absolute_ackno) {
        _outstanding_segements.erase(_outstanding_segements.begin());
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (_bytes_in_flight == 0) {
        return;
    }

    _cur_time += ms_since_last_tick;
    if (_cur_time >= _retransmission_timeout) {
        _segments_out.push(_outstanding_segements.begin()->second);
        if (!_zero_window_size) {
            ++_consecutive_retransmissions;    
            _retransmission_timeout *= 2;
        }
        _cur_time = 0;
    }
}

void TCPSender::send_empty_segment() { _segments_out.push(TCPSegment()); }
