#include "includes.h"


/*==================== ?????? ====================*/
float voltage_power_supply_speed = 0.0f;
float Ualpha_speed = 0.0f, Ubeta_speed = 0.0f;
float Ua_speed = 0.0f, Ub_speed = 0.0f, Uc_speed = 0.0f;
float zero_electric_angle_speed = 0.0f;
#ifdef HALL_SENSOR
int Motor_PP = 4;
#else
int Motor_PP = 7;
#endif
int Sensor_DIR = 1;



/* 在 foc.c 开头添加 */
float zero_electric_angle_cw = 0.0f;    /* 顺时针零点 */
float zero_electric_angle_ccw = 0.0f;   /* 逆时针零点 */
int8_t current_direction = 1;           /* 1=逆时针, -1=顺时针 */

float _electricalAngle_speed(void);


///* 修改 _electricalAngle_speed 函数 */
//float _electricalAngle_speed_1(void)
//{
//    float elect_angle = 0;
//    float mech_angle = Sensor_BLDC_GetElectricityAngle(&S0);
//    float zero_angle;
//    
//    /* 根据当前方向选择对应的零点 */
//    if (Sensor_DIR == 1) {
//        zero_angle = zero_electric_angle_ccw;  /* 逆时针用逆时针零点 */
//    } else {
//        zero_angle = zero_electric_angle_cw;    /* 顺时针用顺时针零点 */
//    }
//    
//    elect_angle = _normalizeAngle_speed((float)(Sensor_DIR) * mech_angle - zero_angle);
//    return elect_angle;
//}

///* 修改校准函数：为两个方向分别校准 */
//void DFOC_alignSensor(int _PP, int _DIR)
//{
//    Motor_PP = _PP;
//    
//    /* ========== 校准逆时针方向 (Sensor_DIR = 1) ========== */
//    Sensor_DIR = 1;
//    printf("Calibrating CCW direction...\n");
//    
//    setTorque(3.0f, _3PI_2);
//    delay_1ms(3000);
//    zero_electric_angle_ccw = _electricalAngle_speed();
//    printf("CCW zero angle: %.3f rad\n", zero_electric_angle_ccw);
//    setTorque(0.0f, _3PI_2);
//    delay_1ms(500);
//    
//    /* ========== 校准顺时针方向 (Sensor_DIR = -1) ========== */
//    Sensor_DIR = -1;
//    printf("Calibrating CW direction...\n");
//    
//    setTorque(3.0f, _3PI_2);
//    delay_1ms(3000);
//    zero_electric_angle_cw = _electricalAngle_speed();
//    printf("CW zero angle: %.3f rad\n", zero_electric_angle_cw);
//    setTorque(0.0f, _3PI_2);
//    delay_1ms(500);
//    
//    /* 恢复默认方向 */
//    Sensor_DIR = _DIR;
//    printf("Alignment complete!\n");
//}

///* 新增：动态切换方向函数 */
//void switch_motor_direction(int8_t new_dir)
//{
//    if (new_dir == Sensor_DIR) return;
//    
//    /* 先停止电机 */
//    setTorque(0.0f, _electricalAngle_speed());
//    delay_1ms(100);
//    
//    /* 切换方向 */
//    Sensor_DIR = new_dir;
//    
//    /* 可选：重新初始化速度环 PID 积分，防止过冲 */
//    PIDController_Init(&vel_loop_M0, vel_loop_M0.P, vel_loop_M0.I, vel_loop_M0.D, 
//                       vel_loop_M0.output_ramp, vel_loop_M0.limit);
//    
//    printf("Direction switched to: %s\n", new_dir == 1 ? "CCW" : "CW");
//}


uint16_t g_print_buffer[2][MAX_RECORD_BUFF] = {0, 0};

/* ??????? */
LowPassFilter M0_Vel_Flt;

/* PID ?? */
PIDController vel_loop_M0;
PIDController angle_loop_M0;

/* AS5600 ????? */
Sensor_BLDC_Para S0;

/* ??????? */
float motor_target = 0.0f;
char received_chars[256];
int received_index = 0;


///* 开环启动函数 */
//void Openloop_Start(float target_vel_rad_s, float duration_sec)
//{
//    uint32_t start_time = _micros();
//    float angle = 0;
//    float freq = target_vel_rad_s / (2.0f * PI);  /* 转/秒 */
//    float dt = 0.001f;  /* 1ms 控制周期 */
//    
//    while ((_micros() - start_time) < duration_sec * 1000000) {
//        /* 开环：直接根据时间累积角度，不依赖传感器反馈 */
//        angle += 2.0f * PI * freq * dt;
//        angle = fmodf(angle, 2.0f * PI);
//        
//        /* 固定 Uq，开环驱动 */
//        setTorque(3.0f, angle);
//        delay_1ms(1);
//    }
//}


///* 测试电角度方向 */
//void test_electrical_angle_direction(Sensor_BLDC_Para *sensor)
//{
//    float angle_before, angle_after;
//    
//	  Hall_Sensor_Init(sensor);
//    /* 记录当前电角度 */
//    angle_before = _electricalAngle_speed();
//    
//    /* 给一个正向力矩，让电机转动一点 */
//    setTorque(0.5f, angle_before);
//    delay_1ms(500);
//    
//	  Hall_Sensor_Init(sensor);
//    /* 读取新的电角度 */
//    angle_after = _electricalAngle_speed();
//    if(angle_after > angle_before)
//		{
//			  Sensor_DIR = 1;
//		}
//		else
//		{
//				Sensor_DIR = -1;
//		}
//    printf("Before: %.3f rad, After: %.3f rad\n", angle_before, angle_after);
//    
//    /* 如果 after > before，说明正转方向正确 */
//    /* 如果 after < before，说明方向反了，需要设置 Sensor_DIR = -1 */
//}

/* ?????? [0, 2PI] */
float _normalizeAngle_speed(float angle)
{
	  float a = 0;
#ifdef SET_ANGLE_360
	  a = fmodf(angle, 360);
    return (a >= 0) ? a : (a + 360);
#else
     a = fmodf(angle, 2.0f * PI);
    return (a >= 0) ? a : (a + 2.0f * PI);
#endif
}

/* ????? */
float _electricalAngle_speed(void)
{
	 float elect_angle = 0;
#ifdef HALL_SENSOR
   elect_angle = _normalizeAngle_speed((float)(Sensor_DIR) * Sensor_BLDC_GetElectricityAngle(&S0) - zero_electric_angle_speed);
	 return elect_angle;
#else
    return _normalizeAngle_speed((float)(Sensor_DIR * Motor_PP) * Sensor_AS5600_GetMechanicalAngle(&S0) - zero_electric_angle_speed);
#endif
}


/*==================== PID 控制 ====================*/
//速度闭环PID
void DFOC_M0_SET_VEL_PID(float P, float I, float D, float ramp)
{
    vel_loop_M0.P = P;
    vel_loop_M0.I = I;
    vel_loop_M0.D = D;
    vel_loop_M0.output_ramp = ramp;
}
//力位角度闭环PID
void DFOC_M0_SET_ANGLE_PID(float P, float I, float D, float ramp)
{
    angle_loop_M0.P = P;
    angle_loop_M0.I = I;
    angle_loop_M0.D = D;
    angle_loop_M0.output_ramp = ramp;
}

void Init_Set_PID_Para(uint8_t model, float kp, float ip, float dp, float ramp)
{
		switch(model)
		{
				case 0: DFOC_M0_SET_VEL_PID(kp, ip, dp, ramp);
						break;
				case 1: DFOC_M0_SET_ANGLE_PID(kp, ip, dp, ramp);
			      break;
				default: break;
		}
}

float DFOC_M0_VEL_PID(float error)
{
    return PIDController_Update(&vel_loop_M0, error);
}

float DFOC_M0_ANGLE_PID(float error)
{
    return PIDController_Update(&angle_loop_M0, error);
}

void setPwm_speed(float Ua, float Ub, float Uc)
{
    float dc_a, dc_b, dc_c;
    
//    /* 限制速度值在有效范围内 */
//    Ua = _CONSTRAIN(Ua, 0.0f, voltage_power_supply_speed);
//    Ub = _CONSTRAIN(Ub, 0.0f, voltage_power_supply_speed);
//    Uc = _CONSTRAIN(Uc, 0.0f, voltage_power_supply_speed);
    
    /* 限制电压值在有效范围内 */
    dc_a = _CONSTRAIN(Ua / voltage_power_supply_speed, 0.0f, 1.0f);
    dc_b = _CONSTRAIN(Ub / voltage_power_supply_speed, 0.0f, 1.0f);
    dc_c = _CONSTRAIN(Uc / voltage_power_supply_speed, 0.0f, 1.0f);
    
    /* ?? PWM (8??? 0-255) */
    Moto1_U_Set_Val((uint32_t)(dc_a * 255.0f));
    Moto1_V_Set_Val((uint32_t)(dc_b * 255.0f));
    Moto1_W_Set_Val((uint32_t)(dc_c * 255.0f));
#ifdef DEBUG_PRINT
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, dc_a, dc_b);
	  DebugPrint_log(0, 500, dc_a, dc_b);
	  DebugPrint_log(0, 500, dc_c, 0);
#endif
}

void setTorque(float Uq, float angle_el)
{
	float half_vbus = voltage_power_supply_speed / 2.0f;
	
#ifdef HALL_SENSOR
	  Sensor_BLDC_Update(&S0);
#else
    Sensor_AS5600_Update(&S0);  /* ????? */
#endif
    Uq = _CONSTRAIN(Uq, -half_vbus, half_vbus);
    
    angle_el = _normalizeAngle_speed(angle_el);
#ifdef HALL_SENSOR
	  // 逆帕克变换 (输入: Uq, Ud(此处为0), 电角度 angle_el)
		Ualpha_speed =  Uq * cosf(angle_el);
		Ubeta_speed  =  Uq * sinf(angle_el);
		// 逆克拉克变换 + 中心点偏移 (SVPWM等效简化)
		Ua_speed = Ualpha_speed + half_vbus;
		Ub_speed = -0.5f * Ualpha_speed + 0.8660254f * Ubeta_speed + half_vbus;
		Uc_speed = -0.5f * Ualpha_speed - 0.8660254f * Ubeta_speed + half_vbus;
#else
    /* 帕克逆变换(Id=0) */
    Ualpha_speed = -Uq * sinf(angle_el);
    Ubeta_speed =  Uq * cosf(angle_el);
    
    /* 克拉克逆变换 (SVPWM) */
    Ua_speed = Ualpha_speed + half_vbus;
    Ub_speed = (0.8660254f * Ubeta_speed - 0.5f * Ualpha_speed) + half_vbus;    //0.8660254f=3开平方/2
    Uc_speed = (-0.8660254f * Ubeta_speed - 0.5f * Ualpha_speed) + half_vbus;
//    Ub_speed = (sqrtf(3.0f) * Ubeta_speed - Ualpha_speed) / 2.0f + voltage_power_supply_speed / 2.0f;
//    Uc_speed = (-Ualpha_speed - sqrtf(3.0f) * Ubeta_speed) / 2.0f + voltage_power_supply_speed / 2.0f;
#endif
#ifdef HALL_SENSOR
//	  /* 使用hall_angle进行FOC计算 */
//    Ua_speed = cosf(angle_el);
//    Ub_speed = cosf(angle_el - 2.0944f);  /* -120° */
//    Uc_speed = cosf(angle_el + 2.0944f);  /* +120° */
#endif
    /* 限制速度值在有效范围内 */
    Ua_speed = _CONSTRAIN(Ua_speed, 0.0f, voltage_power_supply_speed);
    Ub_speed = _CONSTRAIN(Ub_speed, 0.0f, voltage_power_supply_speed);
    Uc_speed = _CONSTRAIN(Uc_speed, 0.0f, voltage_power_supply_speed);
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, Ua_speed, Ub_speed);
//		RecordPrintLog(1, MAX_RECORD_BUFF, g_start_flag, Uc_speed, 0);
    setPwm_speed(Ua_speed, Ub_speed, Uc_speed);
//    setPwm_speed(Uc_speed, Ub_speed, Ua_speed);
}

/*==================== ????? ====================*/
void DFOC_Vbus(float power_supply)
{
#ifdef LOWPASS_TIME
#else
	float dt = 0.01f;
    float Tf = 0.1f;
    float alpha = 0.0f;
   	alpha = dt / (Tf + dt);  /* ?????? */
#endif
	
    voltage_power_supply_speed = power_supply;
//	  test_electrical_angle_direction(&S0);
    
    /* ??? AS5600 ??? */
//    Sensor_AS5600_Init(&S0, 0);
//    Sensor_AS5600_SensorInit(&S0);
    Sensor_Para_Init(&S0, 0);  
	  Sensor_Angle_SensorInit(&S0);
	
    /* ??? PID */
    PIDController_Init(&vel_loop_M0, 2.0f, 0.0f, 0.0f, 100000.0f, voltage_power_supply_speed / 2.0f);
    PIDController_Init(&angle_loop_M0, 2.0f, 0.0f, 0.0f, 0.0f, 100.0f);
    
    /* ???????? */
#ifdef LOWPASS_TIME
    LowPassFilter_Init(&M0_Vel_Flt, 0.001f);
//    LowPassFilter_Init(&M0_Vel_Flt, 0.00001f);
#else
		LowPassFilter_Init(&M0_Vel_Flt, alpha);
#endif
}

/* 校准电角度 */
void DFOC_alignSensor_1(int _PP, int _DIR)
{
    Motor_PP = _PP;
    Sensor_DIR = _DIR;
#ifdef SET_ANGLE_360
	  setTorque(3.0f, 540);  /* ?????? */
#else
    setTorque(3.0f, _3PI_2);  /* ?????? */
#endif
    delay_1ms(3000);
#ifdef HALL_SENSOR

#else
    Sensor_AS5600_Update(&S0);  /* ???? */
#endif
    zero_electric_angle_speed = _electricalAngle_speed();
#ifdef SET_ANGLE_360
	  setTorque(0.0f, 540);  /* ?????? */
#else
    setTorque(0.0f, _3PI_2);    /* ???? */
#endif
}

/*==================== ????? ====================*/
float DFOC_M0_Angle(int8_t dir)
{
	  float vel_ori, vel_flit;

#ifdef HALL_SENSOR
    vel_ori = Sensor_BLDC_GetAngle(&S0);
#else
    vel_ori = Sensor_AS5600_GetAngle(&S0);
#endif
//    vel_flit = LowPassFilter_Update(&M0_Vel_Flt, (float)Sensor_DIR * vel_ori, dir);
#ifdef DEBUG_PRINT
	  if(vel_ori != 0) {
			DebugPrint_log(0, 0, vel_ori, vel_flit);
		}
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, vel_ori, 0);
#endif
	  return vel_ori;
}

float DFOC_M0_Velocity(int8_t dir)
{
    float vel_ori, vel_flit;

#ifdef HALL_SENSOR
	  vel_ori = Sensor_BLDC_GetVelocity(&S0);
#else
    vel_ori = Sensor_AS5600_GetVelocity(&S0);
#endif
    vel_flit = LowPassFilter_Update(&M0_Vel_Flt, (float)Sensor_DIR * vel_ori, dir);
#ifdef DEBUG_PRINT
	    RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, vel_ori, vel_flit);
//	  if(vel_ori != 0.0) {
			DebugPrint_log(0, 0, vel_ori, vel_flit);
//		}
#endif
    return vel_flit;
}

/*==================== ???? ====================*/
void DFOC_M0_set_Velocity_Angle(int8_t dir, float Target)
{
#ifdef SET_ANGLE_360
	  float angle_error = Target * 360 - DFOC_M0_Angle(dir);
#else
    float angle_error = (Target - DFOC_M0_Angle(dir));
#endif
    float vel_target = DFOC_M0_ANGLE_PID(angle_error);
    float vel_error = vel_target - DFOC_M0_Velocity(dir);
    float torque = DFOC_M0_VEL_PID(vel_error);
    RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, angle_error, vel_target);
//	  RecordPrintLog(1, MAX_RECORD_BUFF, g_start_flag, vel_error, torque);
    setTorque(torque, _electricalAngle_speed());
}
//速度闭环
void DFOC_M0_setVelocity(int8_t dir, float Target)
{
	  float vel_error = 0, torque = 0;
	
	  /* 先切换方向（如果需要）*/
//    if (dir != Sensor_DIR) {
//        switch_motor_direction(dir);
//    }
//		vel_error = (Target - DFOC_M0_Velocity(dir));
#if 1
	  if(dir == 1) {
#ifdef SET_ANGLE_360
				vel_error = Target * 360 - DFOC_M0_Velocity(dir);
#else
//				vel_error = (Target - DFOC_M0_Velocity(dir)) * 180.0f / PI;
			  vel_error = (Target - DFOC_M0_Velocity(dir));
#endif
		} else {
#ifdef SET_ANGLE_360
				vel_error = DFOC_M0_Velocity(dir) - Target * 360;
#else
				vel_error = (DFOC_M0_Velocity(dir) - Target);
#endif
		}
#endif
    torque = DFOC_M0_VEL_PID(vel_error);
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 0, vel_error, torque);
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, vel_error, torque);
#endif
    setTorque(torque, _electricalAngle_speed());
}
//角度闭环
void DFOC_M0_set_Force_Angle(int8_t dir, float Target)
{
	  float angle_error, torque;
	  if(dir == 1) {
#ifdef SET_ANGLE_360
	    angle_error = Target * 360 - DFOC_M0_Angle(dir);
#else
			angle_error = (Target - DFOC_M0_Angle(dir));
#endif
		} else {
#ifdef SET_ANGLE_360
			angle_error = DFOC_M0_Angle(dir) - Target * 360;
#else
			angle_error = (DFOC_M0_Angle(dir) - Target);
#endif
		}
    torque = DFOC_M0_ANGLE_PID(angle_error);
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, angle_error, torque);
		RecordPrintLog(0, MAX_RECORD_BUFF, g_start_flag, angle_error, torque);
#endif
    setTorque(torque, _electricalAngle_speed());
}

void DFOC_M0_setTorque(float Target)
{
    setTorque(Target, _electricalAngle_speed());
}
