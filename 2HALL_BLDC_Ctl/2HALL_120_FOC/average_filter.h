#ifndef __AVERAGE_FILTER_H__
#define __AVERAGE_FILTER_H__

#include "gd32f30x.h"

#define FILTER_SIZE  16  /* 滤波窗口大小，必须是2的幂 */

typedef struct {
    uint16_t buffer[FILTER_SIZE];
    uint8_t index;
	  uint8_t count;      /* 当前有效数据个数 */
    uint32_t sum;
} MovingAverageFilter;

void moving_avg_init(MovingAverageFilter *f);
float moving_avg_update(MovingAverageFilter *f, uint16_t value);

#endif

