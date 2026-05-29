#include "includes.h"

extern uint32_t _micros(void);

//#define _CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

void PIDController_Init(PIDController *pid, float P, float I, float D, float ramp, float limit)
{
    pid->P = P;
    pid->I = I;
    pid->D = D;
    pid->output_ramp = ramp;
    pid->limit = limit;
    pid->error_prev = 0.0f;
    pid->output_prev = 0.0f;
    pid->integral_prev = 0.0f;
    pid->timestamp_prev = _micros();
}

float PIDController_Update(PIDController *pid, float error)
{
    uint32_t timestamp_now = _micros();
    float Ts = (timestamp_now - pid->timestamp_prev) * 1e-6f;
    float proportional;
    float integral;
    float derivative;
    float output;
    
	/* 修复micros函数跳变 */
    if(Ts <= 0 || Ts > 0.5f) {
        Ts = 1e-3f;
    }
    
    /* 比例值*误差 */
    proportional = pid->P * error;
    
    /* Tustin变换的离散积分公式 */
    integral = pid->integral_prev + pid->I * Ts * 0.5f * (error + pid->error_prev); //散点积分叠加
    integral = _CONSTRAIN(integral, -pid->limit, pid->limit);
    
    /* D环(微分环节) */
    derivative = pid->D * (error - pid->error_prev) / Ts;
    
    /* 将P, I, D三环的计算值加起来 */
    output = proportional + integral + derivative;
    output = _CONSTRAIN(output, -pid->limit, pid->limit);
    
    /* 对PID的变化速度进行限制 */
    if(pid->output_ramp > 0) {
        float output_rate = (output - pid->output_prev) / Ts;
        if(output_rate > pid->output_ramp) {
            output = pid->output_prev + pid->output_ramp * Ts;
        } else if(output_rate < -pid->output_ramp) {
            output = pid->output_prev - pid->output_ramp * Ts;
        }
    }
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, timestamp_now, pid->timestamp_prev);
#endif
    /* 保存值，用于下一周期计算 */
    pid->integral_prev = integral;
    pid->output_prev = output;
    pid->error_prev = error;
    pid->timestamp_prev = timestamp_now;
    return output;
}

