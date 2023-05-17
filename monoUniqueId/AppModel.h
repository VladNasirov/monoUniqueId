#pragma once
#include "UniqueIdGenerator.h"
#include "MessageQueue.h"
#include <string>
#include <thread>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

class AppModel {
public:
    AppModel(const char* ip_address, int port_number, std::shared_ptr<MessageQueue> Que, std::shared_ptr<UniqueIdGenerator> idgen);
    AppModel(const AppModel& other);
    ~AppModel();
    bool sendDataToRemoteAppModel(const char* remoteIp, int remotePort);
    bool receiveDataFromRemoteAppModel();

    void insertMessageToDBbyId(Message& msg);
    void insertMessageToDBbyUniqueId(Message& msg);

    void close();

    void incCounter();
    void decCounter();

    const char* getAddr();
    int getPort();
    int getCounter();
    std::chrono::nanoseconds uptime() const;
    void setVC(VectorClock vec);

    void start_recv();
    void stop_recv();
private:
    const char* SERVER_IP;
    int PORT;
    bool is_recv;
    bool is_proc;
    static const int BUFFER_SIZE = 1024;
    WSADATA wsaData;
    SOCKET sendSocket;
    SOCKET recvSocket;
    int counter_;
    std::chrono::time_point<std::chrono::system_clock> start_time_;
    std::mutex mutex_;
    std::mutex dbMutex_;
    std::mutex recvAPPMutex_;
    std::mutex counterMutex_;
    std::shared_ptr<UniqueIdGenerator> id_gen;
    VectorClock vc;
    std::shared_ptr<MessageQueue> MesQue;

    
};