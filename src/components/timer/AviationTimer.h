#pragma once

#include <chrono>
#include <cassert>

#include "components/timer/Timer.h"

namespace Pinetime {
  namespace Controllers {
    class AviationTimer : protected Timer {
    public:
      AviationTimer(void* timerData, TimerCallbackFunction_t timerCallbackFunction)
	: Timer(timerData, timerCallbackFunction, "AviationTimer") {
      }

      enum class FlightState { beforeStartup, idleAfterStartup, blocksOff, departed, landed,  blocksOn, idleBeforeShutdown, afterShutdown};
      enum class FlightRules { VFR, IFR };

      typedef std::chrono::nanoseconds duration;
      typedef std::chrono::time_point<std::chrono::system_clock, duration> time_point;

      FlightState getFlightState() {
	return currentFlightState;
      }

      FlightRules getFlightRules() {
	return currentFlightRules;
      }

      /* return value:
       * true if IFR started at start_time
       * false if it will start at future BlocksOffTime
       */
      bool StartIFR(const time_point start_time) {
	currentFlightRules = FlightRules::IFR;
	if(currentFlightState >= FlightState::blocksOff &&
	   currentFlightState < FlightState::blocksOn) {
	  IFRStartStopTime = start_time;
	  return true;
	} else {
	  return false;
	}
      }

      void EndIFR(const time_point stop_time) {
	assert(isIFRInProgress());
	previousIFRTime += stop_time - IFRStartStopTime;
	IFRStartStopTime = stop_time;
      }

      /* return value:
       * true if IFR was in progress and has now stopped
       * false if we merely will start future flight VFR
       */
      bool StopIFR(const time_point stop_time) {
	assert(currentFlightRules == FlightRules::IFR);
	const bool wasIFRInProgress = isIFRInProgress();
	if(wasIFRInProgress) {
	  EndIFR(stop_time);
	}
	currentFlightRules = FlightRules::VFR;
	return wasIFRInProgress;
      }

      time_point getIFRStartTime() {
	if(isIFRInProgress()) {
	  return IFRStartStopTime;
	} else {
	  return time_point();
	}
      }

      time_point getIFRStopTime() {
	if(hasIFREnded()) {
	  return IFRStartStopTime;
	} else {
	  return time_point();
	}
      }

      duration getIFRDuration() {
	if(hasIFREnded()) {
	  return previousIFRTime;
	} else {
	  return duration(0);
	}
      }

      time_point getBlocksOffTime() {
	if(currentFlightState >= FlightState::blocksOff) {
	  return BlocksOffTime;
	} else {
	  return time_point();
	}
      }

      time_point getTakeoffTime() {
	if(currentFlightState >= FlightState::departed) {
	  return TakeoffTime;
	} else {
	  return time_point();
	}
      }

      time_point getLandingTime() {
	if(currentFlightState >= FlightState::landed) {
	  return LandingTime;
	} else {
	  return time_point();
	}
      }

      time_point getBlocksOnTime() {
	if(currentFlightState >= FlightState::blocksOn) {
	  return BlocksOnTime;
	} else {
	  return time_point();
	}
      }

      void newFlight () {
	previousIFRTime = duration(0);
	BlocksOffTime = time_point();
	TakeoffTime = time_point();
	LandingTime = time_point();
	BlocksOnTime = time_point();
	IFRStartStopTime = time_point();
	currentFlightState = FlightState::beforeStartup;
      }

      void blocksOff(const time_point time) {
	assert(currentFlightState < FlightState::blocksOff);
	currentFlightState = FlightState::blocksOff;
	BlocksOffTime = time;
	if (currentFlightRules == FlightRules::IFR) {
	  StartIFR(BlocksOffTime);
	}
      }

      void blocksOn(const time_point time) {
	assert(currentFlightState == FlightState::landed);
	BlocksOnTime = time;
	if (currentFlightRules == FlightRules::IFR) {
	  EndIFR(BlocksOnTime);
	}
	currentFlightState = FlightState::blocksOn;
      }

      void takeoff(const time_point time) {
	assert(currentFlightState == FlightState::blocksOff);
	currentFlightState = FlightState::departed;
	TakeoffTime = time;
      }

      void land(const time_point time) {
	assert(currentFlightState == FlightState::departed);
	currentFlightState = FlightState::landed;
	LandingTime = time;
      }

      void shutdown() {
	assert(currentFlightState >= FlightState::blocksOn &&
	       currentFlightState <  FlightState::afterShutdown);
	currentFlightState = FlightState::afterShutdown;
      }

      void idleAfterStartup(Timer::duration duration) {
	assert(currentFlightState == FlightState::beforeStartup);
	currentFlightState = FlightState::idleAfterStartup;
	if (IsRunning()) {
	  StopTimer();
	}
	StartTimer(duration);
      }

      void idleBeforeShutdown(Timer::duration duration) {
	assert(currentFlightState == FlightState::blocksOn);
	currentFlightState = FlightState::idleBeforeShutdown;
	if (IsRunning()) {
	  StopTimer();
	}
	StartTimer(duration);
      }

      Timer::duration GetTimerTimeRemaining() {
	return GetTimeRemaining();
      }

      bool IsTimerRunning() {
	return IsRunning();
      }

      bool isIFRInProgress() {
	return currentFlightRules == FlightRules::IFR &&
	  currentFlightState >= FlightState::blocksOff &&
	  currentFlightState < FlightState::blocksOn;
      }

      bool hasIFREnded() {
	return currentFlightRules == FlightRules::VFR ||
	  currentFlightState >= FlightState::blocksOn;
      }

    protected:
      FlightState currentFlightState = FlightState::beforeStartup;
      FlightRules currentFlightRules = FlightRules::VFR;

      duration previousIFRTime = duration(0);

      time_point BlocksOffTime;
      time_point TakeoffTime;
      time_point LandingTime;
      time_point BlocksOnTime;
      time_point IFRStartStopTime;

    };
  }
}
