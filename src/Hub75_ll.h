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
    void selectLine(uint16_t line);
    void latch();
    void clock();
    void send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1);
    void send2Pixels(uint16_t colors);
    void renderBuffer();
};

void Hub75::initialize() {
  #ifndef USING_STM32F103C6
  AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
  #endif

  LL_GPIO_InitTypeDef GPIOInit = {0};
  GPIOInit.Pull  = LL_GPIO_PULL_DOWN;
  GPIOInit.Mode  = LL_GPIO_MODE_ALTERNATE;
  GPIOInit.Speed = LL_GPIO_SPEED_FREQ_HIGH;

  // Set pin PA8, PA9, PA10, PA11, PA12, PA15
  GPIOInit.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;
  LL_GPIO_Init(GPIOA, &GPIOInit);

  // Set pin PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB15
  GPIOInit.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_15;
  LL_GPIO_Init(GPIOB, &GPIOInit);

  width = CONFIG_DISPLAY_WIDTH;
  height = CONFIG_DISPLAY_HEIGHT;
  dimension = width * height;
}

void Hub75::selectLine(uint16_t line) {
  line = ((line - 1) & 0x1F) << 8;
  LL_GPIO_SetOutputPin(GPIOA,  line & 0x1F00);
  LL_GPIO_ResetOutputPin(GPIOA, ~line & 0x1F00);
}

void Hub75::latch() {
  // latch
  LL_GPIO_SetOutputPin(GPIOA, HUB75_PIN_LAT);
  LL_GPIO_ResetOutputPin(GPIOA, HUB75_PIN_LAT);

  // output enable
  LL_GPIO_SetOutputPin(GPIOB, HUB75_PIN_OE);
  LL_GPIO_ResetOutputPin(GPIOB, HUB75_PIN_OE);
}

void Hub75::clock() {
  // output enable
  LL_GPIO_SetOutputPin(GPIOB, HUB75_PIN_CLK);
  LL_GPIO_ResetOutputPin(GPIOB, HUB75_PIN_CLK);
}

void Hub75::send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1) {
  send2Pixels(
    (r0 ? 0x01 : 0) |
    (g0 ? 0x02 : 0) |
    (b0 ? 0x04 : 0) |
    (r1 ? 0x08 : 0) |
    (g1 ? 0x10 : 0) |
    (b1 ? 0x20 : 0)
  );
}

void Hub75::send2Pixels(uint16_t colors) {
  colors = colors << 3;
  LL_GPIO_SetOutputPin(GPIOB,  colors & 0x01F8);
  LL_GPIO_ResetOutputPin(GPIOB, ~colors & 0x01F8);
  clock();
}

void Hub75::renderBuffer() {
  uint32_t bitOffset = 0;
  for (uint16_t row = 0; row < (height / 2); row++) {
    selectLine(row);
    for (int32_t col = 0; col < width; col++) {
      if (bitOffset >= dimension) bitOffset = 0;
      bool bit0 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      bool bit1 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      send2Pixels(bit0, 0, 0, bit1, 0, 0);
    }
    latch();
  }
}

#endif
