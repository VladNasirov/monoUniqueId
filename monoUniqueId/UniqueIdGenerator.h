#pragma once
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <mutex>

class UniqueIdGenerator {
public:
    UniqueIdGenerator();
    UniqueIdGenerator(UniqueIdGenerator& other);

    // ����� ��� ��������� ����������� �������������� �� ������ ��������� �����
    std::string generateUniqueId(int timestomp, int port, std::chrono::nanoseconds server_time);

private:
    std::mt19937 generator_;
    std::mutex mtx_;
    uint64_t last_server_nanoseconds_ = 0;
};
