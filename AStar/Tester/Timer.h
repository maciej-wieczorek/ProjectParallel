#pragma once
#include <chrono>

class Timer {
public:
    Timer() : start_time_(std::chrono::high_resolution_clock::now()) {}

    void reset()
    {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    double elapsed() const
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time = std::chrono::duration<double>(end_time - start_time_);
        return elapsed_time.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};
