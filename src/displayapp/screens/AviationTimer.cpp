#include "displayapp/screens/AviationTimer.h"

#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

#include <compat/chrono>

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

  void tng_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<AviationTimer*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->TnGBtnEventHandler();
    }
  }

  template <typename clock, typename t_precision, typename r_precision>
  std::string fmt_hhmmpd(const time_point<clock, r_precision> ref,
                         const time_point<clock, t_precision> tp) {
    const auto ref_days = floor<days>(ref);
    const auto tp_days = floor<days>(tp);
    const hh_mm_ss time{tp - tp_days};
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
    return std::format("{:%R}{:+d}", tp, (tp_days - ref_days).count());
#else
    constexpr size_t bufsize = 15;
    char buf[bufsize];
    int nc = snprintf(buf, bufsize, "%s%02d:%02d+%d",
		      (time.is_negative() ? "-" : ""),
		      static_cast<int>(time.hours().count()),
		      static_cast<int>(time.minutes().count()),
		      static_cast<int>((tp_days - ref_days).count()));
    return std::string(buf, nc);
#endif
  }

  template <typename clock, typename precision>
  std::string fmt_yyyymmdd(const time_point<clock, precision> tp) {
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
    return std::format("{:%F}", tp);
#else
    const auto tpd(floor<days>(tp));
    const year_month_day ymd(tpd);
    constexpr size_t bufsize = 15;
    char buf[bufsize];
    int nc = snprintf(buf, bufsize, "%04d-%02u-%02u", int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
    return std::string(buf, nc);
#endif
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

  btnFlightState = lv_btn_create(lv_scr_act(), nullptr);
  btnFlightState->user_data = this;
  lv_obj_set_event_cb(btnFlightState, flight_state_event_handler);
  lv_obj_set_size(btnFlightState, btnWidth, btnHeight);
  lv_obj_align(btnFlightState, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtFlightState = lv_label_create(btnFlightState, nullptr);

  btnTnG = lv_btn_create(lv_scr_act(), nullptr);
  btnTnG->user_data = this;
  lv_obj_set_event_cb(btnTnG, tng_event_handler);
  lv_obj_set_size(btnTnG, btnWidth, btnHeight);
  lv_obj_align(btnTnG, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtTnG = lv_label_create(btnTnG, nullptr);
  lv_label_set_text_static(txtTnG, PLANEARRIVAL_SYMBOL PLANEDEPARTURE_SYMBOL);

  txtIFRTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtIFRTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
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
  lv_obj_align(txtShowTimer, txtBlockDuration, LV_ALIGN_OUT_BOTTOM_LEFT, 0, lv_obj_get_height(txtShowTimer)/3);

  txtLandingCounter = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtLandingCounter, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(txtLandingCounter, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtLandingCounter, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtLandingCounter, LV_HOR_RES_MAX);
  lv_obj_align(txtLandingCounter, txtIFRTime, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  Redraw();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

AviationTimer::~AviationTimer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void AviationTimer::StartIFR() {
  lv_label_set_text_static(txtFlightRules, IFRLabelStr);
  if (aviationTimerController.StartIFR(dateTimeController.UTCDateTime())) {
    showIFRStart();
  }
}

void AviationTimer::StopIFR() {
  lv_label_set_text_static(txtFlightRules, VFRLabelStr);
  if(aviationTimerController.StopIFR(dateTimeController.UTCDateTime())) {
    showIFRDuration();
  }
}

void AviationTimer::showIFRStart() {
  lv_label_set_text_fmt(txtIFRTime,
			IFRStartFmt,
			fmt_hhmmpd(aviationTimerController.getBlocksOffTime(),
				   aviationTimerController.getIFRStartTime()).c_str());
}

void AviationTimer::showIFRDuration() {
    const hh_mm_ss pITSep {round<seconds>(aviationTimerController.getIFRDuration())};
    lv_label_set_text_fmt(txtIFRTime,
                          IFREndFmt,
                          fmt_hhmmpd(aviationTimerController.getBlocksOffTime(),
                                     aviationTimerController.getIFRStopTime()).c_str(),
                          static_cast<int>(pITSep.hours().count()),
                          static_cast<int>(pITSep.minutes().count()),
                          static_cast<int>(pITSep.seconds().count()));
}

void AviationTimer::blocksOff() {
  lv_label_set_text_static(txtFlightState, blocksOffLabelStr);
  aviationTimerController.blocksOff(dateTimeController.UTCDateTime());
  showBlocksOffTime();
}

void AviationTimer::blocksOn() {
  lv_label_set_text_static(txtFlightState, blocksOnLabelStr);
  aviationTimerController.blocksOn(dateTimeController.UTCDateTime());
  showBlockDuration();
}

void AviationTimer::showBlocksOffTime() {
  const time_point BlocksOffTime(aviationTimerController.getBlocksOffTime());
  lv_label_set_text_fmt(txtStartDate, FlightDateFmt, fmt_yyyymmdd(BlocksOffTime).c_str());
  lv_label_set_text_fmt(txtBlockTime, BlockTimeFmt, fmt_hhmmpd(BlocksOffTime, BlocksOffTime).c_str(), "");
  if (aviationTimerController.isIFRInProgress()) {
    showIFRStart();
  }
}

void AviationTimer::showBlockDuration() {
  const time_point BlocksOffTime(aviationTimerController.getBlocksOffTime());
  const time_point BlocksOnTime (aviationTimerController.getBlocksOnTime ());
  if(lv_label_get_text(txtStartDate)[0] == '\0') {
    lv_label_set_text_fmt(txtStartDate, FlightDateFmt, fmt_yyyymmdd(BlocksOffTime).c_str());
  }
  lv_label_set_text_fmt(txtBlockTime, BlockTimeFmt,
			fmt_hhmmpd(BlocksOffTime, BlocksOffTime).c_str(),
			fmt_hhmmpd(BlocksOffTime, BlocksOnTime ).c_str());
  const hh_mm_ss blocktime(round<seconds>(BlocksOnTime - BlocksOffTime));
  lv_label_set_text_fmt(txtBlockDuration, BlockDurationFmt,
			static_cast<int>(blocktime.hours().count()),
			static_cast<int>(blocktime.minutes().count()),
			static_cast<int>(blocktime.seconds().count()));
  if (aviationTimerController.getFlightRules() == FlightRules::IFR) {
    showIFRDuration();
  }
}

void AviationTimer::takeoff() {
  lv_label_set_text_static(txtFlightState, departedLabelStr);
  aviationTimerController.takeoff(dateTimeController.UTCDateTime());
  showTakeoffTime();
}

void AviationTimer::land() {
  lv_label_set_text_static(txtFlightState, landedLabelStr);
  aviationTimerController.land(dateTimeController.UTCDateTime());
  showAirTime();
  lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
}

void AviationTimer::showTakeoffTime() {
  lv_label_set_text_fmt(txtAirTime,
			AirTimeFmt,
			fmt_hhmmpd(aviationTimerController.getBlocksOffTime(),
				   aviationTimerController.getTakeoffTime()).c_str(), "");
}

void AviationTimer::showAirTime() {
  const auto BlocksOffTime(aviationTimerController.getBlocksOffTime());
  const auto TakeoffTime(aviationTimerController.getTakeoffTime());
  const auto LandingTime(aviationTimerController.getLandingTime());
  lv_label_set_text_fmt(txtAirTime, AirTimeFmt,
			fmt_hhmmpd(BlocksOffTime, TakeoffTime).c_str(),
			fmt_hhmmpd(BlocksOffTime, LandingTime).c_str());
  const hh_mm_ss airtime(round<seconds>(LandingTime - TakeoffTime));
  lv_label_set_text_fmt(txtAirDuration, AirDurationFmt,
			static_cast<int>(airtime.hours().count()),
			static_cast<int>(airtime.minutes().count()),
			static_cast<int>(airtime.seconds().count()));
}

void AviationTimer::shutdown() {
  lv_label_set_text_static(txtFlightState, offLabelStr);
  aviationTimerController.shutdown();
}

void AviationTimer::idleAfterStartup() {
  lv_label_set_text_static(txtFlightState, idleAfterStartupLabelStr);
  aviationTimerController.idleAfterStartup(idleAfterStartupDuration);
}

void AviationTimer::idleBeforeShutdown() {
  lv_label_set_text_static(txtFlightState, idleBeforeShutdownLabelStr);
  aviationTimerController.idleBeforeShutdown(idleBeforeShutdownDuration);
}

void AviationTimer::newFlight() {
  aviationTimerController.newFlight();
  Redraw();
}

void AviationTimer::TimerDone() {
  lv_label_set_text_static(txtShowTimer, "");
}

void AviationTimer::Refresh() {
  if (aviationTimerController.IsTimerRunning()) {
    const hh_mm_ss timesep(aviationTimerController.GetTimerTimeRemaining());
    lv_label_set_text_fmt(txtShowTimer, "%02d:%02d",
			  static_cast<int>(timesep.hours().count() * 60 + timesep.minutes().count()),
			  static_cast<int>(timesep.seconds().count()));
  }
}

void AviationTimer::TnGBtnEventHandler() {
  aviationTimerController.touchAndGo();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
}

void AviationTimer::flightRulesBtnEventHandler() {
  switch (aviationTimerController.getFlightRules()) {
  case FlightRules::VFR:
    StartIFR();
    break;
  case FlightRules::IFR:
    StopIFR();
    break;
  }
}

void AviationTimer::flightStateBtnEventHandler() {
  switch (aviationTimerController.getFlightState()) {
  case FlightState::beforeStartup:
    if (idleAfterStartupDuration > std::chrono::seconds::zero()) {
      AviationTimer::idleAfterStartup();
    }
    else {
      AviationTimer::blocksOff();
    }
    break;
  case FlightState::idleAfterStartup:
    AviationTimer::blocksOff();
    break;
  case FlightState::blocksOff:
    AviationTimer::takeoff();
    break;
  case FlightState::departed:
    AviationTimer::land();
    break;
  case FlightState::landed:
    AviationTimer::blocksOn();
    break;
  case FlightState::blocksOn:
    if (idleBeforeShutdownDuration > std::chrono::seconds::zero()) {
      AviationTimer::idleBeforeShutdown();
    }
    else {
      AviationTimer::shutdown();
    }
    break;
  case FlightState::idleBeforeShutdown:
    AviationTimer::shutdown();
    break;
  case FlightState::afterShutdown:
    newFlight();
    break;
  }
}

void AviationTimer::Redraw() {
  lv_label_set_text_static(txtIFRTime, "");
  lv_label_set_text_static(txtStartDate, "");
  lv_label_set_text_static(txtBlockTime, "");
  lv_label_set_text_static(txtAirTime, "");
  lv_label_set_text_static(txtBlockDuration, "");
  lv_label_set_text_static(txtAirDuration, "");
  lv_label_set_text_static(txtLandingCounter, "");
  TimerDone();

  switch (aviationTimerController.getFlightRules()) {
  case FlightRules::VFR:
    lv_label_set_text_static(txtFlightRules, VFRLabelStr);
    if (aviationTimerController.getIFRDuration() > duration(0)) {
      showIFRDuration();
    }
    break;
  case FlightRules::IFR:
    lv_label_set_text_static(txtFlightRules, IFRLabelStr);
    break;
  }

  switch (aviationTimerController.getFlightState()) {
  case FlightState::beforeStartup:
    lv_label_set_text_static(txtFlightState, offLabelStr);
    break;
  case FlightState::idleAfterStartup:
    lv_label_set_text_static(txtFlightState, idleAfterStartupLabelStr);
    break;
  case FlightState::blocksOff:
    lv_label_set_text_static(txtFlightState, blocksOffLabelStr);
    showBlocksOffTime();
    break;
  case FlightState::departed:
    lv_label_set_text_static(txtFlightState, departedLabelStr);
    showBlocksOffTime();
    showTakeoffTime();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
    break;
  case FlightState::landed:
    lv_label_set_text_static(txtFlightState, landedLabelStr);
    showBlocksOffTime();
    showAirTime();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
    break;
  case FlightState::blocksOn:
    lv_label_set_text_static(txtFlightState, blocksOnLabelStr);
    showBlockDuration();
    showAirTime();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
    break;
  case FlightState::idleBeforeShutdown:
    lv_label_set_text_static(txtFlightState, idleBeforeShutdownLabelStr);
    showBlockDuration();
    showAirTime();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
    break;
  case FlightState::afterShutdown:
    lv_label_set_text_static(txtFlightState, offLabelStr);
    showBlockDuration();
    showAirTime();
    lv_label_set_text_fmt(txtLandingCounter, LandingCounterFmt, aviationTimerController.getLandingCount());
    break;
  }
}
