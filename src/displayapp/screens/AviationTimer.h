#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

#include <FreeRTOS.h>

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "components/timer/AviationTimer.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class AviationTimer : public Screen {
      public:
        explicit AviationTimer(Controllers::DateTime& dateTimeController,
			       Controllers::AviationTimer& aviationTimer);
        ~AviationTimer() override;
        void Refresh() override;
        void TimerDone();

	void flightRulesBtnEventHandler();
	void flightStateBtnEventHandler();

      protected:
	using time_point = Pinetime::Controllers::AviationTimer::time_point;
	using duration = Pinetime::Controllers::AviationTimer::duration;
	using FlightRules = Pinetime::Controllers::AviationTimer::FlightRules;
	using FlightState = Pinetime::Controllers::AviationTimer::FlightState;
	static constexpr const char * const offLabelStr = "off";
	static constexpr const char * const idleAfterStartupLabelStr = "Startup";
	static constexpr const char * const blocksOffLabelStr = "Blocks Off";
	static constexpr const char * const departedLabelStr = "in air";
	static constexpr const char * const landedLabelStr = "landed";
	static constexpr const char * const blocksOnLabelStr = "Blocks On";
	static constexpr const char * const idleBeforeShutdownLabelStr = "Shutdown";
	// TOOD: make these configurable
	static constexpr auto idleAfterStartupDuration   = std::chrono::seconds(60);
	static constexpr auto idleBeforeShutdownDuration = std::chrono::seconds(120);
	// END TODO
	static constexpr const char * const VFRLabelStr = "VFR";
	static constexpr const char * const IFRLabelStr = "IFR";
	lv_obj_t *btnFlightState, *txtFlightState, *btnFlightRules, *txtFlightRules;
	lv_obj_t *txtStartDate, *txtBlockTime, *txtAirTime, *txtBlockDuration, *txtAirDuration;
	lv_obj_t *txtShowTimer, *txtIFRTime;
	lv_task_t* taskRefresh;
	static constexpr const char * const IFRStartFmt = "IFR since %s";
	static constexpr const char * const IFREndFmt = "IFR e%s %dh%02dm%02d";
	static constexpr const char * const FlightDateFmt = "%s";
	static constexpr const char * const BlockTimeFmt = "B %s - %s";
	static constexpr const char * const AirTimeFmt = "A %s - %s";
	static constexpr const char * const BlockDurationFmt = "B%2dh%02dm%02d";
	static constexpr const char * const AirDurationFmt = "A%2dh%02dm%02d";

	void StartIFR();
	void StopIFR();

	void showIFRStart();
	void showIFRDuration();

	void blocksOff();
	void blocksOn();
	void takeoff();
	void land();
	void shutdown();
	void idleAfterStartup();
	void idleBeforeShutdown();

	void showBlocksOffTime();
	void showBlockDuration();
	void showTakeoffTime();
	void showAirTime();

	void newFlight();

	void Redraw();

	Controllers::DateTime& dateTimeController;
	Controllers::AviationTimer& aviationTimerController;

      };
    }

    template <>
    struct AppTraits<Apps::AviationTimer> {
      static constexpr Apps app = Apps::AviationTimer;
      static constexpr const char* icon = Screens::Symbols::plane;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::AviationTimer(controllers.dateTimeController,
					  controllers.aviationTimer);
      };
    };
  }
}
