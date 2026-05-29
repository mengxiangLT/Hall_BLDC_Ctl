#ifndef __GET_ANGLE_FROM_HALL_H__
#define __GET_ANGLE_FROM_HALL_H__

#include "gd32f30x.h"
#include "includes.h"

/* ADC采样值存储 */
extern volatile float hall_a_raw;
extern volatile float hall_b_raw;
extern volatile float electrical_angle;
extern volatile uint8_t angle_ready;
extern AngleUnwrapper angle_record;

void angle_unwrapper_init(AngleUnwrapper *uw, float initial_angle);
//void get_angle_120deg(float sin_120_val, float sin_val, float *sin_out, float *cos_out);
float get_angle_from_120deg(float raw_a, float raw_b, HallCalibration *cal_a, HallCalibration *cal_b);
float get_angle_general(float raw_a, float raw_b, 
                        HallCalibration *cal_a, 
                        HallCalibration *cal_b,
                        float phase_diff);
float get_electrical_angle_unwrapped(float raw_a, float raw_b,
                                      HallCalibration *cal_a,
                                      HallCalibration *cal_b,
                                      AngleUnwrapper *uw);

#endif
