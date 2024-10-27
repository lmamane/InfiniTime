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

        void playPauseBtnEventHandler();
        void stopLapBtnEventHandler();
        bool OnButtonPushed() override;

      private:
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
        lv_obj_t *time, *msecTime, *btnPlayPause, *btnStopLap, *txtPlayPause, *txtStopLap;
        lv_obj_t* lapText;
        bool isHoursLabelUpdated = false;

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
