#ifndef __INPUT_H__
#define __INPUT_H__

#include <Arduino.h>
#include "Memory.h"
#include "Storage.h"

#define IN_Trigger PA1
#define IN_Analog1 PA2 // satuan
#define IN_Analog2 PA0 // puluhan

class ACInput {
  private:
    uint32_t lastMicros = 0;
    uint32_t counter = 0;
    uint32_t accInput1 = 0;
    uint32_t accInput2 = 0;
    uint32_t sumTrigger = 0;
    uint32_t sampleSize = 0;
    void applyThreshold(uint32_t adc1, uint32_t adc2);
    void readInput();
  public:
    bool triggerLock = false;
    bool logDataSampling = false;
    int32_t logCounter = 0;
    int32_t lastIndex = -1;
    int32_t index1 = 0;
    int32_t index2 = 0;
    Storage* storage;
    Renderer* renderer;
    uint32_t triggerStart = 0;
    uint32_t triggerDuration = 0;
    void initialize();
    int32_t getIndex();
    void beginReadInput();
};

void ACInput::initialize() {
  sampleSize = CONFIG_INPUT_SAMPLING;
  for(uint8_t i = 0; i < 18; i++) {
    THRESHOLD[i] = CONFIG_GET_SHORT(CONFIG_IDX_THRESHOLD + i * 2);
  }
}

int32_t ACInput::getIndex() {
  return index1 + index2 * 10;
}

void ACInput::applyThreshold(uint32_t adc1, uint32_t adc2) {
  index1 = index2 = 9;
  for(uint8_t i = 0; i < 9; i++) {
    if(adc1 <= THRESHOLD[i] && index1 == 9) {
      index1 = i;
    }
    if(adc2 <= THRESHOLD[i + 9] && index2 == 9) {
      index2 = i;
    }
  }
}

void ACInput::beginReadInput() {
  if(millis() <= 1000) return;
  while(micros() - lastMicros < 7111) ;
  lastMicros = micros();

  int32_t trigger = digitalRead(IN_Trigger);
  if(trigger == 0) {
    if(!triggerLock) {
      triggerLock = true;
      triggerStart = millis();
    }
    sumTrigger = 0;
  }
  else sumTrigger++;

  // reset and stop sampling
  if(sumTrigger > 40) {
    if(triggerLock) {
      storage->saveSample();
      triggerDuration = millis() - triggerStart - 500;
    }
    triggerLock = false;
  }

  if(triggerLock) {
    uint32_t deltaMillis = millis() - triggerStart;
    if(1000 < deltaMillis) {
      readInput();
    }

    int32_t index = getIndex();
    if(CONFIG_SAMPLE_MODE_NONE) {
      if(index != lastIndex) {
        lastIndex = index;
        storage->loadBuffer(index);
        storage->beginSaveIndex(index);
      }
      else storage->saveIndex();
    }
    // if(index != lastIndex && CONFIG_SAMPLE_MODE_NONE) {
    //   lastIndex = index;
    //   storage->loadBuffer(index);
    //   storage->saveIndex(index);
    // }
  }
}

void ACInput::readInput() {
  if(counter >= sampleSize) {
    accInput1 = accInput1 / counter;
    accInput2 = accInput2 / counter;
    
    applyThreshold(accInput1, accInput2);
    if(CONFIG_SAMPLE_MODE_ACCUMULATION) {
      storage->recordSample(accInput1, accInput2);
    }
    accInput1 = accInput2 = counter = 0;
  }

  int32_t adc1 = analogRead(IN_Analog1);
  int32_t adc2 = analogRead(IN_Analog2);

  accInput1 += adc1;
  accInput2 += adc2;
  counter++;

  if(CONFIG_SAMPLE_MODE_TIMESTAMP) {
    storage->recordTimestamp();
  }
  else if(CONFIG_SAMPLE_MODE_INDIVIDUAL) {
    storage->recordSample(adc1, adc2);
  }
}

#endif
