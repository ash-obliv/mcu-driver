#ifndef __BSP_TFT_H
#define __BSP_TFT_H

#include "st7735s.h"

//========== 硬件引脚定义 ==========
// // GPIOB
// #define TFT_BLK_PORT GPIOB
// #define TFT_BLK_PIN  GPIO_Pin_0
// #define TFT_DC_PORT  GPIOB
// #define TFT_DC_PIN   GPIO_Pin_8
// #define TFT_RST_PORT GPIOB
// #define TFT_RST_PIN  GPIO_Pin_7

// // GPIOA
// #define TFT_SCL_PORT GPIOA
// #define TFT_SCL_PIN  GPIO_Pin_5
// #define TFT_SDA_PORT GPIOA
// #define TFT_SDA_PIN  GPIO_Pin_6
// #define TFT_CS_PORT  GPIOA
// #define TFT_CS_PIN   GPIO_Pin_4

//========== 公共接口 ==========
TFT_Status_t BSP_TFT_Init(void);

#endif
