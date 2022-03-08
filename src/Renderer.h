#ifndef __RENDERER_H__
#define __RENDERER_H__
#include "Memory.h"

class Renderer
{
  public:
    int32_t displayWidth;
    int32_t displayHeight;
    int32_t bufferSize;

    void initialize();
    void clearBuffer();
    void drawPixel(int32_t x, int32_t y);
};

void Renderer::initialize() {
  displayWidth = CONFIG_DISPLAY_WIDTH;
  displayHeight = CONFIG_DISPLAY_HEIGHT;
  int32_t dimension = displayWidth * displayHeight;
  bufferSize = dimension / 8;
  if(dimension % 8 != 0) bufferSize++;
  clearBuffer();
}

void Renderer::clearBuffer() {
  for (int32_t i = 0; i < bufferSize; i++) {
    GLOBAL_BUFFER[i] = 0;
  }
}

void Renderer::drawPixel(int32_t x, int32_t y) {
  if (x < 0 || y < 0) return;

  bool useLower = false;
  if (y >= displayHeight / 2) {
    y -= displayHeight / 2;
    useLower = true;
  }

  int32_t offset = y * displayWidth + x;
  int32_t byteOffset = offset / 4;
  int32_t bitOffset = 2 * (offset % 4);

  if (useLower) {
    GLOBAL_BUFFER[byteOffset] |= (1 << (bitOffset + 1)) & 0xFF;
  }
  else {
    GLOBAL_BUFFER[byteOffset] |= (1 << bitOffset) & 0xFF;
  }
}

#endif