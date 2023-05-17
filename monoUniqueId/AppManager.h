#pragma once
#include <vector>
#include "AppModel.h"

class AppManager {
public:
    AppManager(int number_of_apps, int reqnumber, int period);
    ~AppManager();
    void createApps();
    void deleteApps();

    int generatePort();

    bool isPortUnique(int port);
    void addApp(AppModel* app);

    void startAllApps();
    void setzeroVC();
    void stopAllApps();

    void run();

private:
    std::mutex mt;
    int AppNumber;
    int reqnumber;
    int period;
    std::vector<AppModel*> apps;
    std::shared_ptr<MessageQueue> Que;
    std::shared_ptr<UniqueIdGenerator> id_gen;
    AppModel* getRandomApp(AppModel* app);
};
