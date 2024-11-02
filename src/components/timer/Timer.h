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
      typedef std::chrono::milliseconds duration;

      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void StartTimer(duration duration);

      void StopTimer();

      duration GetTimeRemaining();

      bool IsRunning();

    private:
      TimerHandle_t timer;
    };
  }
}
