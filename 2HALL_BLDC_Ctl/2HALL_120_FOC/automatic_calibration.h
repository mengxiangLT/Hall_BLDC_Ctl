#ifndef __AUTOMATIC_CALIBRATION_H__
#define __AUTOMATIC_CALIBRATION_H__

#include "gd32f30x.h"

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

/* 角度展开结构体 */
typedef struct {
    float last_angle;       /* 上一次角度 (0-2π) */
    float last_unwrapped;   /* 上一次展开后的角度 */
    int32_t rotations;      /* 旋转圈数 */
} AngleUnwrapper;

extern HallCalibration hall_a_cal;
extern HallCalibration hall_b_cal;

void hall_calibration_start(HallCalibration *cal);
void hall_calibration_update(HallCalibration *cal, float value);
void hall_calibration_finish(HallCalibration *cal);
float hall_calibration_apply(HallCalibration *cal, float raw_value);
void hall_calibration_processing(void);

#endif
