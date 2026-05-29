#include "includes.h"

AngleUnwrapper angle_record;    			//记录前一个角度值，前一个展开后的角度及旋转的圈数
volatile float electrical_angle = 0.0f;				//电角度值，用于控制电机转动
volatile uint8_t angle_ready = 0;			//电角度已经计算出来。
#define _PI  3.14159265359f
#define _2PI 6.28318530718f
#define _3PI_2 4.71238898038f


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

/* 使用示例 */
float get_angle_from_120deg(float raw_a, float raw_b, 
                            HallCalibration *cal_a, HallCalibration *cal_b)
{
    float va, vb;
    float sin_theta, cos_theta;
    float angle;
	  float mag;
    
    /* 1. 归一化 */
    va = hall_calibration_apply(cal_a, raw_a);
    vb = hall_calibration_apply(cal_b, raw_b);
//    printf("va = %.3f, vb = %.3f\n", va, vb);
    /* 2. 计算cosθ（假设vb = sin(θ+120°)）*/
	  get_angle_120deg(va, vb, &sin_theta, &cos_theta);
//    sin_theta = va;
//    cos_theta = (vb + 0.5f * va) * 1.1547005f;
    
    /* 3. 可选：归一化幅值 */
    mag = sqrtf(sin_theta * sin_theta + cos_theta * cos_theta);
    if (mag > 0.001f) {
        sin_theta /= mag;
        cos_theta /= mag;
    }
    
    /* 4. 计算角度 */
    angle = atan2f(sin_theta, cos_theta);
    if (angle < 0) {
			  angle += 2.0f * _PI;
        /* 防止浮点误差导致等于 2π */
		}
    if (angle >= _2PI) {
        angle -= _2PI;
    }
    return angle;
}

/* 初始化角度展开器 */
void angle_unwrapper_init(AngleUnwrapper *uw, float initial_angle)
{
    uw->last_angle = initial_angle;
    uw->last_unwrapped = initial_angle;
    uw->rotations = 0;
}

/* 角度展开更新，记录旋转的圈数 */
float angle_unwrapper_update(AngleUnwrapper *uw, float angle)
{
    float delta = angle - uw->last_angle;
    
    /* 检测跨过边界 */
    if (delta > PI) {
        uw->rotations--;
    } else if (delta < -PI) {
        uw->rotations++;
    }
    
    uw->last_angle = angle;
    uw->last_unwrapped = angle + uw->rotations * _2PI;
    
    return uw->last_unwrapped;
}

/**
 * @brief 从两个霍尔信号计算机械角度（通用公式）
 * @param raw_a, raw_b ADC原始值
 * @param cal_a, cal_b 校准参数
 * @param phase_diff 两个霍尔信号的实际电角度差（弧度）
 */
#if 0
float get_angle_general(float raw_a, float raw_b, 
                        HallCalibration *cal_a, HallCalibration *cal_b,
                        float phase_diff)
{
    float va, vb;
    float sin_theta, cos_theta;
    float angle;
    float mag;
    float cos_alpha, sin_alpha;
    
    /* 1. 归一化 */
    va = hall_calibration_apply(cal_a, raw_a);
    vb = hall_calibration_apply(cal_b, raw_b);
    
    /* 2. 通用公式解算
     * 已知：
     *   va = sin(θ)
     *   vb = sin(θ + α) = sinθ·cosα + cosθ·sinα
     * 
     * 解出：
     *   sinθ = va
     *   cosθ = (vb - va·cosα) / sinα
     */
    cos_alpha = cosf(phase_diff);
    sin_alpha = sinf(phase_diff);
    
    /* 检查 sinα 是否太小 */
    if (fabsf(sin_alpha) < 0.1f) {
        /* 相位差太小时无法解算，返回上一次的角度 */
        static float last_angle = 0;
        return last_angle;
    }
    
    sin_theta = va;
    cos_theta = (vb - va * cos_alpha) / sin_alpha;
    
    /* 3. 归一化幅值（提高精度）*/
    mag = sqrtf(sin_theta * sin_theta + cos_theta * cos_theta);
    if (mag > 0.001f) {
        sin_theta /= mag;
        cos_theta /= mag;
    }
//    printf("sin_theta = %f, cos_theta = %f \r\n", sin_theta, cos_theta);
    /* 4. 计算角度 */
    angle = atan2f(sin_theta, cos_theta);
    if (angle < 0) {
        angle += _2PI;
    }
    if (angle >= _2PI) {
        angle -= _2PI;
    }
    
    return angle;
}
#endif

/* 获取展开后的电角度 */
float get_electrical_angle_unwrapped(float raw_a, float raw_b,
                                      HallCalibration *cal_a,
                                      HallCalibration *cal_b,
                                      AngleUnwrapper *uw)
{
    float mech_angle;
    float mech_angle_unwrapped;
    
    /* 1. 计算机械角度 */
    mech_angle = get_angle_from_120deg(raw_a, raw_b, cal_a, cal_b);
//	  mech_angle = get_angle_general(raw_a, raw_b, cal_a, cal_b, cal_a->phase_diff);
//	  printf("mech_angle = %.3f rad (%.1f°)\n", mech_angle, mech_angle * 180 / PI);
	
	  /* 4. 展开（用于速度计算）*/
	  mech_angle_unwrapped = angle_unwrapper_update(uw, mech_angle);
    
    /* 2. 计算电角度（原始值，可能超过2π）*/
    electrical_angle = mech_angle_unwrapped * cal_a->pole_pairs;
	  electrical_angle = mech_angle_unwrapped;
//	  printf("last_angle = %f, last_unwrapped = %f, rotations = %d, elec_angle = %f \r\n", 
//	  angle_record.last_angle, angle_record.last_unwrapped, angle_record.rotations, electrical_angle);
//    DebugPrint_log(1, 0, angle_record.last_angle, angle_record.last_unwrapped);
//	  DebugPrint_log(1, 0, angle_record.rotations, electrical_angle);
    /* 3. 归一化到 0-2π（用于FOC）*/
//    electrical_angle = fmodf(elec_angle_norm, _2PI);
    
//    angle_ready = 1;
    return electrical_angle;
}



