#pragma once

#include "components/timer/Timer.h"

namespace Pinetime {
  namespace Controllers {
    class AviationTimer : public Timer {
    public:
      AviationTimer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

    };
  }
}
