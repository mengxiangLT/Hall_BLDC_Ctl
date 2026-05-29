#ifndef __ADC_H__
#define __ADC_H__

#include "gd32f30x.h"


#define HALL_A_ADC_CH   ADC_CHANNEL_11
#define HALL_B_ADC_CH   ADC_CHANNEL_15


extern uint32_t adc_value[2];

void adc_config_init(void);
uint16_t adc_read_channel(uint8_t channel);

#endif
