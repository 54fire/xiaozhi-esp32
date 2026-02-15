#include "application.h"
#include "assets/lang_config.h"
#include "button.h"
#include "codecs/box_audio_codec.h"
#include "config.h"
#include "display/lcd_display.h"
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
#include "system_reset.h"
// #include "cloud_ai_service/CloudAIService.h"

#define TAG "CqylS3Board"
// 自定义AudioCodec类，忽略禁用输出的请求
class CustomAudioCodec : public BoxAudioCodec {
public:
    CustomAudioCodec(i2c_master_bus_handle_t i2c_bus)
        : BoxAudioCodec(i2c_bus, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                        AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
                        AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN, GPIO_NUM_NC,
                        AUDIO_CODEC_ES8311_ADDR, AUDIO_CODEC_ES7210_ADDR, AUDIO_INPUT_REFERENCE) {}

    virtual void EnableOutput(bool enable) override { BoxAudioCodec::EnableOutput(enable); }
};

// 配置cqyl-s3板为双网络板
class CqylS3Board : public DualNetworkBoard {
private:
    // I2C总线句柄--音频编解码器I2C总线
    i2c_master_bus_handle_t codec_i2c_bus_;
    // LED灯条
    CircularStrip* led_strip_;

    /// 按键实现部分
    // 按键--启动按钮
    Button boot_button_;
    // 传感器部分
    Button hug1_button_;
    Button hug2_button_;
    Button head1_button_;
    Button head2_button_;
    Button l_hand_button_;
    Button r_hand_button_;

    // Button people_button_;
    // Button touch_button_;
    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {.enable_internal_pullup = 1, .allow_pd = 0}};
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    // 按钮初始化
    void InitializeButtons() {
        // 按键单机事件
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (GetNetworkType() == NetworkType::WIFI) {
                if (app.GetDeviceState() == kDeviceStateStarting) {
                    // cast to WifiBoard
                    auto& wifi_board = static_cast<WifiBoard&>(GetCurrentBoard());
                    wifi_board.EnterWifiConfigMode();
                    return;
                }
            }
            app.ToggleChatState();
        });
        boot_button_.OnDoubleClick([this]() {
            ESP_LOGI(TAG, "Button OnDoubleClick");
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting ||
                app.GetDeviceState() == kDeviceStateWifiConfiguring) {
                SwitchNetworkType();
            }
        });
        // 长按重置设备配置信息
        boot_button_.OnLongPress([]() { ESP_LOGI(TAG, "Button is pressed, reset NVS flash"); });

        // hug1_button_.OnClick([this]()
        //                      {
        //                         ESP_LOGI(TAG, "hug1_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你摸了我的眼睛，谢谢你");

        // }); });
        // hug2_button_.OnClick([this]()
        //                      {
        //                         ESP_LOGI(TAG, "hug2_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你摸了我的头，谢谢你");

        // }); });
        // head1_button_.OnClick([this]()
        //                       {
        //                         ESP_LOGI(TAG, "head1_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你抱了我，谢谢你");

        // }); });
        // head2_button_.OnClick([this]()
        //                       {
        //                         ESP_LOGI(TAG, "head2_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你摸了我的眼睛，谢谢你");

        // }); });
        // l_hand_button_.OnClick([this]()
        //                        {
        //                         ESP_LOGI(TAG, "l_hand_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你摸了我的左手，谢谢你");

        // }); });
        // r_hand_button_.OnClick([this]()
        //                        {
        //                         ESP_LOGI(TAG, "r_hand_button_ is pressed");
        // auto& app = Application::GetInstance();

        // // 确保音频输出已启用
        // auto codec = GetAudioCodec();
        // if (codec && !codec->output_enabled()) {
        //     codec->EnableOutput(true);
        // }

        // // 通过MCP协议发送文本消息，让服务器处理TTS
        // app.Schedule([this]() {
        //     auto& app = Application::GetInstance();

        //     // 确保WebSocket连接已建立
        //     auto device_state = app.GetDeviceState();
        //     if (device_state == kDeviceStateIdle) {
        //         ESP_LOGI(TAG, "Opening audio channel before sending MCP
        //         message");
        //         // 使用ToggleChatState来建立WebSocket连接
        //         app.ToggleChatState();
        //         // 等待连接建立
        //         vTaskDelay(pdMS_TO_TICKS(1000));
        //     }

        //     //
        //     发送listen消息，触发服务器的chat功能，使用"请读"指令让服务器直接读这些文字
        //     app.SendListenMessage("请读：你摸了我的右手，谢谢你");

        // }); });
    }

    // MCP Tools 初始化 可以使用ai进行远程控制的部分在这里注册
    void InitializeTools() {
        // led_strip_ = new CircularStrip(BUILTIN_LED_GPIO, 60);
        new LedStripControl(led_strip_);
    }

public:
    // 构造函数
    CqylS3Board()
        : DualNetworkBoard(ML307_TX_PIN, ML307_RX_PIN, ML307_DTR_PIN),
          boot_button_(BUTTON_GPIO_BOOT),
          hug1_button_(GPIO_NUM_NC),
          hug2_button_(GPIO_NUM_NC),
          head1_button_(GPIO_NUM_NC),
          head2_button_(GPIO_NUM_NC),
          l_hand_button_(GPIO_NUM_NC),
          r_hand_button_(GPIO_NUM_NC) {
        InitializeI2c();
        // InitializeSpi();
        // InitializeDisplay();
        InitializeButtons();
        auto codec = GetAudioCodec();
        if (codec) {
            codec->SetOutputVolume(100);
        }
        // InitializeTools();
        // 设置背光亮度
        // GetBacklight()->SetBrightness(10);

        // touch_sensor_handle_t sens_handle = NULL;
        // // 采样配置
        // touch_sensor_sample_config_t sample_cfg[SAMPLE_NUM] = {
        //     // 指定采样配置或通过 `TOUCH_SENSOR_Vn_DEFAULT_SAMPLE_CONFIG` 使用默认采样配置
        //     // ...
        // };
        // // 默认控制器配置
        // touch_sensor_config_t touch_cfg = TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(SAMPLE_NUM,
        // sample_cfg);
        // // 申请一个新的触摸传感器控制器句柄
        // ESP_ERROR_CHECK(touch_sensor_new_controller(&touch_cfg, &sens_handle));
    }

    virtual AudioCodec* GetAudioCodec() override {
        static CustomAudioCodec audio_codec(codec_i2c_bus_);
        return &audio_codec;
    }

    // virtual Led *GetLed() override
    // {
    //     return led_strip_;
    // }

    // 获取背光控制
    // virtual Backlight *GetBacklight() override
    // {
    //     // 启动背光调光
    //     static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
    //     return &backlight;
    // }
};

// 注册开发板
DECLARE_BOARD(CqylS3Board);