#ifndef __LOWPASS_FILTER_H__
#define __LOWPASS_FILTER_H__

#include "includes.h"

#define LOWPASS_TIME

#ifdef LOWPASS_TIME
/* ???????? */
typedef struct {
    float Tf;                   /* ????(?) */
    float y_prev;               /* ?????? */
    uint32_t timestamp_prev;    /* ???????(??) */
} LowPassFilter;

/* ???? */
void LowPassFilter_Init(LowPassFilter *filter, float time_constant);
float LowPassFilter_Update(LowPassFilter *filter, float x, int8_t dir);
#else
typedef struct {
    float alpha;    /* ???? 0-1,???????? */
    float y_prev;   /* ????? */
} LowPassFilter;

/* ???:alpha = dt / (Tf + dt),dt ?????(?) */
void LowPassFilter_Init(LowPassFilter *filter, float alpha);

/* ???? */
float LowPassFilter_Update(LowPassFilter *filter, float input);
#endif
#endif /* __LOWPASS_FILTER_H */


