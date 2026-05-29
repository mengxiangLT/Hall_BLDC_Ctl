#include "includes.h"

/* 全局变量 */
uint8_t voltage_power_supply = 9;  /* 注意：这个值应该是浮点数？*/

/* 辅助函数 */
float constrain_float(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/* FOC控制 */
void foc_control_task(float hall_angle)
{
    static float target_torque = 0.5f;  /* 目标力矩，-1到1之间 */
    float Ua, Ub, Uc;
    float offset;
    uint32_t pwm_duty_a, pwm_duty_b, pwm_duty_c;
    
//	  if (angle_ready) {
//        angle_ready = 0;
    /* 使用hall_angle进行FOC计算 */
    Ua = target_torque * cosf(electrical_angle);
    Ub = target_torque * cosf(electrical_angle - 2.0944f);  /* -120° */
    Uc = target_torque * cosf(electrical_angle + 2.0944f);  /* +120° */
    
    /* 偏移到0-电源电压范围 */
    offset = (float)voltage_power_supply / 2.0f;
    
    /* 输出电压限幅 */
    Ua = constrain_float(Ua + offset, 0, voltage_power_supply);
    Ub = constrain_float(Ub + offset, 0, voltage_power_supply);
    Uc = constrain_float(Uc + offset, 0, voltage_power_supply);
    
//    /* 设置PWM占空比 */
//    setPwm_speed(Ua, Ub, Uc);
	      // 4. 转换为PWM占空比并输出 (假设PWM分辨率为1000，0-1000对应0%-100%)
    pwm_duty_a = (uint32_t)((Ua / voltage_power_supply) * 255.0f);
    pwm_duty_b = (uint32_t)((Ub / voltage_power_supply) * 255.0f);
    pwm_duty_c = (uint32_t)((Uc / voltage_power_supply) * 255.0f);
    
		Moto1_U_Set_Val(pwm_duty_a);
		Moto1_V_Set_Val(pwm_duty_b);
		Moto1_W_Set_Val(pwm_duty_c);
//	}
//    set_pwm_duty(pwm_duty_a, pwm_duty_b, pwm_duty_c);
}
