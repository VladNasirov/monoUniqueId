#pragma once
#include "AppManager.h"
AppManager::AppManager(int number_of_apps, int reqnumber, int period) : AppNumber(number_of_apps), reqnumber(reqnumber), period(period)
{
    createApps();
    startAllApps();
}
AppManager::~AppManager() {
    stopAllApps();
    deleteApps();
}
void AppManager::createApps() {
    Que = std::make_shared<MessageQueue>();
    id_gen = std::make_shared<UniqueIdGenerator>();
    for (int i = 0; i < AppNumber; i++) {
        AppModel* app = new AppModel("127.0.0.1", generatePort(), Que, id_gen);
        addApp(app);
    }
}
void AppManager::deleteApps() {
    for (AppModel* app : apps) {
        delete app;
    }
    apps.clear();
}

int AppManager::generatePort() {
    std::lock_guard <std::mutex> lock(mt);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(5000, 6000);
    int port;
    bool isUnique = false;
    while (!isUnique) {
        port = distrib(gen);
        isUnique = isPortUnique(port);
    }
    return port;
}

bool AppManager::isPortUnique(int port) {
    for (AppModel* app : apps) {
        if (app->getPort() == port) {
            return false;
        }
    }
    return true;
}
void AppManager::addApp(AppModel* app) {
    apps.push_back(app);
}

void AppManager::startAllApps() {
    setzeroVC();
    for (AppModel* app : apps) {
        app->start_recv();
    }

}
void AppManager::setzeroVC()
{
    VectorClock zerovc(AppNumber, 0);
    for (int i = 0; i < AppNumber; i++) {
        zerovc.add_value(apps[i]->getPort(), 0);
    }
    for (int i = 0; i < AppNumber; i++) {
        apps[i]->setVC(zerovc);
    }
}
void AppManager::stopAllApps() {
    for (AppModel* app : apps) {
        app->stop_recv();
    }
}

void AppManager::run() {
    std::random_device rd;
    std::uniform_int_distribution<size_t> dist(0, apps.size() - 1);
    std::vector<std::mt19937> generators(apps.size());  // —оздаем вектор генераторов случайных чисел

    for (size_t i = 0; i < apps.size(); ++i) {
        generators[i] = std::mt19937(rd());  // »нициализируем генераторы случайных чисел
    }

    std::vector<std::thread> threads;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();  // «апоминаем начальное врем€
    for (size_t i = 0; i < apps.size(); ++i) {
        threads.emplace_back([&, i]() {  // «ахватываем генератор по ссылке и индекс по значению
            std::mt19937& gen = generators[i];  // ѕолучаем ссылку на генератор случайных чисел
            while (reqnumber > 0) {
                size_t index = dist(gen);
                AppModel* randomApp = getRandomApp(apps[i]);
                apps[i]->sendDataToRemoteAppModel(randomApp->getAddr(), randomApp->getPort());
                reqnumber--;
                std::this_thread::sleep_for(std::chrono::milliseconds(period));
            }
            });
    }
    for (std::thread& thread : threads) {

        thread.join();
    }
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();  // «апоминаем конечное врем€

    std::chrono::duration<double> duration = endTime - startTime;  // ¬ычисл€ем продолжительность выполнени€ в секундах
    std::cout << "Total execution time: " << duration.count() << " seconds" << std::endl;


}
AppModel* AppManager::getRandomApp(AppModel* app) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, apps.size() - 1);

        while (true) {
            size_t index = dist(gen);
            AppModel* randomApp = apps[index];
            if (randomApp != app) {
                return randomApp;
            }
        }
}