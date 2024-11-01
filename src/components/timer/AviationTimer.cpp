#include "components/timer/AviationTimer.h"

using namespace Pinetime::Controllers;

AviationTimer::AviationTimer(void* const timerData, TimerCallbackFunction_t timerCallbackFunction) :
  Timer(timerData, timerCallbackFunction, "AviationTimer")
{
}

