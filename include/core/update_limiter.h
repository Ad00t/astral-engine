#ifndef UPDATE_LIMITER_H
#define UPDATE_LIMITER_H

#include <chrono>

class UpdateLimiter {
    private:
        std::chrono::nanoseconds minUpdateDuration;

    public:
        std::chrono::steady_clock::time_point updateStart;

        UpdateLimiter(double maxUpdateRate);
        UpdateLimiter();
        ~UpdateLimiter();

        void setMaxUpdateRate(double maxUpdateRate);
        void startUpdate();
        void endUpdate();
};

#endif
