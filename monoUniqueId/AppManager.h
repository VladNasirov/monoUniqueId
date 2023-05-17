#pragma once
#include <vector>
#include "AppModel.h"

class AppManager {
public:
    AppManager(int number_of_apps, int reqnumber, int period) : AppNumber(number_of_apps), reqnumber(reqnumber), period(period) 
    { 
        createApps(); 
        startAllApps();
    }
    ~AppManager() {
        stopAllApps();
        deleteApps();
    }
    void createApps() {
        for (int i = 0; i < AppNumber; i++) {
            AppModel* app = new AppModel("127.0.0.1", generatePort(), Que, id_gen);
            addApp(app);
        }
    }
    void deleteApps() {
        for (AppModel* app : apps) {
            delete app;
        }
        apps.clear();
    }

    int generatePort() {
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

    bool isPortUnique(int port) {
        for (AppModel* app : apps) {
            if (app->getPort() == port) {
                return false;
            }
        }
        return true;
    }
    void addApp(AppModel* app) {
        apps.push_back(app);
    }

    void startAllApps() {
        setzeroVC();
        for (AppModel* app : apps) {
            app->start_recv();
        }
    }
    void setzeroVC()
    {
        VectorClock zerovc(AppNumber, 0);
        for (int i = 0; i < AppNumber; i++) {
            zerovc.add_value(apps[i]->getPort(), 0);
        }
        for (int i = 0; i < AppNumber; i++) {
            apps[i]->setVC(zerovc);
        }
    }
    void stopAllApps() {
        for (AppModel* app : apps) {
            app->stop_recv();
        }
    }

    void run() {
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

private:
    std::mutex mt;
    int AppNumber;
    int reqnumber;
    int period;
    std::vector<AppModel*> apps;
    MessageQueue Que;
    UniqueIdGenerator id_gen;
    AppModel* getRandomApp(AppModel* app) {
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
};
