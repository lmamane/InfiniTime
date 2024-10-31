#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>
#include <chrono>
#include "portmacro_cmsis.h"

#include "systemtask/SystemTask.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "Symbols.h"
#include "StopWatch.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class AviationTimer : public Screen {
      public:
        explicit AviationTimer(System::SystemTask& systemTask,
			       Controllers::DateTime& dateTimeController);
        ~AviationTimer() override;
        void Refresh() override;

	void flightRulesBtnEventHandler();
	void flightStateBtnEventHandler();
	// BEGIN these are from StopWatch, should disappear
        void playPauseBtnEventHandler();
        void stopLapBtnEventHandler();
        bool OnButtonPushed() override;
	// END   these are from StopWatch, should disappear

      protected:
	enum class FlightState { off, idleAfterStartup, blocksOff, departed, landed,  blocksOn, idleBeforeShutdown};
	static constexpr const char * const offLabelStr = "off";
	static constexpr const char * const idleAfterStartupLabelStr = "Startup";
	static constexpr const char * const blocksOffLabelStr = "Blocks Off";
	static constexpr const char * const departedLabelStr = "in air";
	static constexpr const char * const landedLabelStr = "landed";
	static constexpr const char * const blocksOnLabelStr = "Blocks On";
	static constexpr const char * const idleBeforeShutdownLabelStr = "Shutdown";
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
	lv_obj_t *btnFlightState, *txtFlightState, *btnFlightRules, *txtFlightRules;
	lv_obj_t *txtStartDate, *txtBlockTime, *txtAirTime, *txtIFRTime;
	static constexpr const char * const IFRStartFmt = "IFR since %s";
	static constexpr const char * const IFREndFmt = "IFR e%s %dh%02dm%02d";
	static constexpr const char * const FlightDateFmt = "%s";
	static constexpr const char * const BlockTimeFmt = "B %s - %s";

	// TODO: choose between TickType_t or TimeSeparated_t
	std::chrono::nanoseconds previousIFRTime = std::chrono::nanoseconds(0);
	// TODO: does this need to be a Utility::DirtyValue<>??? What is that?
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> BlocksOffTime;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> DepartureTime;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> LandingTime;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> BlocksOnTime;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> IFRStartTime;

	void StartIFR(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_time);
	void StopIFR(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> stop_time);

	void StartIFR();
	void StopIFR();

	void blocksOff();
	void idleAfterStartup();

	Controllers::DateTime& dateTimeController;

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
        return new Screens::AviationTimer(*controllers.systemTask,
					  controllers.dateTimeController);
      };
    };
  }
}
