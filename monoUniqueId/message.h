#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UniqueIdGenerator.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#include "VectorClock.h"
#pragma comment(lib, "ws2_32.lib")
class Message {
public:
    int counter;
    int recv_PORT;
    sockaddr_in client_address;
    std::string request_id;
    VectorClock MessageClock;
    Message() : counter(0), recv_PORT(0), client_address(), request_id(""), MessageClock() {}
    Message(int c, sockaddr_in ca, int rp, const std::string& id, VectorClock& clock) : counter(c), client_address(ca), recv_PORT(rp), request_id(id), MessageClock(clock) {}

    friend std::ostream& operator<<(std::ostream& os, const Message& message) {
        os << message.counter << " "
            << inet_ntoa((message.client_address.sin_addr)) << ":" << ntohs(message.client_address.sin_port)<<" "
            << std::to_string(message.recv_PORT) << " "
            << " " << message.request_id<<" " << message.MessageClock.to_string();
        return os;
    }
    friend std::istream& operator>>(std::istream& is, Message& message) {
        char buffer[INET_ADDRSTRLEN];
        is >> message.counter;
        is.ignore(); // Ignore the space character

        is.getline(buffer, INET_ADDRSTRLEN, ':');
        message.client_address.sin_family = AF_INET;
        std::string port_str;
        std::getline(is, port_str, ' ');
        message.client_address.sin_port = htons(std::stoi(port_str));
        inet_pton(AF_INET, buffer, &message.client_address.sin_addr);

        is >> message.recv_PORT;
        is >> message.request_id;
        std::string clock_str;
        is >> clock_str;
        message.MessageClock.from_string(clock_str);
        return is;
    }

    void changeRequestId(std::string id)
    {
        request_id = id;
    }

    std::string GetSourceAddressAsString() const {
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), buffer, INET_ADDRSTRLEN);
        std::string address(buffer);
        return address + ":" + std::to_string(ntohs(client_address.sin_port));
    }
    sockaddr_in GetSourceAddress() const {
        return client_address;
    }
    const std::string& GetId() const {
        return request_id;
    }
};