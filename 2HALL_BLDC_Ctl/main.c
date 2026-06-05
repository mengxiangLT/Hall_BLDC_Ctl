/*!
    \file    main.c
    \brief   TIMER Breath LED demo

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



//float set_lowpass_para = 0;
uint8_t g_start_flag;
extern uint8_t calibration_mode;
volatile uint16_t hall_a_raw;
volatile uint16_t hall_b_raw;
MovingAverageFilter Hall_A_Val;
MovingAverageFilter Hall_B_Val;


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
	  uint16_t set_speed = 0;
	
    /* configure systick */
    systick_config();
    /* configure the GPIO ports */
    gpio_config();
	  /* USART interrupt configuration */
	  Uart_Init(EVAL_COM1);
	  hall_adc_timer_init();
	  adc_config_init();
    /* 校准霍尔传感器 */
//    calibration_mode = 1;
    hall_calibration_start(&g_Hall_A_Cal);
//    hall_calibration_start(&g_Hall_B_Cal);
	  Sensor_angle_unwrapper_init(&S0,0.0);
    Moto0_U_Enable();
	  delay_1ms(200);
    /* configure the TIMER peripheral */
    timer_config_init();

    DFOC_Vbus(9);
    DFOC_M0_SET_VEL_PID(0.0001,0.01,0,0);   	//设置速度的PID值
//		DFOC_M0_SET_ANGLE_PID(0.1,0.01,0,0);				//设置速度的PID值
//    Init_Set_PID_Para(POSITION_MODEL, 0.8,0.1,0.1,0);
//		DFOC_alignSensor(Motor_PP, 1);
//    DFOC_alignSensor_1(Motor_PP, 1);
		moving_avg_init(&Hall_A_Val);
		moving_avg_init(&Hall_B_Val);
		/* 逆时针旋转 10 rad/s */
//    DFOC_M0_setVelocity(-1, 10.0f);
//    delay_1ms(5000);
    while (1){
			if(rx_counter > 1)
			{
			    set_speed = rx_buffer[0];
				  g_start_flag = rx_buffer[1];
//				  set_lowpass_para = rx_buffer[1];
//				  set_lowpass_para = (float)set_lowpass_para / 100000;
//				  LowPassFilter_Init(&M0_Vel_Flt, set_lowpass_para);
				  printf("\r\n set_speed = %x g_start_flag = %d\r\n", set_speed, g_start_flag);
				  rx_counter = 0;
			}
        /* configure TIMER channel output pulse value */
//        timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_0,i);
//				timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,i);
//        timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,i);

			  // 等待 DMA 转换完成
        if (dma_transfer_complete) {
            dma_transfer_complete = 0; // 清除标志
            
            // 处理 adc_value 数组中的数据
            S0.raw_a = hall_a_raw = (adc_value[0] & 0xFFFF);
            S0.raw_b = hall_b_raw = ((adc_value[0] >> 16) & 0xFFFF);
//					  S0.raw_a = (float)moving_avg_update(&Hall_A_Val, hall_a_raw);
//					  S0.raw_b = (float)moving_avg_update(&Hall_B_Val, hall_b_raw);
//					  printf("\r\n hall_a_raw = %d S0.raw_a = %f\r\n", hall_a_raw, S0.raw_a);
					  DFOC_M0_setVelocity(1, set_speed);
//					  DFOC_M0_set_Force_Angle(0, set_speed);
//					  DFOC_M0_set_Velocity_Angle(1, set_speed);
#if 0            
            // 运行你的 FOC 控制算法...
            get_electrical_angle_unwrapped(&S0);
						/* FOC控制 */
						foc_control_task(electrical_angle);
						RecordPrintLog(1,3000,g_start_flag,electrical_angle,0);
//						hall_calibration_processing();					
#endif
        }
    }
}

