#ifndef __PID_H__
#define __PID_H__

#include "includes.h"


/* PID ?????? */
typedef struct {
    float P;                /* ???? */
    float I;                /* ???? */
    float D;                /* ???? */
    float output_ramp;      /* ??????? */
    float limit;            /* ???? */
    
    float error_prev;       /* ????? */
    float output_prev;      /* ????? */
    float integral_prev;    /* ?????? */
    uint32_t timestamp_prev; /* ??????? */
} PIDController;

/* ???? */
void PIDController_Init(PIDController *pid, float P, float I, float D, float ramp, float limit);
float PIDController_Update(PIDController *pid, float error);

#endif /* __PID_H */

