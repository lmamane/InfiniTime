#include "displayapp/screens/AviationTimer.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  using namespace std::chrono;

  void flight_rules_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<AviationTimer*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->flightRulesBtnEventHandler();
    }
  }

  void flight_state_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<AviationTimer*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->flightStateBtnEventHandler();
    }
  }

  template <typename clock, typename t_precision, typename r_precision>
  std::string fmt_hhmmpd(const time_point<clock, r_precision> ref,
                         const time_point<clock, t_precision> tp) {
    const auto ref_days = floor<days>(ref);
    const auto tp_days = floor<days>(tp);
    const hh_mm_ss time{tp - tp_days};
    return std::format("{:%R}{:+d}", tp, (tp_days - ref_days).count());
  }

}

AviationTimer::AviationTimer(Controllers::DateTime& dateTimeController,
			     Controllers::AviationTimer& aviationTimer)
  : dateTimeController {dateTimeController}
  , aviationTimerController (aviationTimer) {
  static constexpr uint8_t btnWidth = 76;
  static constexpr uint8_t btnHeight = 50;
  btnFlightRules = lv_btn_create(lv_scr_act(), nullptr);
  btnFlightRules->user_data = this;
  lv_obj_set_event_cb(btnFlightRules, flight_rules_event_handler);
  lv_obj_set_size(btnFlightRules, btnWidth, btnHeight);
  lv_obj_align(btnFlightRules, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  txtFlightRules = lv_label_create(btnFlightRules, nullptr);
  lv_label_set_text_static(txtFlightRules, VFRLabelStr);

  btnFlightState = lv_btn_create(lv_scr_act(), nullptr);
  btnFlightState->user_data = this;
  lv_obj_set_event_cb(btnFlightState, flight_state_event_handler);
  lv_obj_set_size(btnFlightState, 2*btnWidth, btnHeight);
  lv_obj_align(btnFlightState, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtFlightState = lv_label_create(btnFlightState, nullptr);
  lv_label_set_text_static(txtFlightState, offLabelStr);

  txtIFRTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtIFRTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(txtIFRTime, "");
  lv_label_set_long_mode(txtIFRTime, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtIFRTime, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtIFRTime, LV_HOR_RES_MAX);
  lv_obj_align(txtIFRTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -btnHeight);

  txtStartDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtStartDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtStartDate, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtStartDate, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(txtStartDate, LV_HOR_RES_MAX);
  lv_obj_align(txtStartDate, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  txtBlockTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtBlockTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtBlockTime, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtBlockTime, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtBlockTime, LV_HOR_RES_MAX);
  lv_obj_align(txtBlockTime, txtStartDate, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  txtAirTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtAirTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtAirTime, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtAirTime, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtAirTime, LV_HOR_RES_MAX);
  lv_obj_align(txtAirTime, txtBlockTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  txtBlockDuration = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtBlockDuration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtBlockDuration, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtBlockDuration, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtBlockDuration, LV_HOR_RES_MAX/2);
  lv_obj_align(txtBlockDuration, txtAirTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  txtAirDuration = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtAirDuration, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtAirDuration, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtAirDuration, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtAirDuration, LV_HOR_RES_MAX/2);
  lv_obj_align(txtAirDuration, txtBlockDuration, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);

  txtShowTimer = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtShowTimer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_set_style_local_text_font(txtShowTimer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_long_mode(txtShowTimer, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtShowTimer, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(txtShowTimer, LV_HOR_RES_MAX);
  lv_obj_align(txtShowTimer, txtBlockDuration, LV_ALIGN_OUT_BOTTOM_LEFT, 0, lv_obj_get_height(txtShowTimer)/2);

  newFlight();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

AviationTimer::~AviationTimer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void AviationTimer::StartIFR() {
  StartIFR(dateTimeController.UTCDateTime());
}

void AviationTimer::StopIFR() {
  StopIFR(dateTimeController.UTCDateTime());
}

void AviationTimer::StartIFR(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_time) {
  currentFlightRules = FlightRules::IFR;
  lv_label_set_text_static(txtFlightRules, IFRLabelStr);
  if(currentFlightState >= FlightState::blocksOff) {
    IFRStartTime = start_time;
    lv_label_set_text_fmt(txtIFRTime, IFRStartFmt, fmt_hhmmpd(BlocksOffTime, IFRStartTime).c_str());
  }
}

void AviationTimer::StopIFR(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> IFRStopTime) {
  currentFlightRules = FlightRules::VFR;
  lv_label_set_text_static(txtFlightRules, VFRLabelStr);
  if(currentFlightState >= FlightState::blocksOff) {
    previousIFRTime += IFRStopTime - IFRStartTime;
    const hh_mm_ss pITSep {round<seconds>(previousIFRTime)};
    // TODO: make utility function out of hms
    lv_label_set_text_fmt(txtIFRTime, IFREndFmt, fmt_hhmmpd(BlocksOffTime, IFRStopTime).c_str(), pITSep.hours(), pITSep.minutes(), pITSep.seconds());
  }
}

void AviationTimer::blocksOff() {
  lv_label_set_text_static(txtFlightState, blocksOffLabelStr);
  currentFlightState = FlightState::blocksOff;
  BlocksOffTime = dateTimeController.UTCDateTime();
  lv_label_set_text_fmt(txtStartDate, FlightDateFmt, std::format("{:%F}", BlocksOffTime).c_str());
  lv_label_set_text_fmt(txtBlockTime, BlockTimeFmt, fmt_hhmmpd(BlocksOffTime, BlocksOffTime).c_str(), "");
  if (currentFlightRules == FlightRules::IFR) {
    StartIFR(BlocksOffTime);
  }
}

void AviationTimer::blocksOn() {
  lv_label_set_text_static(txtFlightState, blocksOnLabelStr);
  currentFlightState = FlightState::blocksOn;
  BlocksOnTime = dateTimeController.UTCDateTime();
  lv_label_set_text_fmt(txtBlockTime, BlockTimeFmt,
			fmt_hhmmpd(BlocksOffTime, BlocksOffTime).c_str(),
			fmt_hhmmpd(BlocksOffTime, BlocksOnTime).c_str());
  const hh_mm_ss blocktime(round<seconds>(BlocksOnTime-BlocksOffTime));
  lv_label_set_text_fmt(txtBlockDuration, BlockDurationFmt,
			blocktime.hours(), blocktime.minutes(), blocktime.seconds());
  if (currentFlightRules == FlightRules::IFR) {
    StopIFR(BlocksOnTime);
  }
}

void AviationTimer::takeoff() {
  lv_label_set_text_static(txtFlightState, departedLabelStr);
  currentFlightState = FlightState::departed;
  TakeoffTime = dateTimeController.UTCDateTime();
  lv_label_set_text_fmt(txtAirTime, AirTimeFmt, fmt_hhmmpd(BlocksOffTime, TakeoffTime).c_str(), "");
}

void AviationTimer::land() {
  lv_label_set_text_static(txtFlightState, landedLabelStr);
  currentFlightState = FlightState::landed;
  LandingTime = dateTimeController.UTCDateTime();
  lv_label_set_text_fmt(txtAirTime, AirTimeFmt,
			fmt_hhmmpd(BlocksOffTime, TakeoffTime).c_str(),
			fmt_hhmmpd(BlocksOffTime, LandingTime).c_str());
  const hh_mm_ss airtime(round<seconds>(LandingTime-TakeoffTime));
  lv_label_set_text_fmt(txtAirDuration, AirDurationFmt,
			airtime.hours(), airtime.minutes(), airtime.seconds());
}

void AviationTimer::shutdown() {
  lv_label_set_text_static(txtFlightState, offLabelStr);
  currentFlightState = FlightState::off;
}

void AviationTimer::idleAfterStartup() {
  lv_label_set_text_static(txtFlightState, idleAfterStartupLabelStr);
  currentFlightState = FlightState::idleAfterStartup;
  aviationTimerController.StartTimer(idleAfterStartupDuration);
}

void AviationTimer::idleBeforeShutdown() {
  lv_label_set_text_static(txtFlightState, idleBeforeShutdownLabelStr);
  currentFlightState = FlightState::idleBeforeShutdown;
  aviationTimerController.StartTimer(idleBeforeShutdownDuration);
}

void AviationTimer::newFlight() {
  lv_label_set_text_static(txtIFRTime, "");
  lv_label_set_text_static(txtStartDate, "");
  lv_label_set_text_static(txtBlockTime, "");
  lv_label_set_text_static(txtAirTime, "");
  lv_label_set_text_static(txtBlockDuration, "");
  lv_label_set_text_static(txtAirDuration, "");

  TimerDone();

  previousIFRTime = std::chrono::nanoseconds(0);
}

void AviationTimer::TimerDone() {
  lv_label_set_text_static(txtShowTimer, "");
}

void AviationTimer::Refresh() {
  if (aviationTimerController.IsRunning()) {
    const hh_mm_ss timesep(aviationTimerController.GetTimeRemaining());
    lv_label_set_text_fmt(txtShowTimer, "%02d:%02d", timesep.hours() * 60 + timesep.minutes(), timesep.seconds());
  }
}

void AviationTimer::flightRulesBtnEventHandler() {
  switch (currentFlightRules) {
  case FlightRules::VFR:
    StartIFR();
    break;
  case FlightRules::IFR:
    StopIFR();
    break;
  }
}

void AviationTimer::flightStateBtnEventHandler() {
  using enum FlightState;
  switch (currentFlightState) {
  case off:
    newFlight();
    if (idleAfterStartupDuration > std::chrono::seconds::zero()) {
      AviationTimer::idleAfterStartup();
    }
    else {
      AviationTimer::blocksOff();
    }
    break;
  case idleAfterStartup:
    AviationTimer::blocksOff();
    break;
  case blocksOff:
    AviationTimer::takeoff();
    break;
  case departed:
    AviationTimer::land();
    break;
  case landed:
    AviationTimer::blocksOn();
    break;
  case blocksOn:
    if (idleBeforeShutdownDuration > std::chrono::seconds::zero()) {
      AviationTimer::idleBeforeShutdown();
    }
    else {
      AviationTimer::shutdown();
    }
    break;
  case idleBeforeShutdown:
    AviationTimer::shutdown();
    break;
  }
}

