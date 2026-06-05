#include "includes.h"


/* 全局校准结构体 */
HallCalibration g_Hall_A_Cal;
HallCalibration g_Hall_B_Cal;

/* 开始校准 */
void hall_calibration_start(HallCalibration *cal)
{
    cal->max_val = -1e6;
    cal->min_val = 1e6;
//    cal->calibrated = 0;
//	  cal->pole_pairs = pole_pairs; // 设置极对数，例如对于7对极的云台电机，这里就填 7.0f
//    g_Hall_A_Cal.offset = 2036.0;
//    g_Hall_A_Cal.amplitude = 1258.5;
	  g_Hall_A_Cal.offset = 2036.0;
		g_Hall_A_Cal.amplitude = 1258.0;
    g_Hall_A_Cal.pole_pairs = POLE_PAIRS;
    g_Hall_A_Cal.phase_diff = PHASE_DIFF_RAD;  // 设置你测量的相位差
    g_Hall_A_Cal.calibrated = 1;

//    g_Hall_B_Cal.offset = 2039.0;
//    g_Hall_B_Cal.amplitude = 1304.0;
	  g_Hall_B_Cal.offset = 2040.0;
    g_Hall_B_Cal.amplitude = 1302.0;
    g_Hall_B_Cal.pole_pairs = POLE_PAIRS;
    g_Hall_B_Cal.phase_diff = PHASE_DIFF_RAD;  // 使用相同的值
    g_Hall_B_Cal.calibrated = 1;
}

/* 更新校准数据（每次采样时调用）*/
void hall_calibration_update(HallCalibration *cal, float value)
{
    if (value > cal->max_val) cal->max_val = value;
    if (value < cal->min_val) cal->min_val = value;
}

/* 结束校准，计算偏移量和幅值 */
void hall_calibration_finish(HallCalibration *cal)
{
    cal->offset = (cal->max_val + cal->min_val) / 2.0f;
    cal->amplitude = (cal->max_val - cal->min_val) / 2.0f;
    cal->calibrated = 1;
	
	  /* 防止幅值为0（霍尔传感器未连接或损坏）*/
    if (cal->amplitude < 0.001f) {
        cal->amplitude = 1.0f;
    }
}

/* 应用校准，将ADC值转换为归一化正弦值（-1 到 1）*/
float hall_calibration_apply(HallCalibration *cal, float raw_value)
{
    if (!cal->calibrated) return 0;
    return (raw_value - cal->offset) / cal->amplitude;
}

/* 校准模式标志 */
uint8_t calibration_mode = 1;  /* 1=校准中, 0=正常运行 */

#if 1
/* 在主循环或定时器中断中处理 */
void hall_calibration_processing(void)
{
    static uint32_t sample_count = 0;
    
    /* 读取ADC原始值（12位，0-4095对应0-3.3V）*/
    if(calibration_mode == 1) {
			  if(sample_count < 100000)
				{
						sample_count++;
				} else {
						sample_count = 0;
					  calibration_mode = 2;
				}
		}
    else if (calibration_mode == 2) {
        /* 校准模式：收集数据 */
        hall_calibration_update(&g_Hall_A_Cal, hall_a_raw);
        hall_calibration_update(&g_Hall_B_Cal, hall_b_raw);
			  if(hall_a_raw != 0)
						sample_count++;
        
        /* 假设采样5000次（约转几圈）后结束校准 */
        if (sample_count >= 10000) {
					  sample_count = 0;
            hall_calibration_finish(&g_Hall_A_Cal);
            hall_calibration_finish(&g_Hall_B_Cal);
            calibration_mode = 0;
            
            printf("Calibration complete!\n");
            printf("Hall A: max_val = %f, min_val = %f, offset=%f, amplitude=%f\n", 
                   g_Hall_A_Cal.max_val, g_Hall_A_Cal.min_val, g_Hall_A_Cal.offset, g_Hall_A_Cal.amplitude);
            printf("Hall B: max_val = %f, min_val = %f, offset=%.2f, amplitude=%.2f\n", 
                   g_Hall_B_Cal.max_val, g_Hall_B_Cal.min_val, g_Hall_B_Cal.offset, g_Hall_B_Cal.amplitude);
        }
    } 
}
#endif


