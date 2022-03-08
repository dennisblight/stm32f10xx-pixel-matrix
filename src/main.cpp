#include <Arduino.h>
#ifdef NO_HAL_GPIO
#include "Hub75_nohal.h"
#else
#include "Hub75.h"
#endif
#include "Storage.h"
#include "Input.h"

Hub75* display = new Hub75();
Storage* storage = new Storage();
Renderer* renderer = new Renderer();
ACInput* acInput = new ACInput();

void readFromControl();
void SystemClock_Config();

void setup() {
  HAL_Init();
  SystemClock_Config();
  pinMode(IN_Analog1, INPUT_PULLUP);
  pinMode(IN_Analog2, INPUT_PULLUP);
  analogReadResolution(12);

  acInput->storage = storage;
  acInput->renderer = renderer;
  storage->renderer = renderer;

  storage->initialize();
  display->initialize();
  renderer->initialize();
  acInput->initialize();

  if(CONFIG_SAMPLE_MODE_NONE) {
    storage->loadBuffer();
    acInput->lastIndex = CONFIG_LAST_INDEX;
  }
}

void loop() {
  if(CONFIG_SAMPLE_MODE_NONE) {
    storage->blinkErrorSignal();
  }
  acInput->beginReadInput();
  display->renderBuffer();
}

void SystemClock_Config() {  
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}