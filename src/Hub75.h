#ifndef __HUB_75_H__
#define __HUB_75_H__

#include <Arduino.h>
#include "Memory.h"
#include <stm32f1xx_hal.h>
#include "Renderer.h"

#define HUB75_PIN_R1  GPIO_PIN_3
#define HUB75_PIN_G1  GPIO_PIN_4
#define HUB75_PIN_B1  GPIO_PIN_5
#define HUB75_PIN_R2  GPIO_PIN_6
#define HUB75_PIN_G2  GPIO_PIN_7
#define HUB75_PIN_B2  GPIO_PIN_8

#define HUB75_PIN_A   GPIO_PIN_8
#define HUB75_PIN_B   GPIO_PIN_9
#define HUB75_PIN_C   GPIO_PIN_10
#define HUB75_PIN_D   GPIO_PIN_11
#define HUB75_PIN_E   GPIO_PIN_12

#define HUB75_PIN_CLK GPIO_PIN_15
#define HUB75_PIN_LAT GPIO_PIN_15
#define HUB75_PIN_OE  GPIO_PIN_9

class Hub75 {
  private:
    uint16_t width;
    uint16_t height;
    uint32_t dimension;
  public:
    bool samplingMode = false;
    void initialize();
    void selectLine(uint16_t &line);
    void send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1, int32_t &col);
    void send2Pixels(uint16_t colors, int32_t &col);
    void renderBuffer();
};

void Hub75::initialize() {
  #ifndef USING_STM32F103C6
  AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
  #endif
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIOInit = {0};
  GPIOInit.Pull  = GPIO_PULLDOWN;
  GPIOInit.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIOInit.Speed = GPIO_SPEED_FREQ_HIGH;

  // Set pin PA8, PA9, PA10, PA11, PA12, PA15
  GPIOInit.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIOInit);

  // Set pin PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB15
  GPIOInit.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &GPIOInit);

  width = CONFIG_DISPLAY_WIDTH;
  height = CONFIG_DISPLAY_HEIGHT;
  dimension = width * height;
}

void Hub75::selectLine(uint16_t &line) {

  HAL_GPIO_WritePin(GPIOB, HUB75_PIN_OE, GPIO_PIN_SET);
  int rowPin = (line & 0x1F) << 8;
  // Addressing
  HAL_GPIO_WritePin(GPIOA,  rowPin & 0x1F00, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, ~rowPin & 0x1F00, GPIO_PIN_RESET);

  // Latching
  HAL_GPIO_WritePin(GPIOA, HUB75_PIN_LAT, GPIO_PIN_SET);
  line++;
  HAL_GPIO_WritePin(GPIOA, HUB75_PIN_LAT, GPIO_PIN_RESET);

  // Output Enable
  HAL_GPIO_WritePin(GPIOB, HUB75_PIN_OE, GPIO_PIN_RESET);
}

void Hub75::send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1, int32_t &col) {
  send2Pixels(
    (r0 ? 0x01 : 0) |
    (g0 ? 0x02 : 0) |
    (b0 ? 0x04 : 0) |
    (r1 ? 0x08 : 0) |
    (g1 ? 0x10 : 0) |
    (b1 ? 0x20 : 0) ,
    col
  );
}

void Hub75::send2Pixels(uint16_t colors, int32_t &col) {
  colors = colors << 3;

  // Colors
  HAL_GPIO_WritePin(GPIOB,  colors & 0x01F8, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, ~colors & 0x01F8, GPIO_PIN_RESET);

  // Clock
  HAL_GPIO_WritePin(GPIOB, HUB75_PIN_CLK, GPIO_PIN_SET);
  col++;
  HAL_GPIO_WritePin(GPIOB, HUB75_PIN_CLK, GPIO_PIN_RESET);
}

void Hub75::renderBuffer() {
  uint32_t bitOffset = 0;
  for (uint16_t row = 0; row < (height / 2);) {
    for (int32_t col = 0; col < width;) {
      if (bitOffset >= dimension) bitOffset = 0;
      bool bit0 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      bool bit1 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      send2Pixels(bit0, 0, 0, bit1, 0, 0, col);
    }
    selectLine(row);
  }
}

#endif
