#include "tcp_connection.hh"

#include <algorithm>
#include <cmath>
#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        return;
    }
    _receiver.segment_received(seg);
    if (!_receiver.ackno().has_value()) {
        return;
    }
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }
    _sender.fill_window();
    if (seg.length_in_sequence_space() != 0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
        (seg.header().seqno == _receiver.ackno().value() - 1) && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    add_segments_out();
    _time_since_last_segment_received = 0;
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    if (_sender.stream_in().input_ended()) {
        return 0;
    }
    size_t bytes_written = _sender.stream_in().write(data);
    _sender.fill_window();
    add_segments_out();
    return bytes_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!_receiver.ackno().has_value() && _sender.bytes_in_flight() == 0) {
        return;
    }
    _time_since_last_segment_received += ms_since_last_tick;
    if (_linger_after_streams_finish && _sender.stream_in().eof() && _receiver.stream_out().eof() &&
        _time_since_last_segment_received >= 10 * _cfg.rt_timeout) {
        _active = false;
        return;
    }

    _sender.tick(ms_since_last_tick);
    _sender.fill_window();
    add_segments_out();
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        while (!_segments_out.empty()) {
            _segments_out.pop();
        }
        _sender.send_empty_segment();
        _sender.segments_out().back().header().rst = true;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        add_segments_out();
        return;
    }
}

void TCPConnection::end_input_stream() {
    if (_sender.stream_in().input_ended()) {
        return;
    }
    _sender.stream_in().end_input();
    _sender.fill_window();
    add_segments_out();
}

void TCPConnection::connect() {
    _sender.fill_window();
    add_segments_out();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _sender.send_empty_segment();
            _sender.segments_out().back().header().rst = true;
            add_segments_out();
            _active = false;
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::add_segments_out() {
    while (!_sender.segments_out().empty()) {
        _segments_out.push(_sender.segments_out().front());
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value()) {
            _segments_out.back().header().ack = true;
            _segments_out.back().header().ackno = _receiver.ackno().value();
            _segments_out.back().header().win =
                min(static_cast<size_t>(numeric_limits<uint16_t>::max()), _receiver.window_size());
        }
    }
    if (!_linger_after_streams_finish && _sender.stream_in().eof() && _receiver.stream_out().eof() &&
        _sender.bytes_in_flight() == 0) {
        _active = false;
    }
}
