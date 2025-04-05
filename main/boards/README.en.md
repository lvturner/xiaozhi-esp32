# Custom Development Board Guide  

This guide explains how to customize a new development board initialization program for the XiaoZhi AI Voice Chatbot project. XiaoZhi AI supports over 50 ESP32 series development boards, with each board's initialization code stored in its corresponding directory.  

## Important Notes  

> **Warning**: For custom development boards, if the IO configuration differs from existing boards, **do not** directly overwrite the original board's configuration when compiling the firmware. You must either:  
> - Create a new board type, or  
> - Use the `builds` configuration in `config.json` to differentiate by specifying unique `name` and `sdkconfig` macro definitions.  
>  
> Use `python scripts/release.py [board_directory_name]` to compile and package the firmware.  
>  
> If you overwrite the original configuration, future OTA updates may replace your custom firmware with the standard firmware of the original board, rendering your device inoperable. Each board has a unique identifier and corresponding firmware update channel, so maintaining uniqueness is critical.  

## Directory Structure  

Each development board's directory typically contains the following files:  

- `xxx_board.cc` – Main board-level initialization code, implementing board-specific setup and functionality.  
- `config.h` – Board configuration file, defining hardware pin mappings and other settings.  
- `config.json` – Build configuration, specifying the target chip and special compilation options.  
- `README.md` – Documentation for the development board.  

## Steps to Customize a Board  

### 1. Create a New Board Directory  

First, create a new directory under `boards/`, e.g., `my-custom-board/`:  

```bash  
mkdir main/boards/my-custom-board  
```  

### 2. Create Configuration Files  

#### config.h  

Define all hardware configurations in `config.h`, including:  

- Audio sampling rate and I2S pin configuration  
- Audio codec chip address and I2C pin configuration  
- Button and LED pin configuration  
- Display parameters and pin configuration  

Example (from `lichuang-c3-dev`):  

```c  
#ifndef _BOARD_CONFIG_H_  
#define _BOARD_CONFIG_H_  

#include <driver/gpio.h>  

// Audio configuration  
#define AUDIO_INPUT_SAMPLE_RATE  24000  
#define AUDIO_OUTPUT_SAMPLE_RATE 24000  

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_10  
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_12  
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_8  
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_7  
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_11  

#define AUDIO_CODEC_PA_PIN       GPIO_NUM_13  
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_0  
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_1  
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR  

// Button configuration  
#define BOOT_BUTTON_GPIO        GPIO_NUM_9  

// Display configuration  
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_3  
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_5  
#define DISPLAY_DC_PIN          GPIO_NUM_6  
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_4  

#define DISPLAY_WIDTH   320  
#define DISPLAY_HEIGHT  240  
#define DISPLAY_MIRROR_X true  
#define DISPLAY_MIRROR_Y false  
#define DISPLAY_SWAP_XY true  

#define DISPLAY_OFFSET_X  0  
#define DISPLAY_OFFSET_Y  0  

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_2  
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true  

#endif // _BOARD_CONFIG_H_  
```  

#### config.json  

Define build settings in `config.json`:  

```json  
{  
    "target": "esp32s3",  // Target chip: esp32, esp32s3, esp32c3, etc.  
    "builds": [  
        {  
            "name": "my-custom-board",  // Board name  
            "sdkconfig_append": [  
                // Additional build configurations  
                "CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y",  
                "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions_8M.csv\""  
            ]  
        }  
    ]  
}  
```  

### 3. Write Board Initialization Code  

Create a `my_custom_board.cc` file to implement all initialization logic for the board.  

A basic board class definition includes:  

1. **Class definition**: Inherits from `WifiBoard` or `ML307Board`.  
2. **Initialization functions**: For I2C, display, buttons, IoT components, etc.  
3. **Virtual function overrides**: Such as `GetAudioCodec()`, `GetDisplay()`, `GetBacklight()`, etc.  
4. **Board registration**: Use the `DECLARE_BOARD` macro to register the board.  

```cpp  
#include "wifi_board.h"  
#include "audio_codecs/es8311_audio_codec.h"  
#include "display/lcd_display.h"  
#include "application.h"  
#include "button.h"  
#include "config.h"  
#include "iot/thing_manager.h"  

#include <esp_log.h>  
#include <driver/i2c_master.h>  
#include <driver/spi_common.h>  

#define TAG "MyCustomBoard"  

// Declare fonts  
LV_FONT_DECLARE(font_puhui_16_4);  
LV_FONT_DECLARE(font_awesome_16_4);  

class MyCustomBoard : public WifiBoard {  
private:  
    i2c_master_bus_handle_t codec_i2c_bus_;  
    Button boot_button_;  
    LcdDisplay* display_;  

    // I2C initialization  
    void InitializeI2c() {  
        i2c_master_bus_config_t i2c_bus_cfg = {  
            .i2c_port = I2C_NUM_0,  
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,  
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,  
            .clk_source = I2C_CLK_SRC_DEFAULT,  
            .glitch_ignore_cnt = 7,  
            .intr_priority = 0,  
            .trans_queue_depth = 0,  
            .flags = {  
                .enable_internal_pullup = 1,  
            },  
        };  
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));  
    }  

    // SPI initialization (for display)  
    void InitializeSpi() {  
        spi_bus_config_t buscfg = {};  
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;  
        buscfg.miso_io_num = GPIO_NUM_NC;  
        buscfg.sclk_io_num = DISPLAY_SPI_SCK_PIN;  
        buscfg.quadwp_io_num = GPIO_NUM_NC;  
        buscfg.quadhd_io_num = GPIO_NUM_NC;  
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);  
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));  
    }  

    // Button initialization  
    void InitializeButtons() {  
        boot_button_.OnClick([this]() {  
            auto& app = Application::GetInstance();  
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {  
                ResetWifiConfiguration();  
            }  
            app.ToggleChatState();  
        });  
    }  

    // Display initialization (example for ST7789)  
    void InitializeDisplay() {  
        esp_lcd_panel_io_handle_t panel_io = nullptr;  
        esp_lcd_panel_handle_t panel = nullptr;  
          
        esp_lcd_panel_io_spi_config_t io_config = {};  
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;  
        io_config.dc_gpio_num = DISPLAY_DC_PIN;  
        io_config.spi_mode = 2;  
        io_config.pclk_hz = 80 * 1000 * 1000;  
        io_config.trans_queue_depth = 10;  
        io_config.lcd_cmd_bits = 8;  
        io_config.lcd_param_bits = 8;  
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));  

        esp_lcd_panel_dev_config_t panel_config = {};  
        panel_config.reset_gpio_num = GPIO_NUM_NC;  
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;  
        panel_config.bits_per_pixel = 16;  
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));  
          
        esp_lcd_panel_reset(panel);  
        esp_lcd_panel_init(panel);  
        esp_lcd_panel_invert_color(panel, true);  
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);  
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);  
          
        // Create display object  
        display_ = new SpiLcdDisplay(panel_io, panel,  
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT,   
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,   
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,  
                                    {  
                                        .text_font = &font_puhui_16_4,  
                                        .icon_font = &font_awesome_16_4,  
                                        .emoji_font = font_emoji_32_init(),  
                                    });  
    }  

    // IoT device initialization  
    void InitializeIot() {  
        auto& thing_manager = iot::ThingManager::GetInstance();  
        thing_manager.AddThing(iot::CreateThing("Speaker"));  
        thing_manager.AddThing(iot::CreateThing("Screen"));  
        // Add more IoT devices as needed  
    }  

public:  
    // Constructor  
    MyCustomBoard() : boot_button_(BOOT_BUTTON_GPIO) {  
        InitializeI2c();  
        InitializeSpi();  
        InitializeDisplay();  
        InitializeButtons();  
        InitializeIot();  
        GetBacklight()->SetBrightness(100);  
    }  

    // Get audio codec  
    virtual AudioCodec* GetAudioCodec() override {  
        static Es8311AudioCodec audio_codec(  
            codec_i2c_bus_,   
            I2C_NUM_0,   
            AUDIO_INPUT_SAMPLE_RATE,   
            AUDIO_OUTPUT_SAMPLE_RATE,  
            AUDIO_I2S_GPIO_MCLK,   
            AUDIO_I2S_GPIO_BCLK,   
            AUDIO_I2S_GPIO_WS,   
            AUDIO_I2S_GPIO_DOUT,   
            AUDIO_I2S_GPIO_DIN,  
            AUDIO_CODEC_PA_PIN,   
            AUDIO_CODEC_ES8311_ADDR);  
        return &audio_codec;  
    }  

    // Get display  
    virtual Display* GetDisplay() override {  
        return display_;  
    }  
      
    // Get backlight control  
    virtual Backlight* GetBacklight() override {  
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);  
        return &backlight;  
    }  
};  

// Register the board  
DECLARE_BOARD(MyCustomBoard);  
```  

### 4. Create README.md  

Document the board's features, hardware requirements, and flashing instructions in `README.md`.  

## Common Board Components  

### 1. Displays  

Supported display drivers include:  
- ST7789 (SPI)  
- ILI9341 (SPI)  
- SH8601 (QSPI)  
- Others...  

### 2. Audio Codecs  

Supported codecs:  
- ES8311 (common)  
- ES7210 (microphone array)  
- AW88298 (amplifier)  
- Others...  

### 3. Power Management  

Some boards use power management ICs:  
- AXP2101  
- Other available PMICs  

### 4. IoT Devices  

Add various IoT devices for AI interaction:  
- Speaker  
- Screen  
- Battery  
- Light  
- Others...  

## Board Class Inheritance  

- `Board` – Base board class  
  - `WifiBoard` – WiFi-enabled boards  
  - `ML307Board` – 4G module-enabled boards  

## Development Tips  

1. **Reference similar boards**: If your new board resembles an existing one, study its implementation.  
2. **Debug step-by-step**: Start with basic features (e.g., display) before adding complex functionality (e.g., audio).  
3. **Pin mapping**: Ensure all pin mappings in `config.h` are correct.  
4. **Hardware compatibility**: Verify chip and driver compatibility.  

## Common Issues  

1. **Display issues**: Check SPI configuration, mirror settings, and color inversion.  
2. **No audio output**: Verify I2S settings, PA enable pin, and codec address.  
3. **Network failure**: Check WiFi credentials and network configuration.  
4. **Server communication failure**: Inspect MQTT or WebSocket settings.  

## References  

- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/  
- LVGL Documentation: https://docs.lvgl.io/  
- ESP-SR Documentation: https://github.com/espressif/esp-sr