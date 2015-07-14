//
// Created by riku on 5/3/15.
//

#ifndef CCS2015_TIMER_HPP
#define CCS2015_TIMER_HPP
#include <chrono>
namespace MDL {
class Timer {
public:
    Timer() {
        acc = 0.0;
        bytes_nr = 0;
    }

    void start() {
        stamp = clock_::now();
    }

    void end() {
        acc += std::chrono::duration_cast<second_>(clock_::now() - stamp).count();
    }

    void end(size_t bytes_nr) {
        end();
        this->bytes_nr += bytes_nr;
    }

    void reset() {
        acc = 0.0;
        bytes_nr = 0;
        stamp = clock_::now();
    }

    double second() const {
        return acc;
    }

    double KB() const {
        return bytes_nr / 1024.0;
    }

    double MB() const {
        return KB() / 1024.0;
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> stamp;
    double acc;
    size_t bytes_nr;
};
}
#endif //CCS2015_TIMER_HPP
