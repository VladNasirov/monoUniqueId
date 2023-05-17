#pragma once
#include "AppManager.h"

int main()
{
    int number_of_apps = 10;
    int period = 100; // milliseconds
    int requestsnumber = 1000;
    bool run = true;

    AppManager Run(number_of_apps, requestsnumber, period);
    Run.run();
    return 0;
}
