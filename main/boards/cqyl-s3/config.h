#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>


// 音频配置
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// 如果使用 Duplex I2S 模式，请注释下面一行
#define AUDIO_INPUT_REFERENCE       true
#define AUDIO_I2S_GPIO_MCLK         GPIO_NUM_38
#define AUDIO_I2S_GPIO_WS           GPIO_NUM_13
#define AUDIO_I2S_GPIO_BCLK         GPIO_NUM_14
#define AUDIO_I2S_GPIO_DIN          GPIO_NUM_12
#define AUDIO_I2S_GPIO_DOUT         GPIO_NUM_45

#define AUDIO_CODEC_I2C_SDA_PIN     GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN     GPIO_NUM_2
#define AUDIO_CODEC_ES8311_ADDR     ES8311_CODEC_DEFAULT_ADDR
#define AUDIO_CODEC_ES7210_ADDR     0x82




// 按钮配置
#define BUTTON_GPIO_BOOT        GPIO_NUM_0
// #define BUTTON_GPIO_HUG1        GPIO_NUM_10
// #define BUTTON_GPIO_HUG2        GPIO_NUM_9
// #define BUTTON_GPIO_HEAD1        GPIO_NUM_8
// #define BUTTON_GPIO_HEAD2        GPIO_NUM_3
// #define BUTTON_GPIO_L_HAND        GPIO_NUM_14
// #define BUTTON_GPIO_R_HAND        GPIO_NUM_13
// #define BUTTON_GPIO_PEPLE        GPIO_NUM_11
// #define BUTTON_GPIO_TOUCH        GPIO_NUM_46


//4G模块配置
#define ML307_RX_PIN GPIO_NUM_39
#define ML307_TX_PIN GPIO_NUM_40
#define ML307_DTR_PIN GPIO_NUM_41


//灯光控制引脚 板子未使用的引脚
// #define BUILTIN_LED_GPIO       GPIO_NUM_2
// #define DISPLAY_BACKLIGHT_PIN GPIO_NUM_1
// #define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

//灯带控制引脚
// #define BUILTIN_LED_GPIO        GPIO_NUM_1

// lamp控制引脚 板子未使用的引脚
// #define LAMP_GPIO GPIO_NUM_17

#endif // _BOARD_CONFIG_H_