#pragma once
#include <iostream>
#include "UniqueIdGenerator.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
class Message {
public:
    int counter;
    sockaddr_in client_address;
    std::string request_id;

    Message() : counter(0), client_address(), request_id("") {}
    Message(int c, sockaddr_in ca, const std::string& id) : counter(c), client_address(ca), request_id(id) {}

    friend std::ostream& operator<<(std::ostream& os, const Message& message) {
        os << message.counter << " "
            << inet_ntoa((message.client_address.sin_addr)) << ":" << ntohs(message.client_address.sin_port)
            << " " << message.request_id;
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
        is >> message.request_id;
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