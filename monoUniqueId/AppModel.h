#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "UniqueIdGenerator.h"
#include "message.h"
#include <string>
#include <thread>
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
    bool receiveDataFromRemoteAppModel();
    void insertMessageToDBbyId(Message& msg);
    void insertMessageToDBbyUniqueId(Message& msg);
    void close();
    
    void incCounter();
    void decCounter();
    
    const char* getAddr();
    int getPort();
    
    bool isAvailable();
    
    void start_recv();
    void stop_recv();
private:
    const char* SERVER_IP;
    int PORT;
    bool is_recv;
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

};