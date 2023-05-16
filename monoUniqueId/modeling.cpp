#include "AppManager.h"
#include "memory"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>

int main()
{
    int number_of_apps = 5;
    int period = 10; // milliseconds
    int requestsnumber = 100;
    bool run = true;

    AppManager Run(number_of_apps, requestsnumber, period);
    Run.stop();
    return 0;
}
