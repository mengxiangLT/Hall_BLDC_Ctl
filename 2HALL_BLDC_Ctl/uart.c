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

#include "gd32f30x.h"

void usart2_config(void) {
    /* 1. ???? */
    rcu_periph_clock_enable(RCU_GPIOB);      // ??GPIOB??
    rcu_periph_clock_enable(RCU_USART2);     // ??USART2??

    /* 2. ??PB10?PB11????????? */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // TX
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11); // RX

    /* 3. ????:?????????? */
    // ?PB10???USART2_TX,?PB11???USART2_RX
    gpio_pin_remap_config(GPIO_USART2_PARTIAL_REMAP, ENABLE);
	
		/* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200U);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
}


void Uart_Init(uint32_t com)
{
		if(EVAL_COM2 != com)
		{
				gd_eval_com_init(com);
			  usart2_config();		
		}
    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
	  nvic_irq_enable(USART1_IRQn, 0, 0);
	  nvic_irq_enable(USART2_IRQn, 0, 0);
	  /* enable USART0 receive interrupt */
    usart_interrupt_enable(USART0, USART_INT_RBNE);
	  usart_interrupt_enable(USART1, USART_INT_RBNE);
	  usart_interrupt_enable(USART2, USART_INT_RBNE);
    /* enable USART0 transmit interrupt */
//    usart_interrupt_enable(USART0, USART_INT_TBE);

//    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TC)){
//    }
}


