/*!
    \file    systick.c
    \brief   the systick configuration file

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


#include "includes.h"

volatile static uint32_t delay;
/* sysTickUptime 在 SysTick_Handler 中累加 */
volatile uint32_t sysTickUptime = 0;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while(1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if (0U != delay){
        delay--;
    }
}

uint32_t _micros(void)
{
	uint32_t  ms = 0,cycle_cnt = 0,us = 0;
	uint32_t reload = 0;
//	printf("\r\n ms = %4d, sysTickUptime = %4d \r\n", ms, sysTickUptime);
//	printf("LOAD = %4d\n", SysTick->LOAD);
//  printf("SystemCoreClock = %4d\n", SystemCoreClock);
	do{
		ms = sysTickUptime;
		cycle_cnt = SysTick->VAL;
	} while (ms != sysTickUptime);
	/* 获取重装载值（实际计数值 = LOAD + 1）*/
  reload = SysTick->LOAD + 1;
//	printf("\r\n cycle_cnt = %4d \r\n", cycle_cnt);
	/* 计算微秒：毫秒部分 + 当前计数转换的微秒部分 */
  /* 72MHz 时：计数差值 / 72 = 微秒，但需要浮点或定点运算 */
	us = ms * 1000;
	us += (uint32_t)(((uint64_t)(reload  - cycle_cnt) * 1000000) / SystemCoreClock);  /* 先乘1000再除，避免精度丢失 */ 
	return us;
}
