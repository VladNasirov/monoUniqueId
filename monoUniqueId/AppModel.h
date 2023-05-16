#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UniqueIdGenerator.h"
#include "message.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>


enum class ServerState {
    kIdle,       
    kSending,   
    kReceiving   
};
class AppModel {
public:
    AppModel(const char* ip_address, int port_number, UniqueIdGenerator& idgen);
    AppModel(const AppModel& other);
    ~AppModel();
    void setState(ServerState state);
    ServerState getState();
    bool sendDataToRemoteAppModel(const char* remoteIp, int remotePort);
    int getCounter();
    std::chrono::nanoseconds uptime() const;
    bool receiveDataFromRemoteAppModel(int localPort);
    void insertMessageToDBbyId(Message& msg);
    void insertMessageToDBbyUniqueId(Message& msg);
    void close();
    void incCounter();
    void decCounter();
    bool isAvailable();
private:
    const char* SERVER_IP;
    int PORT;
    UniqueIdGenerator id_gen;
    static const int BUFFER_SIZE = 1024;
    WSADATA wsaData;
    SOCKET sendSocket;
    SOCKET recvSocket;
    int counter_;
    std::chrono::time_point<std::chrono::system_clock> start_time_;
    ServerState state_;
    std::mutex mutex_;
    std::mutex dbMutex_;
    std::mutex counterMutex_;
    std::mutex recvMutex_;

};