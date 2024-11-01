#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace Controllers {
    class Timer {
    protected:
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction, const char * const timerName);

    public:
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void StartTimer(std::chrono::milliseconds duration);

      void StopTimer();

      std::chrono::milliseconds GetTimeRemaining();

      bool IsRunning();

    private:
      TimerHandle_t timer;
    };
  }
}
