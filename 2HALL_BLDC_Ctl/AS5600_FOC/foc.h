#ifndef __FOC_H__
#define __FOC_H__

#include "includes.h"


/*==================== ???? ====================*/
#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define _3PI_2 4.71238898038f
#define PI 3.14159265359f

/* ?????? */
extern float voltage_power_supply_speed;
extern float Ualpha_speed, Ubeta_speed, Ua_speed, Ub_speed, Uc_speed;
extern float zero_electric_angle_speed;
extern int Motor_PP, Sensor_DIR;
extern int pwmA, pwmB, pwmC;

/* ????? */
extern LowPassFilter M0_Vel_Flt;

/* PID ?? */
extern PIDController vel_loop_M0;
extern PIDController angle_loop_M0;

/* AS5600 ????? */
extern Sensor_AS5600 S0;

/* ???? */
void DFOC_Vbus(float power_supply);
void DFOC_alignSensor(int _PP, int _DIR);
float _normalizeAngle_speed(float angle);
float _electricalAngle_speed(void);
void setPwm_speed(float Ua_speed, float Ub_speed, float Uc_speed);
void setTorque(float Uq, float angle_el);

/* PID ???? */
void DFOC_M0_SET_VEL_PID(float P, float I, float D, float ramp);
void DFOC_M0_SET_ANGLE_PID(float P, float I, float D, float ramp);
float DFOC_M0_VEL_PID(float error);
float DFOC_M0_ANGLE_PID(float error);

/* ????? */
float DFOC_M0_Angle(int8_t dir);
float DFOC_M0_Velocity(int8_t dir);

/* ???? */
void DFOC_M0_set_Velocity_Angle(int8_t dir, float Target);
void DFOC_M0_setVelocity(int8_t dir, float Target);
void DFOC_M0_set_Force_Angle(int8_t dir, float Target);
void DFOC_M0_setTorque(float Target);

/* ???? */
float serial_motor_target(void);
void serialReceiveUserCommand(void);

#endif /* __FOC_H */

