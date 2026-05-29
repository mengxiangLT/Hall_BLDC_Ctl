#include "includes.h"

#ifdef LOWPASS_TIME
extern uint32_t _micros(void);  /* ?????? */

void LowPassFilter_Init(LowPassFilter *filter, float time_constant)
{
    filter->Tf = time_constant;
    filter->y_prev = 0.0f;
    filter->timestamp_prev = _micros();
}

float LowPassFilter_Update(LowPassFilter *filter, float x, int8_t dir)
{
    uint32_t timestamp = _micros();
    float dt = (timestamp - filter->timestamp_prev) * 1e-6f;
    float alpha;
    float y;

    
	/* 如果时间间隔小于0，直接赋值为0.001 */
    if(dt < 0.0f) {
        dt = 1e-3f;
    } else if(dt > 0.3f) {
        /* 如果时间间隔很大，就没有必要做滤波处理 */
        filter->y_prev = x;
        filter->timestamp_prev = timestamp;
        return x;
    }
#if 1
//		if((dir == 1 && x <= 11) || (dir == -1 && x >= -11)){
		if(dir == 1 && x <= 13){
			  filter->y_prev = 0;
        filter->timestamp_prev = timestamp;
        return 0;
		}
#endif
    /* ??????:y = alpha * y_prev + (1-alpha) * x */
    /* ?? alpha = Tf / (Tf + dt) */
    alpha = filter->Tf / (filter->Tf + dt);
    y = alpha * filter->y_prev + (1.0f - alpha) * x;
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, dt, alpha);
#endif
    filter->y_prev = y;
    filter->timestamp_prev = timestamp;
    
    return y;
}
#else
void LowPassFilter_Init(LowPassFilter *filter, float alpha)
{
    filter->alpha = alpha;
    filter->y_prev = 0.0f;
}

float LowPassFilter_Update(LowPassFilter *filter, float input)
{
    float output = filter->alpha * input + (1.0f - filter->alpha) * filter->y_prev;
    filter->y_prev = output;
    return output;
}
#endif
