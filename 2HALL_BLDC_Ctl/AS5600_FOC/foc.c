#include "includes.h"


/*==================== ?????? ====================*/
float voltage_power_supply_speed = 0.0f;
float Ualpha_speed = 0.0f, Ubeta_speed = 0.0f;
float Ua_speed = 0.0f, Ub_speed = 0.0f, Uc_speed = 0.0f;
float zero_electric_angle_speed = 0.0f;
int Motor_PP = 7;
int Sensor_DIR = 1;


uint16_t g_print_buffer[2][MAX_RECORD_BUFF] = {0, 0};
uint8_t g_start_flag = 0;

/* ??????? */
LowPassFilter M0_Vel_Flt;

/* PID ?? */
PIDController vel_loop_M0;
PIDController angle_loop_M0;

/* AS5600 ????? */
Sensor_AS5600 S0;

/* ??????? */
float motor_target = 0.0f;
char received_chars[256];
int received_index = 0;


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
    return _normalizeAngle_speed((float)(Sensor_DIR * Motor_PP) * Sensor_AS5600_GetMechanicalAngle(&S0) - zero_electric_angle_speed);
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
    
    /* 限制速度值在有效范围内 */
    Ua = _CONSTRAIN(Ua, 0.0f, voltage_power_supply_speed);
    Ub = _CONSTRAIN(Ub, 0.0f, voltage_power_supply_speed);
    Uc = _CONSTRAIN(Uc, 0.0f, voltage_power_supply_speed);
    
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
    Sensor_AS5600_Update(&S0);  /* ????? */
    
    Uq = _CONSTRAIN(Uq, -voltage_power_supply_speed / 2.0f, voltage_power_supply_speed / 2.0f);
    
    angle_el = _normalizeAngle_speed(angle_el);
    
    /* 帕克逆变换(Id=0) */
    Ualpha_speed = -Uq * sinf(angle_el);
    Ubeta_speed =  Uq * cosf(angle_el);
    
    /* 克拉克逆变换 (SVPWM) */
    Ua_speed = Ualpha_speed + voltage_power_supply_speed / 2.0f;
    Ub_speed = (sqrtf(3.0f) * Ubeta_speed - Ualpha_speed) / 2.0f + voltage_power_supply_speed / 2.0f;
    Uc_speed = (-Ualpha_speed - sqrtf(3.0f) * Ubeta_speed) / 2.0f + voltage_power_supply_speed / 2.0f;
    
    setPwm_speed(Ua_speed, Ub_speed, Uc_speed);
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
    
    /* ??? AS5600 ??? */
    Sensor_AS5600_Init(&S0, 0);
    Sensor_AS5600_SensorInit(&S0);
    
    /* ??? PID */
    PIDController_Init(&vel_loop_M0, 2.0f, 0.0f, 0.0f, 100000.0f, voltage_power_supply_speed / 2.0f);
    PIDController_Init(&angle_loop_M0, 2.0f, 0.0f, 0.0f, 100000.0f, 100.0f);
    
    /* ???????? */
#ifdef LOWPASS_TIME
    LowPassFilter_Init(&M0_Vel_Flt, 0.00001f);
#else
		LowPassFilter_Init(&M0_Vel_Flt, alpha);
#endif
}

/* 校准电角度 */
void DFOC_alignSensor(int _PP, int _DIR)
{
    Motor_PP = _PP;
    Sensor_DIR = _DIR;
#ifdef SET_ANGLE_360
	  setTorque(3.0f, 540);  /* ?????? */
#else
    setTorque(3.0f, _3PI_2);  /* ?????? */
#endif
    delay_1ms(3000);
    
    Sensor_AS5600_Update(&S0);  /* ???? */
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
	
    vel_ori = Sensor_AS5600_GetAngle(&S0);
//    vel_flit = LowPassFilter_Update(&M0_Vel_Flt, (float)Sensor_DIR * vel_ori, dir);
#ifdef DEBUG_PRINT
	  if(vel_ori != 0) {
			DebugPrint_log(0, 0, vel_ori, vel_flit);
		}
#endif
	  return vel_ori;
}

float DFOC_M0_Velocity(int8_t dir)
{
    float vel_ori, vel_flit;
	
    vel_ori = Sensor_AS5600_GetVelocity(&S0);
    vel_flit = LowPassFilter_Update(&M0_Vel_Flt, (float)Sensor_DIR * vel_ori, dir);
#ifdef DEBUG_PRINT
	    RecordPrintLog(1, MAX_RECORD_BUFF, g_start_flag, vel_ori, vel_flit);
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
    float angle_error = (Target - DFOC_M0_Angle(dir)) * 180.0f / PI;
#endif
    float vel_target = DFOC_M0_ANGLE_PID(angle_error);
    float vel_error = vel_target - DFOC_M0_Velocity(dir);
    float torque = DFOC_M0_VEL_PID(vel_error);
    
    setTorque(torque, _electricalAngle_speed());
}
//速度闭环
void DFOC_M0_setVelocity(int8_t dir, float Target)
{
	  float vel_error = 0, torque = 0;
	  if(dir == 1) {
#ifdef SET_ANGLE_360
				vel_error = Target * 360 - DFOC_M0_Velocity(dir);
#else
				vel_error = (Target - DFOC_M0_Velocity(dir)) * 180.0f / PI;
#endif
		} else {
#ifdef SET_ANGLE_360
				vel_error = DFOC_M0_Velocity(dir) - Target * 360;
#else
				vel_error = (DFOC_M0_Velocity(dir) + Target) * 180.0f / PI;
#endif
		}
    torque = DFOC_M0_VEL_PID(vel_error);
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 0, vel_error, torque);
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
			angle_error = (Target - DFOC_M0_Angle(dir)) * 180.0f / PI;
#endif
		} else {
#ifdef SET_ANGLE_360
			angle_error = DFOC_M0_Angle(dir) - Target * 360;
#else
			angle_error = (DFOC_M0_Angle(dir) - Target) * 180.0f / PI;
#endif
		}
    torque = DFOC_M0_ANGLE_PID(angle_error);
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, angle_error, torque);
#endif
    setTorque(torque, _electricalAngle_speed());
}

void DFOC_M0_setTorque(float Target)
{
    setTorque(Target, _electricalAngle_speed());
}
