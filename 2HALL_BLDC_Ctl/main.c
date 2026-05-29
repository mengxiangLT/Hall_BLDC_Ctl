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



float set_lowpass_para = 0;
uint8_t g_start_flag;
extern uint8_t calibration_mode;
extern volatile float hall_angle;
volatile float hall_a_raw;
volatile float hall_b_raw;

//void measure_phase_difference(void)
//{
//    float raw_a, raw_b;
//    float va, vb;
//    static float last_va = 0;
//    static int cross_zero_count = 0;
//    static float vb_at_zero[10];
//    
//    raw_a = (float)(adc_value & 0xFFFF);
//    raw_b = (float)((adc_value >> 16) & 0xFFFF);
//    
//    va = hall_calibration_apply(&hall_a_cal, raw_a);
//    vb = hall_calibration_apply(&hall_b_cal, raw_b);
//    
//    /* 检测 va 过零点（从负到正）*/
//    if (last_va < 0 && va >= 0 && cross_zero_count < 10) {
//        /* 当 va = sinθ = 0 时，vb = sin(θ + α) = sin(α) */
//        vb_at_zero[cross_zero_count] = vb;
//        cross_zero_count++;
//        
//        printf("Zero cross %d: vb = %.4f, alpha = %.4f rad (%.1f°)\n", 
//               cross_zero_count, vb, asinf(vb), asinf(vb) * 180 / PI);
//    }
//    last_va = va;
//}

//// 测试代码
//void test_mech_angle(void)
//{
//    static uint32_t last_time = 0;
//    uint32_t now = _micros();
//    float raw_a, raw_b;
//    float mech_angle;
//    
//    raw_a = (float)(adc_value & 0xFFFF);
//    raw_b = (float)((adc_value >> 16) & 0xFFFF);
//    
//    // 使用新函数
//    mech_angle = get_angle_general(raw_a, raw_b, &hall_a_cal, &hall_b_cal, 
//                                    hall_a_cal.phase_diff);
//    
//    if (now - last_time > 100000) {  // 100ms
//        last_time = now;
//        printf("mech_angle = %.3f rad (%.1f°)\n", mech_angle, mech_angle * 180 / PI);
//    }
//}
//void debug_va_vb(void)
//{
//    float raw_a, raw_b;
//    float va, vb;
//    
//    raw_a = (float)(adc_value & 0xFFFF);
//    raw_b = (float)((adc_value >> 16) & 0xFFFF);
//    
//    va = hall_calibration_apply(&hall_a_cal, raw_a);
//    vb = hall_calibration_apply(&hall_b_cal, raw_b);
//    
//    printf("va=%.4f, vb=%.4f\n", va, vb);
//}

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
    hall_calibration_start(&hall_a_cal);
//    hall_calibration_start(&hall_b_cal);
    angle_unwrapper_init(&angle_record, 0.0);
    Moto0_U_Enable();
	  delay_1ms(200);
    /* configure the TIMER peripheral */
    timer_config_init();
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
            hall_a_raw = (float)(adc_value[0] & 0xFFFF);
            hall_b_raw = (float)((adc_value[0] >> 16) & 0xFFFF);
            
            // 运行你的 FOC 控制算法...
            get_electrical_angle_unwrapped(
											(float)hall_a_raw,
											(float)hall_b_raw,
											&hall_a_cal,
											&hall_b_cal,
											&angle_record);
						/* FOC控制 */
						foc_control_task(electrical_angle);
						RecordPrintLog(1,3000,g_start_flag,electrical_angle,0);
//						hall_calibration_processing();					
        }
    }
}

