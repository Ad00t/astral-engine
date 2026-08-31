#include "update_limiter.h"
#include <chrono>
#include <thread>

UpdateLimiter::UpdateLimiter(double maxUpdateRate) {
    setMaxUpdateRate(maxUpdateRate);
}

UpdateLimiter::UpdateLimiter() {}
UpdateLimiter::~UpdateLimiter() {}

void UpdateLimiter::setMaxUpdateRate(double maxUpdateRate) {
    minUpdateDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double, std::nano>(1'000'000'000.0 / maxUpdateRate)
    );
}

void UpdateLimiter::startUpdate() {
    updateStart = std::chrono::steady_clock::now();
}

void UpdateLimiter::endUpdate() {
    auto updateEnd = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(updateEnd - updateStart);

    if (elapsedTime < minUpdateDuration) {
        auto remainingTime = minUpdateDuration - elapsedTime;

        auto sleepDuration = remainingTime - std::chrono::milliseconds(1);
        if (sleepDuration > std::chrono::nanoseconds::zero()) {
            std::this_thread::sleep_for(sleepDuration);
        }

        while (std::chrono::steady_clock::now() - updateStart < minUpdateDuration) {
            #if defined(__x86_64__) || defined(_M_X64)
            __builtin_ia32_pause();
            #endif
        }
    }
}
