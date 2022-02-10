#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>
#include <cassert>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().type = EthernetHeader::TYPE_IPv4;
    frame.payload() = std::move(dgram.serialize());

    if (_peer_ethernet_address.find(next_hop_ip) == _peer_ethernet_address.end() ||
        _time >= _peer_ethernet_address.find(next_hop_ip)->second.expiration_time) {
        
        _incomplete_frames[next_hop_ip].push_back(std::move(frame));

        EthernetFrame request;
        request.header().dst = ETHERNET_BROADCAST;
        request.header().src = _ethernet_address;
        request.header().type = EthernetHeader::TYPE_ARP;
        ARPMessage message;
        message.opcode = ARPMessage::OPCODE_REQUEST;
        message.sender_ethernet_address = _ethernet_address;
        message.sender_ip_address = _ip_address.ipv4_numeric();
        message.target_ip_address = next_hop_ip;
        request.payload() = std::move(message.serialize());
        _arp_requests.push(std::move(request));
        send_arp_request();
    } else {
        frame.header().dst = _peer_ethernet_address.find(next_hop_ip)->second.mac;
        _frames_out.push(std::move(frame));
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != ETHERNET_BROADCAST &&
        frame.header().dst != _ethernet_address) {
        return {};
    }
    if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage message;
        if (ParseResult::NoError != message.parse(frame.payload())) {
            return {};
        }
        switch (message.opcode) {
        case ARPMessage::OPCODE_REPLY:
            recv_arp_reply(message);
            break;
        case ARPMessage::OPCODE_REQUEST:
            recv_arp_request(message);
            break;
        default:
            assert(false);
            break;
        }
        return {};
    } else if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        IPv4Datagram datagram;
        if (ParseResult::NoError != datagram.parse(frame.payload())) {
            return {};
        }
        return datagram;
    } else {
        return {};
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    _time += ms_since_last_tick;
    send_arp_request();
}

void NetworkInterface::send_arp_request() {
    if (_arp_requests.empty()) {
        return;
    }
    if (_waiting_arp_reply && _time < _reply_timeout) {
        return;
    }
    _frames_out.push(_arp_requests.front());
    _waiting_arp_reply = true;
    _reply_timeout = _time + 5000;
}

void NetworkInterface::recv_arp_reply(ARPMessage& message) {
    if (message.target_ip_address != _ip_address.ipv4_numeric() ||
        message.target_ethernet_address != _ethernet_address) {
        return;
    }
    _arp_requests.pop();
    _waiting_arp_reply = false;
    _reply_timeout = 0;
    _peer_ethernet_address[message.sender_ip_address] = {message.sender_ethernet_address, _time + 30000};
    for (EthernetFrame& frame : _incomplete_frames[message.sender_ip_address]) {
        frame.header().dst = message.sender_ethernet_address;
        _frames_out.push(std::move(frame));
    }
    _incomplete_frames.erase(message.sender_ip_address);
    send_arp_request();
}

void NetworkInterface::recv_arp_request(ARPMessage& message) {
    if (message.target_ip_address != _ip_address.ipv4_numeric()) {
        return;
    }

    _peer_ethernet_address[message.sender_ip_address] = {message.sender_ethernet_address, _time + 30000};

    EthernetFrame reply;
    reply.header().dst = message.sender_ethernet_address;
    reply.header().src = _ethernet_address;
    reply.header().type = EthernetHeader::TYPE_ARP;
    message.opcode = ARPMessage::OPCODE_REPLY;
    std::swap(message.sender_ip_address, message.target_ip_address);
    std::swap(message.sender_ethernet_address, message.target_ethernet_address);
    message.sender_ethernet_address = _ethernet_address;
    reply.payload() = std::move(message.serialize());
    _frames_out.push(std::move(reply));
}