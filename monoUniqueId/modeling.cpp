#pragma once
#include "AppManager.h"

int main()
{
    int number_of_apps = 10;
    int period = 1000; // milliseconds
    int requestsnumber = 100;
    bool run = true;

    AppManager Run(number_of_apps, requestsnumber, period);
    Run.run();
    return 0;
}
