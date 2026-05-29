#ifndef __UART_H__
#define __UART_H__

#include "includes.h"

extern uint8_t   tx_buffer[];
extern uint8_t   rx_buffer[] ;
extern uint16_t  tx_counter, rx_counter;

void Uart_Init(uint32_t com);


#endif

