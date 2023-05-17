#pragma once
#include <iostream>
#include <string>
#include <map>

class VectorClock {
public:
    VectorClock(int num_apps, int port);
    VectorClock();
    VectorClock(const VectorClock& other);
    void increment(int port);
    void update(const VectorClock& other);
    void add_value(int port, int value);
    bool operator<=(const VectorClock& other) const;
    void setAppId(int port);
    int getValueByPort(int port);
    std::map<int, int> get_clock() const;
    std::string to_string() const;

    void from_string(const std::string& str);
private:
    int num_apps_;
    int app_id_;
    std::map<int, int> clock_;
};
