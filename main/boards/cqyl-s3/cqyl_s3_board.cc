#include "application.h"
#include "assets/lang_config.h"
#include "button.h"
#include "codecs/box_audio_codec.h"
#include "config.h"
#include "display/lcd_display.h"
#include "led/single_led.h"
#include "dual_network_board.h"
#include "lamp_controller.h"
#include "led/circular_strip.h"
#include "led_control.h"
#include "mcp_server.h"
#include "system_reset.h"

#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_log.h>
#include <wifi_station.h>

#define TAG "CqylS3Board"

class CustomAudioCodec : public BoxAudioCodec {
public:
    CustomAudioCodec(i2c_master_bus_handle_t i2c_bus)
        : BoxAudioCodec(i2c_bus, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                        AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
                        AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN, AUDIO_PA_PIN,
                        AUDIO_CODEC_ES8311_ADDR, AUDIO_CODEC_ES7210_ADDR, AUDIO_INPUT_REFERENCE) {}

    void EnableOutput(bool enable) override { BoxAudioCodec::EnableOutput(enable); }
};

class CqylS3Board : public DualNetworkBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    CircularStrip* led_strip_;

    Button boot_button_;
    Button hug1_button_;
    Button hug2_button_;
    Button head1_button_;
    Button head2_button_;
    Button l_hand_button_;
    Button r_hand_button_;

    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {.enable_internal_pullup = 1, .allow_pd = 0},
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    void EnsureAudioOutputEnabled() {
        auto codec = GetAudioCodec();
        if (codec && !codec->output_enabled()) {
            codec->EnableOutput(true);
        }
    }

    static void EnsureAudioChannelReady(Application& app) {
        if (app.GetDeviceState() == kDeviceStateIdle) {
            ESP_LOGI(TAG, "Opening audio channel before sending MCP message");
            app.ToggleChatState();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    void ScheduleTouchMessage(const char* text) {
        auto& app = Application::GetInstance();
        EnsureAudioOutputEnabled();
        app.Schedule([text = std::string(text)]() {
            auto& app = Application::GetInstance();
            EnsureAudioChannelReady(app);
            app.WakeWordInvoke(text.c_str());
            return;
        });
    }

    void RegisterTouchButton(Button& button, const char* name, const char* text) {
        button.OnClick([this, name, text]() {
            ESP_LOGI(TAG, "%s is pressed", name);
            ScheduleTouchMessage(text);
        });
        button.OnLongPress([this, name, text]() {
            ESP_LOGI(TAG, "%s is long pressed", name);
            ScheduleTouchMessage(text);
        });
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (GetNetworkType() == NetworkType::WIFI &&
                app.GetDeviceState() == kDeviceStateStarting) {
                auto& wifi_board = static_cast<WifiBoard&>(GetCurrentBoard());
                wifi_board.EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });

        boot_button_.OnDoubleClick([this]() {
            ESP_LOGI(TAG, "Button OnDoubleClick");
            // auto& app = Application::GetInstance();
            // if (app.GetDeviceState() == kDeviceStateStarting ||
                // app.GetDeviceState() == kDeviceStateWifiConfiguring) {
                SwitchNetworkType();
            // }
        });

        boot_button_.OnLongPress([]() { ESP_LOGI(TAG, "Button is pressed, reset NVS flash"); });

        RegisterTouchButton(hug1_button_, "hug1_button", "请读：你摸了我的左睛，谢谢你");
        RegisterTouchButton(hug2_button_, "hug2_button", "请读：你摸了我的右眼，谢谢你");
        RegisterTouchButton(head1_button_, "head1_button", "请读：你抱了我，谢谢你");
        RegisterTouchButton(head2_button_, "head2_button", "请读：你摸了我的左手，谢谢你");
        // RegisterTouchButton(l_hand_button_, "l_hand_button", "请读：你摸了我的左手，谢谢你");
        RegisterTouchButton(r_hand_button_, "r_hand_button", "请读：你摸了我的右手，谢谢你");
    }

    void InitializeTools() {
        led_strip_ = new CircularStrip(BUILTIN_LED_GPIO, 147);
        new LedStripControl(led_strip_);
    }

public:
    CqylS3Board()
        : DualNetworkBoard(ML307_TX_PIN, ML307_RX_PIN, ML307_DTR_PIN),
          led_strip_(nullptr),
          boot_button_(BUTTON_GPIO_BOOT),
          hug1_button_(BUTTON_GPIO_HUG1),
          hug2_button_(BUTTON_GPIO_HUG2),
          head1_button_(BUTTON_GPIO_HEAD1),
          head2_button_(BUTTON_GPIO_HEAD2),
          l_hand_button_(BUTTON_GPIO_L_HAND),
          r_hand_button_(BUTTON_GPIO_R_HAND) {
        InitializeI2c();
        // InitializeSpi();
        // InitializeDisplay();
        InitializeButtons();
        InitializeTools();

        // 设置背光亮度
        // GetBacklight()->SetBrightness(10);

        // touch_sensor_handle_t sens_handle = NULL;
        // touch_sensor_sample_config_t sample_cfg[SAMPLE_NUM] = {
        // };
        // touch_sensor_config_t touch_cfg = TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(SAMPLE_NUM,
        // sample_cfg);
        // ESP_ERROR_CHECK(touch_sensor_new_controller(&touch_cfg, &sens_handle));
    }

    AudioCodec* GetAudioCodec() override {
        static CustomAudioCodec audio_codec(codec_i2c_bus_);
        return &audio_codec;
    }

    virtual Led* GetLed() override {
        return led_strip_;
    }

    // virtual Backlight* GetBacklight() override {
    //     static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN,
    //                                  DISPLAY_BACKLIGHT_OUTPUT_INVERT);
    //     return &backlight;
    // }
};

DECLARE_BOARD(CqylS3Board);
