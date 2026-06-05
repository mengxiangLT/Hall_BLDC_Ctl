#include "includes.h"


#define MAX_SENSOR_INIT_TIMEOUT   10000


void Hall_Sensor_Init(Sensor_BLDC_Para *sensor)
{
	 uint16_t i = 0;
	
	  for(i = 0;i < MAX_SENSOR_INIT_TIMEOUT;i++)
	  {
				if (dma_transfer_complete) {
						dma_transfer_complete = 0; // 清除标志
				
						// 处理 adc_value 数组中的数据
						S0.raw_a = hall_a_raw = (float)(adc_value[0] & 0xFFFF);
						S0.raw_b = hall_b_raw = (float)((adc_value[0] >> 16) & 0xFFFF);
					  break;
				}
		}
		if(i < MAX_SENSOR_INIT_TIMEOUT)
			 printf("sensor angle init ok! timeout = %d\r\n", i);
}

/* 初始化传感器的参数 */
void Sensor_Para_Init(Sensor_BLDC_Para *sensor, int Mot_Num)
{
    sensor->Mot_Num = Mot_Num;
    sensor->angle_prev = 0.0f;
    sensor->angle_prev_ts = 0;
    sensor->vel_angle_prev = 0.0f;
    sensor->vel_angle_prev_ts = 0;
    sensor->full_rotations = 0;
    sensor->vel_full_rotations = 0; 
	  sensor->elect_angle_rotations = 0;
	  sensor->vel_elect_angle_prev = 0;   
	  sensor->raw_a = 0;
	  sensor->raw_b = 0;
	  sensor->last_unwrapped = 0;
}

/* 初始化传感器参数值 */
void Sensor_Angle_SensorInit(Sensor_BLDC_Para *sensor)
{	
#ifdef HALL_SENSOR
	  Hall_Sensor_Init(sensor);
	  sensor->vel_angle_prev = get_angle_from_120deg(sensor);
#else
	  //    Sensor_AS5600_GetSensorAngle(sensor);
    sensor->vel_angle_prev = Sensor_AS5600_GetSensorAngle(sensor);
#endif
    sensor->vel_angle_prev_ts = _micros();
    
#ifdef HALL_SENSOR
    sensor->angle_prev = get_angle_from_120deg(sensor);	
#else
//    Sensor_AS5600_GetSensorAngle(sensor);
    sensor->angle_prev = Sensor_AS5600_GetSensorAngle(sensor);
#endif
	  sensor->angle_prev_ts = _micros();
}


/* 根据磁编码器获取的角度值，算出电机转过的圈数 */
void Sensor_BLDC_Update(Sensor_BLDC_Para *sensor)
{
#ifdef HALL_SENSOR
	//从霍尔值算出来的是电角度，极对数是4，所以需要除以4得到机械角度.
	  float val = get_angle_from_120deg(sensor);  
#else
    float val = Sensor_AS5600_GetSensorAngle(sensor);
#endif
    float d_angle;
    
    sensor->angle_prev_ts = _micros();
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 0, val, sensor->angle_prev);
#endif
	
#ifdef HALL_SENSOR
	
#else
    d_angle = val - sensor->angle_prev;
    
    /* 当角度转过一圈（2PI/360°）的80%时，就认为完成了一圈 */
#ifdef SET_ANGLE_360
		if(fabsf(d_angle) > (0.8f * 360)) { 
#else
	  if(fabsf(d_angle) > (0.8f * _2PI)) {
#endif
        if(d_angle < 0) {
            sensor->full_rotations--;
        } else {
            sensor->full_rotations++;
        }
    }
		sensor->angle_prev = val;
#endif
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, sensor->full_rotations, 0);
		RecordPrintLog(0, 3000, g_start_flag, d_angle, sensor->full_rotations);
#endif

}

float Sensor_BLDC_GetAngle(Sensor_BLDC_Para *sensor)
{
//	  Sensor_AS5600_Update(sensor);
#ifdef SET_ANGLE_360
	  return (float)-sensor->full_rotations * 360 + sensor->angle_prev;
#else
#ifdef HALL_SENSOR
	  return (float)(sensor->full_rotations * _2PI + sensor->angle_prev);
#else
    return (float)-sensor->full_rotations * _2PI + sensor->angle_prev;
#endif
#endif
}

/* 获取当前转过的角度值(0-2PI)*/
float Sensor_BLDC_GetMechanicalAngle(Sensor_BLDC_Para *sensor)
{
    return sensor->angle_prev;
}

/* 获取电角度值(0-2PI)*/
float Sensor_BLDC_GetElectricityAngle(Sensor_BLDC_Para *sensor)
{
	  return sensor->vel_elect_angle_prev;
}

float Sensor_BLDC_GetVelocity(Sensor_BLDC_Para *sensor)
{
    float Ts;
    float vel;
    
	/* 用当前时间减去之前的时间*10的-6次方，转换为秒*/
    Ts = (sensor->angle_prev_ts - sensor->vel_angle_prev_ts) * 1e-6f;
    
    /* 当时间小于等于零时，就直接赋值0.001s */
    if(Ts <= 0) Ts = 1e-3f;
    
	/* ((本时刻的圈数(full_rotations)-前一时刻的圈数(vel_full_rotations))*一圈对应的弧度(_2PI)+(当前角度-上次循环的角度))/过去的时间(Ts) */
#ifdef SET_ANGLE_360
		vel = ((float)(sensor->full_rotations - sensor->vel_full_rotations) * 360 
           + (-(sensor->angle_prev - sensor->vel_angle_prev))) / Ts;  //这里磁编码器顺时针转动数值是递减的，所以需要加个“-”转换一下
#else
	  vel = ((float)(sensor->full_rotations - sensor->vel_full_rotations) * _2PI 
           + (-(sensor->angle_prev - sensor->vel_angle_prev))) / Ts;  //这里磁编码器顺时针转动数值是递减的，所以需要加个“-”转换一下
#endif
#ifdef DEBUG_PRINT
if(vel < 0) {
		DebugPrint_log(0, 0, sensor->full_rotations, sensor->vel_full_rotations);
		DebugPrint_log(0, 0, sensor->angle_prev, sensor->vel_angle_prev);
		DebugPrint_log(0, 0, sensor->angle_prev_ts, sensor->vel_angle_prev_ts);
}
    RecordPrintLog(0, 3000, g_start_flag, sensor->angle_prev, sensor->full_rotations);
#endif
		/* 将当前参数赋值给前一时刻的参数，用于下次周期的计算 */
    sensor->vel_angle_prev = sensor->angle_prev;
    sensor->vel_full_rotations = sensor->full_rotations;
    sensor->vel_angle_prev_ts = sensor->angle_prev_ts;
    
    return vel;
}


