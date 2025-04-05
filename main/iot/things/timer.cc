#include "iot/thing.h"
#include "board.h"
#include "application.h"
#include "assets/lang_config.h"

#include <esp_log.h>

#define TAG "Timer"

namespace iot {

class Timer : public Thing {
private:
    bool running = false;

public:
    Timer() : Thing("Timer", "Timer") {
        properties_.AddNumberProperty("TimerRunning", "Returns a boolean indicating if there is a timer running or not", [this]() -> bool {
                return running;
        });
        methods_.AddMethod("StartTimer", "Start or set a timer", ParameterList({
            Parameter("duration", "Timer duration in seconds as a whole number integer", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
                // start a timer and make a noise
           int duration = static_cast<int>(parameters["duration"].number()); 
           auto& app = Application::GetInstance();
           // We don't want to listen etc, but we also don't want to go to sleep
           app.SetDeviceState(kDeviceStateUnknown);
           

           ESP_LOGI(TAG, "Starting timer for %d seconds", duration);

           running = true;
           vTaskDelay(duration * 1000 / portTICK_PERIOD_MS);
           running = false;

           ESP_LOGI(TAG, "Done");

           app.Alert(Lang::Strings::INFO, "Timer finished", "happy", Lang::Sounds::P3_SUCCESS); 
           app.SetDeviceState(kDeviceStateIdle);

           ESP_LOGI(TAG, "Done");
        });
    }
};

} // namespace iot

DECLARE_THING(Timer);
