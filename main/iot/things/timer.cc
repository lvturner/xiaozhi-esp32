#include "iot/thing.h"
#include "board.h"
#include "application.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "Timer"
#define TIMER_TASK_STACK_SIZE 3072  // Increased from 2048
#define TIMER_TASK_PRIORITY 5

namespace iot {

class Timer : public Thing {
private:
    bool running = false;
    TaskHandle_t timer_task_handle = nullptr;

    struct TimerParams {
        Timer* timer;
        int duration;
        Application* app;
    };

    static void timerTaskFunction(void* params) {
        TimerParams* p = static_cast<TimerParams*>(params);
        p->timer->running = true;
        
        // Check remaining stack space
        UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(nullptr);
        ESP_LOGI(TAG, "Timer task stack remaining: %u", stackRemaining);

        // Delay in the task, not the main thread
        vTaskDelay(p->duration * 1000 / portTICK_PERIOD_MS);
        
        p->timer->running = false;
        ESP_LOGI(TAG, "Timer done");
        
        // Keep alert message minimal to reduce stack usage
        p->app->Alert("Timer", "Timer finished", "happy", Lang::Sounds::P3_SUCCESS);
        vTaskDelay(500); // Delay to let sound etc go thru
        p->app->SetDeviceState(kDeviceStateListening);
        
        // Clean up
        p->timer->timer_task_handle = nullptr;
        delete p;
        vTaskDelete(nullptr);
    }

public:
    Timer() : Thing("Timer", "Timer") {
        properties_.AddNumberProperty("TimerRunning", "Returns a boolean indicating if there is a timer running or not", [this]() -> bool {
            return running;
        });
        
        methods_.AddMethod("StartTimer", "Start or set a timer", ParameterList({
            Parameter("duration", "Timer duration in seconds as a whole number integer", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            // Cancel any existing timer first
            if (timer_task_handle != nullptr) {
                vTaskDelete(timer_task_handle);
                timer_task_handle = nullptr;
                running = false;
            }

            int duration = static_cast<int>(parameters["duration"].number()); 
            auto& app = Application::GetInstance();
            app.SetDeviceState(kDeviceStateUnknown);
            
            ESP_LOGI(TAG, "Starting timer for %d seconds", duration);

            // Create a task for the timer
            TimerParams* params = new TimerParams{this, duration, &app};
            BaseType_t result = xTaskCreate(
                timerTaskFunction,
                "timer_task",
                TIMER_TASK_STACK_SIZE,
                params,
                TIMER_TASK_PRIORITY,
                &timer_task_handle
            );

            if (result != pdPASS) {
                ESP_LOGE(TAG, "Failed to create timer task (out of memory?)");
                delete params;
                app.SetDeviceState(kDeviceStateIdle);
                running = false;
            }
        });
    }

    ~Timer() {
        if (timer_task_handle != nullptr) {
            vTaskDelete(timer_task_handle);
            timer_task_handle = nullptr;
            running = false;
        }
    }
};

} // namespace iot

DECLARE_THING(Timer);