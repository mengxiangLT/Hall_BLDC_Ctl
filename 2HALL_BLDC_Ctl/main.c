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
//#include "gd32f30x.h"


//float set_lowpass_para = 0;
uint8_t g_start_flag;
extern uint8_t calibration_mode;
volatile uint16_t hall_a_raw;
volatile uint16_t hall_b_raw;
MovingAverageFilter Hall_A_Val;
MovingAverageFilter Hall_B_Val;


#define RUN_CODE 4
/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
float set_speed = 0;
		
#if (RUN_CODE == 1)
int main(void)
{

	
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
//    DFOC_M0_SET_VEL_PID(0.0001,0.01,0,0);   	//设置速度的PID值
//		DFOC_M0_SET_ANGLE_PID(0.1,0.01,0,0);				//设置速度的PID值
    DFOC_M0_SET_VEL_PID(0.8,0.1,0,0);   	//设置速度的PID值
    Init_Set_PID_Para(POSITION_MODEL, 0.2,0.1,0,0);
    Position_Para_Init(&g_Position_Value);
//		DFOC_alignSensor(Motor_PP, 1);
    DFOC_alignSensor_1(Motor_PP, 1);
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
            hall_a_raw = (adc_value[0] & 0xFFFF);
            hall_b_raw = ((adc_value[0] >> 16) & 0xFFFF);
					  S0.raw_a = (float)moving_avg_update(&Hall_A_Val, hall_a_raw);
					  S0.raw_b = (float)moving_avg_update(&Hall_B_Val, hall_b_raw);
//					  printf("\r\n hall_a_raw = %d S0.raw_a = %f\r\n", hall_a_raw, S0.raw_a);
//					  printf("\r\n hall_b_raw = %d S0.raw_b = %f\r\n", hall_b_raw, S0.raw_b);
//					  DFOC_M0_setVelocity(0, set_speed);
//					  g_Position_Value.target_velocity = set_speed;
//					  g_Position_Value.target_position = set_speed;
//					  position_control_update(&g_Position_Value);
					  DFOC_M0_set_Force_Angle(0, set_speed);
//					  DFOC_M0_set_Velocity_Angle(0, set_speed);
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
#elif  (RUN_CODE == 2)

// 线程函数声明
void Thread_LED(void *argument);
void Thread_UART(void *argument);

// 线程ID
osThreadId_t tid_LED;
osThreadId_t tid_UART;

// 使用默认配置（不指定栈，系统自动分配）
static const osThreadAttr_t LED_attr = {
    .name = "LED",
    .priority = osPriorityNormal,
    .stack_size = 512      // 单位：字节
};

static const osThreadAttr_t UART_attr = {
    .name = "UART",
    .priority = osPriorityNormal,  // 先和LED同优先级，避免优先级问题
    .stack_size = 512
};

int main(void)
{
    // 硬件初始化
    systick_config();           // 系统滴答定时器
    Uart_Init(EVAL_COM1);       // 串口初始化
    
    // 在启动调度器前打印（仅用于调试，确保串口工作正常）
    printf("System init done, creating tasks...\r\n");
    
    // 创建线程
    tid_LED = osThreadNew(Thread_LED, NULL, &LED_attr);
    tid_UART = osThreadNew(Thread_UART, NULL, &UART_attr);
    
    // 检查线程是否创建成功
    if (tid_LED == NULL || tid_UART == NULL) {
        printf("Task creation failed!\r\n");
        while(1);  // 挂起
    }
    
    printf("Tasks created, starting kernel...\r\n");
    
    // 启动 RTOS 调度器（之后不会再执行到这里）
    osKernelStart();
    
    // 程序不会运行到这里
    while(1);
}

void Thread_LED(void *argument)
{
    (void)argument;
    while(1) {
        // 闪烁 LED（根据你的硬件修改引脚）
        // gpio_bit_toggle(GPIOB, GPIO_PIN_12);
        printf("LED task running\r\n");  // 临时调试输出
        osDelay(500);
    }
}

void Thread_UART(void *argument)
{
    (void)argument;
    while(1) {
        printf("RTX5 is running on GD32F303...\r\n");
        osDelay(1000);
    }
}
#elif  (RUN_CODE == 3)
RingBuffer g_GetRingBuffer;

/* 主循环中处理Pelco-D命令 */
void pelco_d_task(void)
{
    PelcoD_Command cmd;
    PelcoD_ParseResult result;
    
    /* 尝试从环形队列中解析命令 */
    if (pelco_d_ring_parse_command(&g_pelco_ring, &cmd)) {
        /* 解析命令 */
        pelco_d_parse_command(&cmd, &result);
        
        /* 处理命令 */
        if (result.valid) {
            pelco_d_process_command(&result);
        }
    }
}

int main(void)
{    
		uint32_t available;
	  uint32_t dropped;
	  uint32_t last_monitor;
	  
    // 硬件初始化
    systick_config();           // 系统滴答定时器
    Uart_Init(EVAL_COM1);       // 串口初始化
    ring_buffer_init(&g_GetRingBuffer);
	  pelco_d_ring_init(&g_pelco_ring);
    /* 初始化串口（2400波特率） */
    pelco_d_uart_init(2400);
    
    /* 设置设备地址 */
    pelco_d_set_address(0x01);
    
    while (1) {
        /* 处理Pelco-D命令 */
        pelco_d_task();
        
        /* 可选：监控队列状态 */
        last_monitor = 0;
        if (_micros() - last_monitor > 1000000) {
            last_monitor = _micros();
            
            /* 打印队列状态（调试用） */
            available = ring_buffer_available(&g_pelco_ring.rx_ring);
            dropped = g_pelco_ring.rx_ring.dropped;
            
            if (available > 0 || dropped > 0) {
                /* 通过调试接口输出状态 */
                printf("Ring: available=%d, dropped=%d\r\n", available, dropped);
            }
        }
    }
}
#elif  (RUN_CODE == 4)
#include "gd32f30x.h"
#include "visca.h"

/* 全局VISCA接收器 */
ViscaReceiver g_visca_receiver;

/**
 * @brief 主任务：处理VISCA命令
 */
#if 0
/* 接收处理示例 */
void visca_task(void)
{
    ViscaPacket packet;
    ViscaParsedCommand cmd;
    
    while (visca_get_packet(&g_visca_receiver, &packet)) {
        /* 解析命令 */
        visca_parse_packet(&packet, &cmd);
        
        /* 发送ACK */
        if (cmd.command_type == VISCA_CMD_COMMAND) {
            visca_send_ack(&g_visca_receiver, &packet);
        }
        
        /* 处理命令 */
        visca_process_command(&cmd);
        
        /* 发送完成包 */
        if (cmd.command_type == VISCA_CMD_COMMAND) {
            visca_send_completion(&g_visca_receiver, &packet);
        }
    }
}
#else
void visca_task(void)
{
    ViscaPacket packet;
    ViscaParsedCommand cmd;
    static uint32_t last_debug = 0;
    uint32_t now = _micros()/1000;
    
    /* 获取并处理数据包 */
    while (visca_get_packet(&g_visca_receiver, &packet)) {
        /* 解析命令 */
        visca_parse_packet(&packet, &cmd);
        
        /* 可选：发送ACK */
        if (cmd.command_type == VISCA_CMD_COMMAND) {
            visca_send_ack(&g_visca_receiver, &packet);
        }
        
        /* 处理命令 */
        visca_process_command(&cmd);
        
        /* 可选：发送完成包 */
        if (cmd.command_type == VISCA_CMD_COMMAND) {
            visca_send_completion(&g_visca_receiver, &packet);
        }
        
        /* 查询包需要返回数据 */
        if (cmd.command_type == VISCA_CMD_INQUIRY) {
            /* 根据查询内容构造返回数据 */
            uint8_t inquiry_response[] = {0x90, 0x50, 0x00, 0x00, 0xFF};
            visca_send_packet(inquiry_response, sizeof(inquiry_response));
        }
    }
    
    /* 调试信息：每1秒输出队列状态 */
    if (now - last_debug > 1000) {
        last_debug = now;
        uint32_t available = visca_ring_buffer_available(&g_visca_receiver.rx_ring);
        uint32_t overflow = g_visca_receiver.rx_ring.overflow_cnt;
        
        if (available > 0 || overflow > 0) {
            /* 通过调试输出（如USART1打印） */
            // printf("Queue: available=%lu, overflow=%lu\r\n", available, overflow);
        }
    }
}
#endif
/**
 * @brief 发送PTZ控制命令示例
 */
void visca_send_ptz_command(uint8_t camera_id, uint8_t direction, uint8_t speed)
{
    uint8_t cmd[8];
    uint8_t idx = 0;
    
    cmd[idx++] = 0x80 | camera_id;           /* 地址 */
    cmd[idx++] = 0x01;                       /* 命令包 */
    cmd[idx++] = VISCA_CATEGORY_PTZ;          /* 云台类别 */
    cmd[idx++] = direction;                  /* 方向 */
    cmd[idx++] = speed;                      /* 水平速度 */
    cmd[idx++] = speed;                      /* 垂直速度 */
    cmd[idx++] = VISCA_TERMINATOR;           /* 终止符 */
    
    visca_send_packet(cmd, idx);
}

/**
 * @brief 发送变焦控制命令示例
 */
void visca_send_zoom_command(uint8_t camera_id, uint8_t operation, uint8_t speed)
{
    uint8_t cmd[6];
    uint8_t idx = 0;
    
    cmd[idx++] = 0x80 | camera_id;
    cmd[idx++] = 0x01;
    cmd[idx++] = VISCA_CATEGORY_ZOOM;
    cmd[idx++] = operation;
    cmd[idx++] = speed;
    cmd[idx++] = VISCA_TERMINATOR;
    
    visca_send_packet(cmd, idx);
}

/**
 * @brief 主函数
 */
int main(void)
{
    systick_config();           // 系统滴答定时器
    /* 初始化VISCA接收器 (设备地址1) */
    visca_receiver_init(&g_visca_receiver, 0x01);   
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
//    DFOC_M0_SET_VEL_PID(0.0001,0.01,0,0);   	//设置速度的PID值
//		DFOC_M0_SET_ANGLE_PID(0.1,0.01,0,0);				//设置速度的PID值
    DFOC_M0_SET_VEL_PID(0.8,0.1,0,0);   	//设置速度的PID值
    Init_Set_PID_Para(POSITION_MODEL, 0.2,0.1,0,0);
    Position_Para_Init(&g_Position_Value);
//		DFOC_alignSensor(Motor_PP, 1);
    DFOC_alignSensor_1(Motor_PP, 1);
		moving_avg_init(&Hall_A_Val);
		moving_avg_init(&Hall_B_Val);
		/* 逆时针旋转 10 rad/s */
//    DFOC_M0_setVelocity(-1, 10.0f);
//    delay_1ms(5000);
    while (1){
			  /* 处理VISCA命令 */
        visca_task();
			  // 等待 DMA 转换完成
        if (dma_transfer_complete) {
            dma_transfer_complete = 0; // 清除标志
            
            // 处理 adc_value 数组中的数据
            hall_a_raw = (adc_value[0] & 0xFFFF);
            hall_b_raw = ((adc_value[0] >> 16) & 0xFFFF);
					  S0.raw_a = (float)moving_avg_update(&Hall_A_Val, hall_a_raw);
					  S0.raw_b = (float)moving_avg_update(&Hall_B_Val, hall_b_raw);
					  
					  DFOC_M0_set_Force_Angle(0, (float)set_speed);
//					  DFOC_M0_set_Velocity_Angle(0, set_speed);
        }
//				GPIO_BC(GPIOB) = GPIO_PIN_10;
//				GPIO_BC(GPIOB) = GPIO_PIN_11;
//				delay_1ms(1000);
//				GPIO_BOP(GPIOB) = GPIO_PIN_11;
//				GPIO_BOP(GPIOB) = GPIO_PIN_10;
//				delay_1ms(1000);
		}
}

#endif

