#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>
#include "portmacro_cmsis.h"

#include "systemtask/SystemTask.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "StopWatch.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class AviationTimer : public Screen {
      public:
        explicit AviationTimer(System::SystemTask& systemTask);
        ~AviationTimer() override;
        void Refresh() override;

	void flightRulesBtnEventHandler();
	// BEGIN these are from StopWatch, should disappear
        void playPauseBtnEventHandler();
        void stopLapBtnEventHandler();
        bool OnButtonPushed() override;
	// END   these are from StopWatch, should disappear

      protected:
	enum class FlightState { off, idleAfterStartup, blocksOff, departed, landed,  blocksOn, idleBeforeShutdown};
	FlightState currentFlightState = FlightState::off;
	// TOOD: make these configurable
	// FIXME: at state changee, don't forget to test idle durations and if zero, skip state entirely
	// TODO: need to check what the duration of "tick" in TickType_t is and adapt
	static constexpr int idleAfterStartupDuration   = 60;
	static constexpr int idleBeforeShutdownDuration = 120;
	// END TODO
	enum class FlightRules { VFR, IFR };
	FlightRules currentFlightRules = FlightRules::VFR;
	static constexpr const char * const VFRLabelStr = "VFR";
	static constexpr const char * const IFRLabelStr = "IFR";
	lv_obj_t *btnFlightState, *btnFlightRules, *txtFlightRules;
	lv_obj_t *txtBlockTime, *txtAirTime, *txtIFRTime;
	// TODO actual h:m time
	static constexpr const char * const IFRStartFmt = "IFR s%d:%02d d%d:%02d";
	static constexpr const char * const IFREndFmt = "IFR e%d:%02d d%d:%02d";

	// TODO: choose between TickType_t or TimeSeparated_t
	TickType_t previousIFRTime = 0; //= {0, 0, 0, 0};
	TickType_t IFRStartTime;

	void StartIFR();
	void StopIFR();

      private:
	// BEGIN these are from StopWatch, should disappear
        void SetInterfacePaused();
        void SetInterfaceRunning();
        void SetInterfaceStopped();

        void Reset();
        void Start();
        void Pause();

        Pinetime::System::SystemTask& systemTask;
        States currentState = States::Init;
        TickType_t startTime;
        TickType_t oldTimeElapsed = 0;
        TickType_t blinkTime = 0;
        static constexpr int maxLapCount = 20;
        TickType_t laps[maxLapCount + 1];
        static constexpr int displayedLaps = 2;
        int lapsDone = 0;
        lv_obj_t *time, *msecTime, *btnStopLap, *txtStopLap;
        lv_obj_t* lapText;
        bool isHoursLabelUpdated = false;
	// END    these are from StopWatch, should disappear

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::AviationTimer> {
      static constexpr Apps app = Apps::AviationTimer;
      static constexpr const char* icon = Screens::Symbols::plane;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::AviationTimer(*controllers.systemTask);
      };
    };
  }
}
