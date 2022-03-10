#ifndef __STORAGE_H__
#define __STORAGE_H__

#define SDCARD_PIN_CS   PA4
#define CONFIG_FILENAME (char*)"CONFIG.BIN"
#define SIGNAL_PIN      PC13
#include <Arduino.h>
#include "Memory.h"
#include <SPI.h>
#include <SD.h>
#include "Renderer.h"

class Storage {
  private:
    bool errorSignal = false;
    uint32_t lastMillis = 0;
    uint32_t beginSaveMillis = 0;
    File file;

    bool savingIndex = false;
    uint8_t logIndex = 0;
    uint16_t sampleIndex = 0;
    uint16_t sampleSize;
    bool openFile(char* filename, uint8_t mode);
    void storeSample(uint8_t blockSize, uint16_t offset);
    void normalizeSampleMode();
  public:
    Renderer* renderer;
    void initialize();
    void loadBuffer(int32_t index, int32_t size);
    void loadBuffer(int32_t index);
    void loadBuffer();
    void beginSaveIndex(int8_t index);
    void saveIndex();
    void saveSample();
    void recordSample(uint16_t adc1, uint16_t adc2);
    void recordTimestamp();
    void blinkErrorSignal();
};

void Storage::initialize() {
  pinMode(SIGNAL_PIN, OUTPUT);
  delay(500);
  while(!SD.begin(SPI_QUARTER_SPEED, SDCARD_PIN_CS)) {
    delay(100);
    digitalToggle(SIGNAL_PIN);
  }
  if(openFile(CONFIG_FILENAME, FILE_READ)) {
    file.read(CONFIG, CONFIG_SIZE);
    file.close();
  }
  digitalWrite(SIGNAL_PIN, HIGH);
  normalizeSampleMode();
  sampleSize = CONFIG_SAMPLE_SIZE;
}

void Storage::normalizeSampleMode() {
  CONFIG_SAMPLE_MODE &= 0x0F;
  if(CONFIG_SAMPLE_MODE_TIMESTAMP) {
    CONFIG_SAMPLE_MODE &= 0x03;
  }
  else if(CONFIG_SAMPLE_MODE_NONE || !CONFIG_SAMPLE_MODE_HAS_CHANNEL) {
    CONFIG_SAMPLE_MODE = 0x00;
  }
  
  if(CONFIG_SAMPLE_SIZE > (GLOBAL_BUFFER_SIZE / 4)) {
    uint16_t maximumSampling = (GLOBAL_BUFFER_SIZE / 4);
    CONFIG[CONFIG_IDX_SAMPLE_SIZE + 0] = maximumSampling & 0xFF;
    CONFIG[CONFIG_IDX_SAMPLE_SIZE + 1] = (maximumSampling >> 8) & 0xFF;
  }
}

bool Storage::openFile(char* filename, uint8_t mode) {
  if(file) file.close();
  file = SD.open(filename, mode);
  return !!file;
}

void Storage::loadBuffer(int32_t index, int32_t size) {
  sprintf(FILENAME, "DISPLAY/%02d.BIN", index);
  if(!openFile(FILENAME, FILE_READ)) {
    errorSignal = true;
    return;
  }
  digitalWrite(SIGNAL_PIN, LOW);

  file.readBytes(GLOBAL_BUFFER, size);
  file.close();
  errorSignal = false;
  
  digitalWrite(SIGNAL_PIN, HIGH);
}

void Storage::loadBuffer(int32_t index) {
  renderer->clearBuffer();
  loadBuffer(index, renderer->bufferSize);
}

void Storage::loadBuffer() {
  loadBuffer(CONFIG_LAST_INDEX, renderer->bufferSize);
}

void Storage::beginSaveIndex(int8_t index) {
  CONFIG_LAST_INDEX = index;
  savingIndex = true;
  beginSaveMillis = millis();
}

void Storage::saveIndex() {
  if(savingIndex && (millis() - beginSaveMillis) > 500) {
    digitalWrite(SIGNAL_PIN, LOW);
    if(SD.exists(CONFIG_FILENAME)) {
      SD.remove(CONFIG_FILENAME);
    }
    if(openFile(CONFIG_FILENAME, FILE_WRITE)) {
      file.write(CONFIG, CONFIG_SIZE);
      file.flush();
      file.close();
    }
    digitalWrite(SIGNAL_PIN, HIGH);
    savingIndex = false;
  }
}

void Storage::storeSample(uint8_t blockSize, uint16_t offset) {
  if(openFile(FILENAME, FILE_WRITE)) {
    if(blockSize == 4) {
      for(uint16_t i = 0; i < sampleIndex; i++) {
        uint32_t record = BUFFER_READ_INT(offset + i * 4);
        file.println(String(record));
      }
    }
    else if(blockSize == 2) {
      for(uint16_t i = 0; i < sampleIndex; i++) {
        uint16_t record = BUFFER_READ_SHORT(offset + i * 2);
        file.println(String(record));
      }
    }
    file.println("--------------------");
    file.flush();
    file.close();
  }
}

void Storage::saveSample() {
  if(CONFIG_SAMPLE_MODE) {
    if(!SD.exists("SAMPLE")) {
      SD.mkdir("SAMPLE");
    }
    if(CONFIG_SAMPLE_MODE_TIMESTAMP) {
      sprintf(FILENAME, "SAMPLE/TSTMP.TXT");
      storeSample(4, 0);
    }
    else if(CONFIG_SAMPLE_MODE_HAS_CHANNEL) {
      char* pattern = CONFIG_SAMPLE_MODE_ACCUMULATION ? PATTERN_ACC : PATTERN_IND;
      if(CONFIG_SAMPLE_MODE_ADC_BOTH) {
        sprintf(FILENAME, pattern, 1, logIndex);
        storeSample(2, 0);

        sprintf(FILENAME, pattern, 2, logIndex);
        storeSample(2, GLOBAL_BUFFER_SIZE / 2);
      }
      else if(CONFIG_SAMPLE_MODE_ADC1) {
        sprintf(FILENAME, pattern, 1, logIndex);
        storeSample(2, 0);
      }
      else if(CONFIG_SAMPLE_MODE_ADC2) {
        sprintf(FILENAME, pattern, 2, logIndex);
        storeSample(2, 0);
      }
      logIndex = (logIndex + 1) % 10;
    }
    sampleIndex = 0;
    digitalWrite(SIGNAL_PIN, HIGH);
  }
}

void Storage::recordSample(uint16_t adc1, uint16_t adc2) {
  if(sampleIndex < sampleSize) {
    if(CONFIG_SAMPLE_MODE_ADC_BOTH) {
      GLOBAL_BUFFER[sampleIndex * 2] = adc1 & 0xFF;
      GLOBAL_BUFFER[sampleIndex * 2 + 1] = (adc1 >> 8) & 0xFF;
      GLOBAL_BUFFER[(GLOBAL_BUFFER_SIZE / 2) + sampleIndex * 2] = adc2 & 0xFF;
      GLOBAL_BUFFER[(GLOBAL_BUFFER_SIZE / 2) + sampleIndex * 2 + 1] = (adc2 >> 8) & 0xFF;
    }
    else if(CONFIG_SAMPLE_MODE_ADC1) {
      GLOBAL_BUFFER[sampleIndex * 2] = adc1 & 0xFF;
      GLOBAL_BUFFER[sampleIndex * 2 + 1] = (adc1 >> 8) & 0xFF;
    }
    else if(CONFIG_SAMPLE_MODE_ADC2) {
      GLOBAL_BUFFER[sampleIndex * 2] = adc2 & 0xFF;
      GLOBAL_BUFFER[sampleIndex * 2 + 1] = (adc2 >> 8) & 0xFF;
    }
    sampleIndex++;
    digitalWrite(SIGNAL_PIN, LOW);
  }
  else digitalWrite(SIGNAL_PIN, HIGH);
}

void Storage::recordTimestamp() {
  if(sampleIndex < sampleSize) {
    uint32_t currentMicros = micros();
    GLOBAL_BUFFER[4 * sampleIndex + 0] = (currentMicros >> 0x00) & 0xFF;
    GLOBAL_BUFFER[4 * sampleIndex + 1] = (currentMicros >> 0x08) & 0xFF;
    GLOBAL_BUFFER[4 * sampleIndex + 2] = (currentMicros >> 0x10) & 0xFF;
    GLOBAL_BUFFER[4 * sampleIndex + 3] = (currentMicros >> 0x18) & 0xFF;
    sampleIndex++;
    digitalWrite(SIGNAL_PIN, LOW);
  }
  else digitalWrite(SIGNAL_PIN, HIGH);
}

void Storage::blinkErrorSignal() {
  if(errorSignal) {
    if(millis() - lastMillis > 200) {
      lastMillis = millis();
      digitalToggle(SIGNAL_PIN);
    }
  }
  else digitalWrite(SIGNAL_PIN, HIGH);
}

#endif