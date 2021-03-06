#ifndef __MEMORY_H__
#define __MEMORY_H__

#define CONFIG_IDX_LAST_INDEX     0
#define CONFIG_IDX_DISPLAY_WIDTH  1
#define CONFIG_IDX_DISPLAY_HEIGHT 3
#define CONFIG_IDX_INPUT_SAMPLING 5
#define CONFIG_IDX_SAMPLE_MODE    7
#define CONFIG_IDX_SAMPLE_SIZE    8
#define CONFIG_IDX_THRESHOLD      10

#define CONFIG_GET_BYTE(INDEX)  CONFIG[INDEX]
#define CONFIG_GET_SHORT(INDEX) (CONFIG[INDEX] | (CONFIG[INDEX + 1] << 8))

#define CONFIG_LAST_INDEX     CONFIG_GET_BYTE(CONFIG_IDX_LAST_INDEX)
#define CONFIG_DISPLAY_WIDTH  CONFIG_GET_SHORT(CONFIG_IDX_DISPLAY_WIDTH)
#define CONFIG_DISPLAY_HEIGHT CONFIG_GET_SHORT(CONFIG_IDX_DISPLAY_HEIGHT)
#define CONFIG_INPUT_SAMPLING CONFIG_GET_SHORT(CONFIG_IDX_INPUT_SAMPLING)
#define CONFIG_SAMPLE_MODE    CONFIG_GET_BYTE(CONFIG_IDX_SAMPLE_MODE)
#define CONFIG_SAMPLE_SIZE    CONFIG_GET_SHORT(CONFIG_IDX_SAMPLE_SIZE)

#define CONFIG_SAMPLE_MODE_NONE         ((CONFIG_SAMPLE_MODE & 0x03) == 0x00)
#define CONFIG_SAMPLE_MODE_ACCUMULATION ((CONFIG_SAMPLE_MODE & 0x03) == 0x01)
#define CONFIG_SAMPLE_MODE_INDIVIDUAL   ((CONFIG_SAMPLE_MODE & 0x03) == 0x02)
#define CONFIG_SAMPLE_MODE_TIMESTAMP    ((CONFIG_SAMPLE_MODE & 0x03) == 0x03)
#define CONFIG_SAMPLE_MODE_ADC1         ((CONFIG_SAMPLE_MODE & 0x0C) == 0x04)
#define CONFIG_SAMPLE_MODE_ADC2         ((CONFIG_SAMPLE_MODE & 0x0C) == 0x08)
#define CONFIG_SAMPLE_MODE_ADC_BOTH     ((CONFIG_SAMPLE_MODE & 0x0C) == 0x0C)
#define CONFIG_SAMPLE_MODE_HAS_CHANNEL  ((CONFIG_SAMPLE_MODE & 0x0C) != 0x00)

#define BUFFER_READ_SHORT(INDEX) (GLOBAL_BUFFER[INDEX] | (GLOBAL_BUFFER[INDEX + 1] << 8))
#define BUFFER_READ_INT(INDEX) (BUFFER_READ_SHORT(INDEX) | (BUFFER_READ_SHORT(INDEX + 2) << 16))

#define CONFIG_SIZE (CONFIG_IDX_THRESHOLD + 36)
#ifdef USING_STM32F103C6
#define GLOBAL_BUFFER_SIZE 0x1000
#else
#define GLOBAL_BUFFER_SIZE 0x4000
#endif

unsigned char CONFIG[CONFIG_SIZE];
unsigned short THRESHOLD[18];
unsigned char GLOBAL_BUFFER[GLOBAL_BUFFER_SIZE];
char FILENAME[15];
char PATTERN_IND[] = "SAMPLE/IND%d%d.TXT";
char PATTERN_ACC[] = "SAMPLE/ACC%d%d.TXT";

#endif