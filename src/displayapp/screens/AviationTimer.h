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

	void TnGBtnEventHandler();
	void flightRulesBtnEventHandler();
	void flightStateBtnEventHandler();

      protected:
	using time_point = Pinetime::Controllers::AviationTimer::time_point;
	using duration = Pinetime::Controllers::AviationTimer::duration;
	using FlightRules = Pinetime::Controllers::AviationTimer::FlightRules;
	using FlightState = Pinetime::Controllers::AviationTimer::FlightState;
	static constexpr const char * const offLabelStr = Screens::Symbols::planeCircleXMark;
	static constexpr const char * const idleAfterStartupLabelStr = HOURGLASS_SYMBOL PLANEDEPARTURE_SYMBOL;
	static constexpr const char * const blocksOffLabelStr = PLANEDEPARTURE_SYMBOL;
	static constexpr const char * const departedLabelStr = Screens::Symbols::plane;
	static constexpr const char * const landedLabelStr = Screens::Symbols::planeArrival;
	static constexpr const char * const blocksOnLabelStr = Screens::Symbols::planeLock;
	static constexpr const char * const idleBeforeShutdownLabelStr = HOURGLASS_SYMBOL PLANECIRCLEXMARK_SYMBOL;
	// TOOD: make these configurable
	static constexpr auto idleAfterStartupDuration   = std::chrono::seconds(60);
	static constexpr auto idleBeforeShutdownDuration = std::chrono::seconds(120);
	// END TODO
	static constexpr const char * const VFRLabelStr = Screens::Symbols::eye;
	static constexpr const char * const IFRLabelStr = Screens::Symbols::cloud;
	lv_obj_t *btnFlightState, *txtFlightState, *btnFlightRules, *txtFlightRules;
	lv_obj_t *txtStartDate, *txtBlockTime, *txtAirTime, *txtBlockDuration, *txtAirDuration;
	lv_obj_t *txtShowTimer, *txtIFRTime;
	lv_obj_t *btnTnG, *txtTnG, *txtLandingCounter;
	lv_task_t* taskRefresh;
	static constexpr const char * const IFRStartFmt = "IFR since %s";
	static constexpr const char * const IFREndFmt = "IFR e%s %dh%02dm%02d";
	static constexpr const char * const FlightDateFmt = "%s";
	static constexpr const char * const BlockTimeFmt = "B %s - %s";
	static constexpr const char * const AirTimeFmt = "A %s - %s";
	static constexpr const char * const BlockDurationFmt = "B%2dh%02dm%02d";
	static constexpr const char * const AirDurationFmt = "A%2dh%02dm%02d";
	static constexpr const char * const LandingCounterFmt = "Landings: %d";

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
      static constexpr const char* icon = Screens::Symbols::planeUp;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::AviationTimer(controllers.dateTimeController,
					  controllers.aviationTimer);
      };
    };
  }
}
