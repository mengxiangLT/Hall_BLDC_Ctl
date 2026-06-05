#ifndef __GET_ANGLE_FROM_HALL_H__
#define __GET_ANGLE_FROM_HALL_H__

#include "gd32f30x.h"
#include "includes.h"
#include "sensor_angle_update.h"

#define HALL_SENSOR
#define _PI  3.14159265359f
#define _2PI 6.28318530718f
#define _3PI_2 4.71238898038f
#define POLE_PAIRS 4



/* Sensor type */
typedef struct {
	  int Mot_Num;                    /* 电机数量 */
	  float angle_prev;               /* 当前机械角度 */
	  uint32_t angle_prev_ts;         /* 采集当前机械角度的时间(s) */
	  float vel_angle_prev;           /* 上一次采集到的机械角度 */
	  uint32_t vel_angle_prev_ts;     /* 上一次采集到机械角度的时间 */
	  int32_t full_rotations;         /* 机械角度的整圈圈数 */
	  int32_t vel_full_rotations;     /* 上一次机械角度的整圈圈数 */
	  int32_t elect_angle_rotations;  /* 电角度整圈的圈数 */
	  float vel_elect_angle_prev;     /* 上一次采集到的电角度值 */
	  float raw_a;										/* Hall A的原始值 */
	  float raw_b;										/* Hall B的原始值 */
		float last_unwrapped;						/* 上一次机械角度的展开值 */
} Sensor_BLDC_Para;

/* ADC采样值存储 */
extern volatile uint16_t hall_a_raw;
extern volatile uint16_t hall_b_raw;

void Sensor_angle_unwrapper_init(Sensor_BLDC_Para *sensor, float initial_angle);
float get_angle_from_120deg(Sensor_BLDC_Para *sensor);

#endif
