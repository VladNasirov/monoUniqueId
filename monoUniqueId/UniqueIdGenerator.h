#pragma once
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <mutex>

class UniqueIdGenerator {
public:
    UniqueIdGenerator() {
        // Инициализация генератора случайных чисел
        std::random_device rd;
        generator_ = std::mt19937(rd());
    }

    UniqueIdGenerator(UniqueIdGenerator& other)
    {
        generator_=other.generator_;
        last_server_nanoseconds_ = other.last_server_nanoseconds_;
    }

    // Метод для генерации уникального идентификатора на основе временной метки
    std::string generateUniqueId(int server_id, std::chrono::nanoseconds server_time) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

        // Учитываем разрешающую способность системного таймера
        auto resolution = std::chrono::system_clock::period::den / std::chrono::system_clock::period::num;
        nanoseconds -= nanoseconds % resolution;

        // Генерируем случайное число
        std::uniform_int_distribution<std::uint64_t> distribution(0, std::numeric_limits<std::uint64_t>::max());
        std::uint64_t random_number = distribution(generator_);

        // Получаем текущее системное время на сервере
        auto server_nanoseconds = server_time.count();

        // Учитываем возможные скачки системного времени
        if (server_nanoseconds < last_server_nanoseconds_) {
            server_nanoseconds = last_server_nanoseconds_;
        }
        last_server_nanoseconds_ = server_nanoseconds;

        // Составляем идентификатор из временной метки, случайного числа и идентификатора сервера
        std::ostringstream os;
        os << std::setfill('0') << std::setw(20) << nanoseconds;
        os << std::setfill('0') << std::setw(20) << random_number;
        os << std::setfill('0') << std::setw(5) << server_id;
        os << std::setfill('0') << std::setw(20) << server_nanoseconds;
        return os.str();
    }

private:
    std::mt19937 generator_;
    std::mutex mtx_;
    uint64_t last_server_nanoseconds_ = 0;
};
