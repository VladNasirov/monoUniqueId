#include "VectorClock.h"

VectorClock::VectorClock(int num_apps, int port)
    : num_apps_(num_apps),
    app_id_(port),
    clock_(std::map<int, int>()) {

}
VectorClock::VectorClock() :num_apps_(0), app_id_(0), clock_(std::map<int, int>()) {};
VectorClock::VectorClock(const VectorClock& other)
{
    num_apps_ = other.num_apps_;
    app_id_ = other.app_id_;
    clock_ = other.clock_;
}
void VectorClock::increment(int port) {
    clock_[port]++;
}
void VectorClock::update(const VectorClock& other) {
    for (auto it = other.clock_.cbegin(); it != other.clock_.cend(); ++it) {
        const int app_id = it->first;
        int other_clock = it->second;
        clock_[app_id] = std::max(clock_[app_id], other_clock);
    }
    increment(app_id_);
}
void VectorClock::add_value(int port, int value) {
    clock_[port] = value;
}
void VectorClock::setAppId(int port)
{
    app_id_ = port;
}
bool VectorClock::operator<=(const VectorClock& other) const {
    for (const auto& entry : clock_) {
        int app_id = entry.first;
        int clock_value = entry.second;
        if (clock_value > other.clock_.at(app_id)) {
            return false;
        }
    }
    return true;
}

std::map<int, int> VectorClock::get_clock() const {
    return clock_;
}
std::string VectorClock::to_string() const {
    std::string result = "";
    for (auto it = clock_.cbegin(); it != clock_.cend(); ++it) {
        const int app_id = it->first;
        int clock_value = it->second;
        result += std::to_string(app_id) + ":" + std::to_string(clock_value) + ",";
    }
    return result;
}

void VectorClock::from_string(const std::string& str) {
    clock_.clear();
    std::string app_id = "";
    std::string value_str = "";
    bool reading_id = true;
    for (char c : str) {
        if (c == ':') {
            reading_id = false;
        }
        else if (c == ',') {
            if (!app_id.empty() && !value_str.empty()) {
                clock_[std::stoi(app_id)] = std::stoi(value_str);
            }
            app_id = "";
            value_str = "";
            reading_id = true;
        }
        else {
            if (reading_id) {
                app_id += c;
            }
            else {
                value_str += c;
            }
        }
    }
    if (!app_id.empty() && !value_str.empty()) {
        clock_[std::stoi(app_id)] = std::stoi(value_str);
    }
}