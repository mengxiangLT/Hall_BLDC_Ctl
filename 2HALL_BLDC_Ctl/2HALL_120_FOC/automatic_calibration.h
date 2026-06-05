#ifndef __AUTOMATIC_CALIBRATION_H__
#define __AUTOMATIC_CALIBRATION_H__

#include "gd32f30x.h"

/* 将角度转换为弧度 */
#define PHASE_DIFF_DEGREE 53.81f
#define PHASE_DIFF_RAD (PHASE_DIFF_DEGREE * _PI / 180.0f)  // ≈ 0.939 rad

// automatic_calibration.h
typedef struct {
    float offset;       /* 直流偏移量 */
    float amplitude;    /* 信号幅值 */
    float max_val;
    float min_val;
    float pole_pairs;   /* 极对数 */
    float phase_diff;   /* 两个霍尔信号的电角度差（弧度）*/  // 新增
    uint8_t calibrated;
} HallCalibration;

extern HallCalibration g_Hall_A_Cal;
extern HallCalibration g_Hall_B_Cal;

void hall_calibration_start(HallCalibration *cal);
void hall_calibration_update(HallCalibration *cal, float value);
void hall_calibration_finish(HallCalibration *cal);
float hall_calibration_apply(HallCalibration *cal, float raw_value);
void hall_calibration_processing(void);

#endif
