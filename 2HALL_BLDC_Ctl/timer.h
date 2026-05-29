#ifndef __TIMER_H__
#define __TIMER_H__
#include "gd32f30x.h"
#include "gd32f303c_eval.h"

#define MOTOR0_PWM_TIM_CLK		RCU_TIMER0
#define MOTOR0_PWM_TIM				TIMER0

#define MOTOR1_PWM_TIM_CLK		RCU_TIMER1
#define MOTOR1_PWM_TIM				TIMER1

#define MOTOR0_PWM_TIM				TIMER0
#define MOTOR0_U_TIM_CH 			TIMER_CH_0
#define MOTOR0_V_TIM_CH 			TIMER_CH_1
#define MOTOR0_W_TIM_CH 			TIMER_CH_2

#define MOTOR1_PWM_TIM				TIMER1
#define MOTOR1_U_TIM_CH 			TIMER_CH_1
#define MOTOR1_V_TIM_CH 			TIMER_CH_2
#define MOTOR1_W_TIM_CH 			TIMER_CH_3

void timer_config_init(void);
void Moto0_U_Enable(void);
void Moto0_V_Enable(void);
void Moto0_W_Enable(void);
void Moto0_U_Set_Val(uint16_t val);
void Moto0_V_Set_Val(uint16_t val);
void Moto0_W_Set_Val(uint16_t val);
void Moto1_U_Set_Val(uint16_t val);
void Moto1_V_Set_Val(uint16_t val);
void Moto1_W_Set_Val(uint16_t val);

#endif


