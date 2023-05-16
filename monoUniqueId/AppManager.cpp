#pragma once
#include "AppManager.h"
AppManager::AppManager(int numApps, int numRequests, int per) {
    period = per;
    UniqueIdGenerator id_gen;
    for (int i = 0; i < numApps; ++i) {
        int port = generatePort();
        apps.emplace_back(SERVER_IP, port, id_gen);
        appAddresses.push_back(SERVER_IP);
        appPorts.push_back(port);
    }
    this->numRequests = numRequests;
    this->numApps = numApps;

    threads.reserve(numApps);
    for (int i = 0; i < numApps; ++i) {
        threads.emplace_back(&AppManager::runApp, this, i);
    }
}
void AppManager::stop() {
    // Остановить работу потоков
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Вызвать деструкторы для объектов AppModel
    for (auto& app : apps) {
        app.close();
    }
}
AppManager::~AppManager() {
    stop();
}
//int AppManager::findMinCounterIndex() {
//    int minIndex = 0;
//    for (int i = 0; i < apps.size() - 1; ++i) {
//        if (apps[i].getCounter() < apps[minIndex].getCounter()) {
//            minIndex = i;
//        }
//    }
//    return minIndex;
//}
int AppManager::generatePort() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(5000, 6000);
    return distrib(gen);
}
int AppManager::GenerateReciver(int i)
{
    //mutex
    std::random_device rd;
    std::mt19937 gen(rd());
   std::uniform_int_distribution<> distrib(0, numApps - 1);
   int recvindex;
   do {       
       recvindex = distrib(gen);
   } while (i == recvindex && apps[recvindex].getState()==ServerState::kIdle);
   return recvindex;

}
int AppManager::countFreeApps() {
    int count = 0;
    for (auto& app : apps) {
        if (app.getState() == ServerState::kIdle) {
            count++;
        }
    }
    return count;
}
int AppManager::GenerateSender(int i) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, numApps - 1);
    int senderIndex;
    do {
        senderIndex = distrib(gen);
    } while (i == senderIndex && apps[senderIndex].getState() == ServerState::kIdle);
    return senderIndex;
}

void AppManager::runApp(int i) {
    while (numRequests)
    {
        switch (apps[i].getState()) {
        case ServerState::kIdle:
        {
           
            if (countFreeApps() > 1)
            {
                std::lock_guard<std::mutex> lock(mtx);
                int recvind = GenerateReciver(i);
                apps[i].setState(ServerState::kSending);
                apps[recvind].setState(ServerState::kReceiving);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(period));
            }
            break;
        }
        case ServerState::kReceiving:
        {
            // Handle receiving state
            
            if (apps[i].receiveDataFromRemoteAppModel(appPorts[i]))
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "App " << i << " received\n";
                apps[i].setState(ServerState::kIdle);
                numRequests--;
                std::cout << "NumRequest= " << numRequests << std::endl;
            }
            else {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "App " << i << " can't receive\n";
                apps[i].setState(ServerState::kIdle);

            }
            break;
        }
        case ServerState::kSending:
        {
            // Handle sending state

            
            if (apps[i].sendDataToRemoteAppModel(appAddresses[recind], appPorts[recind]))
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "App " << i << " send to " << recind << std::endl;
                apps[i].setState(ServerState::kIdle);
            }
            else {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "App " << i << " can't send to " << recind << std::endl;
                apps[i].setState(ServerState::kIdle);
            }
            break;
        }
       
        }
        Que.removeSuccessRequests();
    }
   // stop();
}