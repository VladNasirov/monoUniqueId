//#pragma once
//#include "AppManager.h"
//AppManager::AppManager(int numApps, int numRequests, int per) {
//    period = per;
//    UniqueIdGenerator id_gen;
//    for (int i = 0; i < numApps; ++i) {
//        int port = generatePort();
//        apps.emplace_back(SERVER_IP, port, id_gen);
//        appAddresses.push_back(SERVER_IP);
//        appPorts.push_back(port);
//    }
//    this->numRequests = numRequests;
//    this->numApps = numApps;
//
//    threads.reserve(numApps);
//    for (int i = 0; i < numApps; ++i) {
//        threads.emplace_back(&AppManager::runApp, this, i);
//    }
//}
//void AppManager::stop() {
//    // Остановить работу потоков
//    for (auto& thread : threads) {
//        if (thread.joinable()) {
//            thread.join();
//        }
//    }
//
//    // Вызвать деструкторы для объектов AppModel
//    for (auto& app : apps) {
//        app.close();
//    }
//}
//AppManager::~AppManager() {
//    stop();
//}
////int AppManager::findMinCounterIndex() {
////    int minIndex = 0;
////    for (int i = 0; i < apps.size() - 1; ++i) {
////        if (apps[i].getCounter() < apps[minIndex].getCounter()) {
////            minIndex = i;
////        }
////    }
////    return minIndex;
////}
//int AppManager::generatePort() {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> distrib(5000, 6000);
//    return distrib(gen);
//}
//int AppManager::GenerateReciver(int i)
//{
//    //mutex
//    std::random_device rd;
//    std::mt19937 gen(rd());
//   std::uniform_int_distribution<> distrib(0, numApps - 1);
//   int recvindex;
//   do {       
//       recvindex = distrib(gen);
//   } while (i == recvindex && apps[recvindex].getState()==ServerState::kIdle);
//   return recvindex;
//
//}
//int AppManager::countFreeApps() {
//    int count = 0;
//    for (auto& app : apps) {
//        if (app.isAvailable()) {
//            count++;
//        }
//    }
//    return count;
//}
//int AppManager::GenerateSender(int i) {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> distrib(0, numApps - 1);
//    int senderIndex;
//    do {
//        senderIndex = distrib(gen);
//    } while (i == senderIndex && apps[senderIndex].getState() == ServerState::kIdle);
//    return senderIndex;
//}
//void AppManager::dataExchange(int sender, int reciever) {
//    // Получаем ссылки на объекты AppModel по индексам
//    AppModel& app1 = apps[sender];
//    AppModel& app2 = apps[reciever];
//   
//    // Принимаем данные для app2
//    app2.receiveDataFromRemoteAppModel();
//
//    // Отправляем данные от app1 к app2
//    app1.sendDataToRemoteAppModel(app2.getAddr(), app2.getPort());
//}
//
//void AppManager::runApp(int i) {
//    while (numRequests > 0) {
//
//        // Проверяем доступность отправителя и получателя
//        if (apps[sender].isAvailable() && apps[receiver].isAvailable()) {
//            // Обмен данными между отправителем и получателем
//            dataExchange(sender, receiver);
//        }
//        else {
//            // Обработка случая, когда отправитель или получатель недоступны
//            // Например, ожидание доступности или выбор других приложений
//            // ...
//
//            // Пропустить текущую итерацию и перейти к следующей
//            continue;
//        }
//
//        // Уменьшаем счетчик оставшихся запросов
//        numRequests--;
//    }
//   // stop();
//}
