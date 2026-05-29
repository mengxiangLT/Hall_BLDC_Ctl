#include "includes.h"

#define BUFFER_SIZE   (COUNTOF(tx_buffer))
#define COUNTOF(a)   (sizeof(a)/sizeof(*(a)))
	
uint8_t tx_buffer[10];

uint8_t rx_buffer[10];
uint16_t tx_counter = 0, rx_counter = 0;
volatile ErrStatus transfer_status = ERROR; 


/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));
    return ch;
}


void Uart_Init(uint32_t com)
{
    gd_eval_com_init(com);
    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
	  /* enable USART0 receive interrupt */
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    /* enable USART0 transmit interrupt */
//    usart_interrupt_enable(USART0, USART_INT_TBE);

//    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TC)){
//    }
}


