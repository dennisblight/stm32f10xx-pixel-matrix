#ifndef __HUB_75_H__
#define __HUB_75_H__

#include <Arduino.h>
#include "Memory.h"
#include "Renderer.h"

#define HUB75_PIN_R1  PB3
#define HUB75_PIN_G1  PB4
#define HUB75_PIN_B1  PB5
#define HUB75_PIN_R2  PB6
#define HUB75_PIN_G2  PB7
#define HUB75_PIN_B2  PB8
#define HUB75_PIN_A   PA8
#define HUB75_PIN_B   PA9
#define HUB75_PIN_C   PA10
#define HUB75_PIN_D   PA11
#define HUB75_PIN_E   PA12
#define HUB75_PIN_CLK PB15
#define HUB75_PIN_LAT PA15
#define HUB75_PIN_OE  PB9

class Hub75 {
  private:
    uint16_t width;
    uint16_t height;
    uint32_t dimension;
  public:
    bool samplingMode = false;
    void initialize();
    void selectLine(int line);
    void latch();
    void clock();
    void send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1);
    void send2Pixels(int colors);
    void renderBuffer();
};

void Hub75::initialize() {
  pinMode(HUB75_PIN_R1, OUTPUT);
  pinMode(HUB75_PIN_G1, OUTPUT);
  pinMode(HUB75_PIN_B1, OUTPUT);
  pinMode(HUB75_PIN_R2, OUTPUT);
  pinMode(HUB75_PIN_G2, OUTPUT);
  pinMode(HUB75_PIN_B2, OUTPUT);
  pinMode(HUB75_PIN_A, OUTPUT);
  pinMode(HUB75_PIN_B, OUTPUT);
  pinMode(HUB75_PIN_C, OUTPUT);
  pinMode(HUB75_PIN_D, OUTPUT);
  pinMode(HUB75_PIN_E, OUTPUT);
  pinMode(HUB75_PIN_CLK, OUTPUT);
  pinMode(HUB75_PIN_LAT, OUTPUT);
  pinMode(HUB75_PIN_OE, OUTPUT);

  width = CONFIG_DISPLAY_WIDTH;
  height = CONFIG_DISPLAY_HEIGHT;
  dimension = width * height;
}

void Hub75::selectLine(int line) {
  line = (line - 1) & 0x1F;
  digitalWrite(HUB75_PIN_A, (line & 0x01) == 0x01 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_B, (line & 0x02) == 0x02 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_C, (line & 0x04) == 0x04 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_D, (line & 0x08) == 0x08 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_E, (line & 0x10) == 0x10 ? HIGH : LOW);
}

void Hub75::latch() {
  // latch
  digitalWrite(HUB75_PIN_LAT, HIGH);
  digitalWrite(HUB75_PIN_LAT, LOW);

  // selectLine(1);

  // output enable
  digitalWrite(HUB75_PIN_OE, HIGH);
  digitalWrite(HUB75_PIN_OE, LOW);
}

void Hub75::clock() {
  digitalWrite(HUB75_PIN_CLK, HIGH);
  digitalWrite(HUB75_PIN_CLK, LOW);
}

void Hub75::send2Pixels(bool r0, bool g0, bool b0, bool r1, bool g1, bool b1) {
  int colors =
    (r0 ? 0x01 : 0) |
    (g0 ? 0x02 : 0) |
    (b0 ? 0x04 : 0) |
    (r1 ? 0x08 : 0) |
    (g1 ? 0x10 : 0) |
    (b1 ? 0x20 : 0) ;
  send2Pixels(colors);
}

void Hub75::send2Pixels(int colors) {
  digitalWrite(HUB75_PIN_R1, (colors & 0x01) == 0x01 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_G1, (colors & 0x02) == 0x02 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_B1, (colors & 0x04) == 0x04 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_R2, (colors & 0x08) == 0x08 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_G2, (colors & 0x10) == 0x10 ? HIGH : LOW);
  digitalWrite(HUB75_PIN_B2, (colors & 0x20) == 0x20 ? HIGH : LOW);
  clock();
}

void Hub75::renderBuffer() {
  unsigned int bitOffset = 0;
  for (int row = 0; row < (height / 2); row++) {
    selectLine(row);
    for (int col = 0; col < width; col++) {
      if (bitOffset >= dimension) bitOffset = 0;
      bool bit0 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      bool bit1 = (GLOBAL_BUFFER[bitOffset / 8] >> (bitOffset++ % 8)) & 0x01;
      send2Pixels(bit0, 0, 0, bit1, 0, 0);
    }
    latch();
  }
}

#endif
