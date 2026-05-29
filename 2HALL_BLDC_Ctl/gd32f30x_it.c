/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2025-08-20, V3.0.2, demo for GD32F30x
*/

/*
    Copyright (c) 2025, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"
#include "systick.h"
#include "includes.h"

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles USART0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART0_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){
        /* read one byte from the receive data register */
        rx_buffer[rx_counter++] = (uint8_t)usart_data_receive(USART0);
			  if(rx_counter >= 2)
			  /* clear the flag of USART0 receive buffer not empty */
			  usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    }       
//    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE)){
//        /* write one byte to the transmit data register */
//        usart_data_transmit(USART0, tx_buffer[tx_counter++]);
//        /* clear the flag of USART0 transmission complete */
//			  usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
//    }
}

volatile uint8_t dma_transfer_complete = 0;

void DMA0_Channel0_IRQHandler(void)
{
    if (dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
        
        // 设置标志，通知主循环数据已更新
        dma_transfer_complete = 1;
    }
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern volatile uint32_t sysTickUptime;
void SysTick_Handler(void)
{
    delay_decrement();
	  sysTickUptime++;
}

/* 全局变量 */
extern volatile float hall_angle;
extern volatile uint8_t angle_ready;


#if 0
/* ADC中断服务函数（采样完成时触发）*/
void ADC0_IRQHandler(void)
{
    static uint8_t sample_phase = 0;
    
    if (adc_flag_get(ADC0, ADC_FLAG_EOC)) {
        uint16_t value = adc_regular_data_read(ADC0);
        
        /* 交替采样两个通道 */
        if (sample_phase == 0) {
            hall_a_raw = value;
            sample_phase = 1;
            /* 切换到通道B */
            adc_regular_channel_config(ADC0, 0, HALL_B_ADC_CH, ADC_SAMPLETIME_1POINT5);
        } else {
            hall_b_raw = value;
            sample_phase = 0;
            /* 切换回通道A */
            adc_regular_channel_config(ADC0, 0, HALL_A_ADC_CH, ADC_SAMPLETIME_1POINT5);
            
//            /* 计算角度（在中断中执行，或设置标志在主循环中计算）*/
//            hall_angle = get_angle_from_120deg(
//                (float)hall_a_raw, 
//                (float)hall_b_raw,
//                &hall_a_cal, 
//                &hall_b_cal
//            );
            angle_ready = 1;
        }
        
        adc_flag_clear(ADC0, ADC_FLAG_EOC);
    }
}
#endif
