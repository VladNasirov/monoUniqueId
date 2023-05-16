#pragma once
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "AppModel.h"
#include "RequestQueue.h"
#include <condition_variable>
class AppManager {
public:
    AppManager(int numApps, int numRequests, int per);
    ~AppManager();
    int findMinCounterIndex();
    void stop();
private:
    std::vector<AppModel> apps;
    const char* SERVER_IP = "127.0.0.1";
    std::vector<const char*> appAddresses;
    std::vector<int> appPorts;
   
    int numRequests;
    int numApps;
    int period;
    int senderIndex;
    int receiverIndex;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::mutex send_mtx;
    std::mutex state_mtx;
    std::condition_variable sendCV;
    int countFreeApps();
    int generatePort();

    bool sendApp(int i);
    bool recvApp(int i);
    void runApp(int i);
    int GenerateReciver(int i);
    int GenerateSender(int i);
    RequestQueue Que;
};