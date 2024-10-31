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

  // TODO: share this with StopWatch, now is duplicate
  // maybe not used anymore in the end...
  TimeSeparated_t convertTicksToTimeSegments(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int hundredths = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = ((timeElapsedCentis / 100) / 60) % 60;
    const int hours = ((timeElapsedCentis / 100) / 60) / 60;
    return TimeSeparated_t {hours, mins, secs, hundredths};
  }

  // all the following should disappear
  void play_pause_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<AviationTimer*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->playPauseBtnEventHandler();
    }
  }

  void stop_lap_event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* stopWatch = static_cast<AviationTimer*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      stopWatch->stopLapBtnEventHandler();
    }
  }

  constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);
}

AviationTimer::AviationTimer(System::SystemTask& systemTask, Controllers::DateTime& dateTimeController)
  : systemTask {systemTask}
  ,  dateTimeController {dateTimeController} {
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
  lv_label_set_text_static(txtStartDate, "");
  lv_label_set_long_mode(txtStartDate, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtStartDate, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(txtStartDate, LV_HOR_RES_MAX);
  lv_obj_align(txtStartDate, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  txtBlockTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(txtBlockTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text_static(txtBlockTime, "");
  lv_label_set_long_mode(txtBlockTime, LV_LABEL_LONG_BREAK);
  lv_label_set_align(txtBlockTime, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_width(txtBlockTime, LV_HOR_RES_MAX);
  lv_obj_align(txtBlockTime, txtStartDate, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

AviationTimer::~AviationTimer() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void AviationTimer::SetInterfacePaused() {
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_label_set_text_static(txtStopLap, Symbols::stop);
}

void AviationTimer::SetInterfaceRunning() {
  lv_obj_set_state(time, LV_STATE_DEFAULT);
  lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
  lv_obj_set_style_local_bg_color(btnStopLap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);

  lv_obj_set_state(btnStopLap, LV_STATE_DEFAULT);
  lv_obj_set_state(txtStopLap, LV_STATE_DEFAULT);
}

void AviationTimer::SetInterfaceStopped() {
  lv_obj_set_state(time, LV_STATE_DISABLED);
  lv_obj_set_state(msecTime, LV_STATE_DISABLED);

  lv_label_set_text_static(time, "00:00");
  lv_label_set_text_static(msecTime, "00");

  if (isHoursLabelUpdated) {
    lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
    lv_obj_realign(time);
    isHoursLabelUpdated = false;
  }

  lv_label_set_text_static(txtStopLap, Symbols::lapsFlag);
  lv_obj_set_state(btnStopLap, LV_STATE_DISABLED);
  lv_obj_set_state(txtStopLap, LV_STATE_DISABLED);
}

void AviationTimer::StartIFR() {
  currentFlightRules = FlightRules::IFR;
  IFRStartTime = dateTimeController.UTCDateTime();
  lv_label_set_text_static(txtFlightRules, IFRLabelStr);
  lv_label_set_text_fmt(txtIFRTime, IFRStartFmt, fmt_hhmmpd(BlocksOffTime, IFRStartTime).c_str());
}

void AviationTimer::StopIFR() {
  currentFlightRules = FlightRules::VFR;
  const auto IFRStopTime = dateTimeController.UTCDateTime();;
  previousIFRTime += IFRStopTime - IFRStartTime;
  const hh_mm_ss pITSep {duration_cast<seconds>(previousIFRTime)};
  lv_label_set_text_static(txtFlightRules, VFRLabelStr);
  // TODO: make utility function out of hms
  lv_label_set_text_fmt(txtIFRTime, IFREndFmt, fmt_hhmmpd(BlocksOffTime, IFRStopTime).c_str(), pITSep.hours(), pITSep.minutes(), pITSep.seconds());
}

void AviationTimer::blocksOff() {
  lv_label_set_text_static(txtFlightState, blocksOffLabelStr);
  using namespace std::chrono;
  currentFlightState = FlightState::blocksOff;
  BlocksOffTime = dateTimeController.UTCDateTime();
  lv_label_set_text_fmt(txtBlockTime, BlockTimeFmt, fmt_hhmmpd(BlocksOffTime, BlocksOffTime).c_str(), "");
  lv_label_set_text_fmt(txtStartDate, FlightDateFmt, std::format("{:%F}", BlocksOffTime).c_str());
}

void AviationTimer::idleAfterStartup() {
  lv_label_set_text_static(txtFlightState, idleAfterStartupLabelStr);
  currentFlightState = FlightState::idleAfterStartup;
  // TODO
}

// START from StopWatch, should go away
void AviationTimer::Reset() {
  SetInterfaceStopped();
  currentState = States::Init;
  oldTimeElapsed = 0;
  lapsDone = 0;
}

void AviationTimer::Start() {
  SetInterfaceRunning();
  startTime = xTaskGetTickCount();
  currentState = States::Running;
  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void AviationTimer::Pause() {
  SetInterfacePaused();
  startTime = 0;
  // Store the current time elapsed in cache
  oldTimeElapsed = laps[lapsDone];
  blinkTime = xTaskGetTickCount() + blinkInterval;
  currentState = States::Halted;
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}
// END from StopWatch, should go away

void AviationTimer::Refresh() {
  if (currentState == States::Running) {
    laps[lapsDone] = oldTimeElapsed + xTaskGetTickCount() - startTime;

    TimeSeparated_t currentTimeSeparated = convertTicksToTimeSegments(laps[lapsDone]);
    if (currentTimeSeparated.hours == 0) {
      lv_label_set_text_fmt(time, "%02d:%02d", currentTimeSeparated.mins, currentTimeSeparated.secs);
    } else {
      lv_label_set_text_fmt(time, "%02d:%02d:%02d", currentTimeSeparated.hours, currentTimeSeparated.mins, currentTimeSeparated.secs);
      if (!isHoursLabelUpdated) {
        lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
        lv_obj_realign(time);
        isHoursLabelUpdated = true;
      }
    }
    lv_label_set_text_fmt(msecTime, "%02d", currentTimeSeparated.hundredths);
  } else if (currentState == States::Halted) {
    const TickType_t currentTime = xTaskGetTickCount();
    if (currentTime > blinkTime) {
      blinkTime = currentTime + blinkInterval;
      if (lv_obj_get_state(time, LV_LABEL_PART_MAIN) == LV_STATE_DEFAULT) {
        lv_obj_set_state(time, LV_STATE_DISABLED);
        lv_obj_set_state(msecTime, LV_STATE_DISABLED);
      } else {
        lv_obj_set_state(time, LV_STATE_DEFAULT);
        lv_obj_set_state(msecTime, LV_STATE_DEFAULT);
      }
    }
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
    if (idleAfterStartupDuration > 0) {
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
    break;
  case departed:
    break;
  case landed:
    break;
  case blocksOn:
    break;
  case idleBeforeShutdown:
    break;
  }
}

// BEGIN should be removed
void AviationTimer::playPauseBtnEventHandler() {
  if (currentState == States::Init || currentState == States::Halted) {
    Start();
  } else if (currentState == States::Running) {
    Pause();
  }
}

void AviationTimer::stopLapBtnEventHandler() {
  // If running, then this button is used to save laps
  if (currentState == States::Running) {
    lapsDone = std::min(lapsDone + 1, maxLapCount);
    for (int i = lapsDone - displayedLaps; i < lapsDone; i++) {
      if (i < 0) {
        continue;
      }
      TimeSeparated_t times = convertTicksToTimeSegments(laps[i]);
      char buffer[17];
      if (times.hours == 0) {
        snprintf(buffer, sizeof(buffer), "#%2d    %2d:%02d.%02d\n", i + 1, times.mins, times.secs, times.hundredths);
      } else {
        snprintf(buffer, sizeof(buffer), "#%2d %2d:%02d:%02d.%02d\n", i + 1, times.hours, times.mins, times.secs, times.hundredths);
      }
    }
  } else if (currentState == States::Halted) {
    Reset();
  }
}

bool AviationTimer::OnButtonPushed() {
  if (currentState == States::Running) {
    Pause();
    return true;
  }
  return false;
}
// END should be removed
