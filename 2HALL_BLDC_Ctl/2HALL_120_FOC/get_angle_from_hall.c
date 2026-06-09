#include "includes.h"


#if 1
//将电角度转换为机械角度
float Elect_Angle_Conversion_Mechanical_Angle(Sensor_BLDC_Para *sensor, float delta)
{
		//霍尔值转换的角度是电角度，所以算机械角度时需要除以极对数
		sensor->full_rotations = sensor->elect_angle_rotations/Motor_PP;
		
		//如果角度是递减的，需要用2PI-现在读到的角度，代表已经转动的角度
		//delta>0说明顺时针转动，角度是递增的
		if(delta < 0)
		{
				//当转到（电角度）边界2PI（360°）时会出现一次delta反转，通过判断电角度圈数的极性来区分走哪个逻辑运算
			  //这是电机顺时针转动的条件
			  if(sensor->elect_angle_rotations > 0)
				{
						sensor->angle_prev = sensor->vel_elect_angle_prev/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
				else //这是电机逆时针转动越界时的条件
				{
						sensor->angle_prev = -(_2PI-sensor->vel_elect_angle_prev)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
		}
		else
		{
			  //这是电机顺时针转动越界时的条件
			  if(sensor->elect_angle_rotations >= 0)
				{
						sensor->angle_prev = sensor->vel_elect_angle_prev/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
				else //这是电机逆时针转动的条件
				{
						sensor->angle_prev = -(_2PI-sensor->vel_elect_angle_prev)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
//				sensor->angle_prev = (_2PI-angle)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
		}
		//如果圈数是反转的，那么需要将读取的角度取反和圈数相加
		sensor->last_unwrapped = sensor->angle_prev + sensor->full_rotations * _2PI;
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, sensor->vel_elect_angle_prev, sensor->angle_prev);
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, sensor->full_rotations, sensor->last_unwrapped);
//		printf("vel_elect_angle_prev = %f, angle_prev = %f, elect_angle_rotations = %d, last_unwrapped = %f \r\n", sensor->vel_elect_angle_prev, sensor->angle_prev, sensor->elect_angle_rotations, sensor->last_unwrapped);
		
		return sensor->angle_prev;
}

/* 角度展开更新，记录旋转的圈数 */
float angle_unwrapper_update(Sensor_BLDC_Para *sensor, float angle)
{
    float delta;
	
		delta = angle - sensor->vel_elect_angle_prev;
    
    /* 一圈是2PI(360°)，当跨越边界时需要检测累计圈数，这里记得是电角度圈数 */
	  if(fabsf(delta) > (float)(_2PI * 0.8))
		{
			  if (delta > 0) {
						sensor->elect_angle_rotations--;
				} else{
						sensor->elect_angle_rotations++;
				}
		}
		//将当前的电角度记录下来，用于下次计算
    sensor->vel_elect_angle_prev = angle;
		Elect_Angle_Conversion_Mechanical_Angle(sensor, delta);

    return sensor->angle_prev;
}

#else
//将电角度转换为机械角度
float Elect_Angle_Conversion_Mechanical_Angle(Sensor_BLDC_Para *sensor, float delta)
{
		float delta;
	
		//霍尔值转换的角度是电角度，所以算机械角度时需要除以极对数
		sensor->full_rotations = sensor->elect_angle_rotations/Motor_PP;
		
		//如果角度是递减的，需要用2PI-现在读到的角度，代表已经转动的角度
		//delta>0说明顺时针转动，角度是递增的
		if(delta > 0)
		{
				//当转到（电角度）边界2PI（360°）时会出现一次delta反转，通过判断电角度圈数的极性来区分走哪个逻辑运算
			  //这是电机顺时针转动的条件
			  if(sensor->elect_angle_rotations <= 0)
				{
						sensor->angle_prev = -sensor->vel_elect_angle_prev/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
				else //这是电机逆时针转动越界时的条件
				{
						sensor->angle_prev = (_2PI-sensor->vel_elect_angle_prev)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
		}
		else
		{
			  //这是电机顺时针转动越界时的条件
			  if(sensor->elect_angle_rotations < 0)
				{
						sensor->angle_prev = -sensor->vel_elect_angle_prev/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
				else //这是电机逆时针转动的条件
				{
						sensor->angle_prev = (_2PI-sensor->vel_elect_angle_prev)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
				}
//				sensor->angle_prev = (_2PI-angle)/Motor_PP + _PI/2 * (sensor->elect_angle_rotations % Motor_PP);  //对4取余
		}
		//如果圈数是反转的，那么需要将读取的角度取反和圈数相加
		sensor->last_unwrapped = sensor->angle_prev + sensor->full_rotations * _2PI;
//		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, sensor->vel_elect_angle_prev, sensor->angle_prev);
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, sensor->full_rotations, sensor->last_unwrapped);
//		printf("vel_elect_angle_prev = %f, angle_prev = %f, elect_angle_rotations = %d, last_unwrapped = %f \r\n", sensor->vel_elect_angle_prev, sensor->angle_prev, sensor->elect_angle_rotations, sensor->last_unwrapped);
		return sensor->angle_prev;
}

/* 角度展开更新，记录旋转的圈数 */
float angle_unwrapper_update(Sensor_BLDC_Para *sensor, float angle)
{
    float delta;
	
		delta = angle - sensor->vel_elect_angle_prev;
    
    /* 一圈是2PI(360°)，当跨越边界时需要检测累计圈数，这里记得是电角度圈数 */
	  if(fabsf(delta) > (float)(_2PI * 0.8))
		{
			  if (delta < 0) {
						sensor->elect_angle_rotations--;
				} else{
						sensor->elect_angle_rotations++;
				}
		}
		//将当前的电角度记录下来，用于下次计算
    sensor->vel_elect_angle_prev = angle;
		Elect_Angle_Conversion_Mechanical_Angle(sensor, delta);

    return sensor->angle_prev;
}
#endif

/* 两个霍尔相差120°电角度 */
void get_angle_120deg(float sin_120_val, float sin_val, 
                       float *sin_out, float *cos_out)
{
    /* 
     * 已知: 
     *   Hall_A = sinθ
     *   Hall_B = sin(θ + 120°) = -0.5×sinθ + 0.866×cosθ
     * 
     * 推导出:
     *   sinθ = Hall_A
     *   cosθ = (Hall_B + 0.5×Hall_A) / 0.866
     */
    *sin_out = sin_120_val;  /* 实际上是 sinθ */
    *cos_out = (sin_val + 0.5f * sin_120_val) * 1.1547005f;
}

/* 从霍尔值算出来的是电角度 */
float get_angle_from_120deg(Sensor_BLDC_Para *sensor)
{
    float va, vb;
    float sin_theta, cos_theta;
    float angle;
	  float mag;
    
    /* 1. 归一化 */
    va = hall_calibration_apply(&g_Hall_A_Cal, sensor->raw_a);
    vb = hall_calibration_apply(&g_Hall_B_Cal, sensor->raw_b);
	
    /* 2. 计算cosθ（假设vb = sin(θ+120°)）*/
	  get_angle_120deg(va, vb, &sin_theta, &cos_theta);
    
    /* 3. 可选：归一化幅值 */
    mag = sqrtf(sin_theta * sin_theta + cos_theta * cos_theta);
    if (mag > 0.001f) {
        sin_theta /= mag;
        cos_theta /= mag;
    }
    
    /* 4. 计算角度 */
    angle = atan2f(sin_theta, cos_theta);
    if (angle < 0) {
			  angle += _2PI;
        /* 防止浮点误差导致等于 2π */
		}
    if (angle >= _2PI) {
        angle -= _2PI;
    }
		angle = angle_unwrapper_update(sensor, angle);
    
    /* 6. 因为电机转一圈机械角度变化 2π，但 raw_angle 可能变化了多圈 */
    /*    需要根据实际极对数或传动比进行缩放 */
    angle = fmodf(angle, _2PI);
    if (angle < 0) angle += _2PI;

    return angle;
}

void Sensor_angle_unwrapper_init(Sensor_BLDC_Para *sensor, float initial_angle)
{
    sensor->vel_angle_prev = initial_angle;
    sensor->last_unwrapped = initial_angle;
    sensor->full_rotations = 0;
}





