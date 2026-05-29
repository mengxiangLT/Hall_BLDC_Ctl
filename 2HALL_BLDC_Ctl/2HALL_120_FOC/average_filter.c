#include "includes.h"



void moving_avg_init(MovingAverageFilter *f)
{
	  uint16_t i = 0;  
			
    f->index = 0;
    f->sum = 0;
    for (i = 0; i < FILTER_SIZE; i++) {
        f->buffer[i] = 0;
    }
}

uint16_t moving_avg_update(MovingAverageFilter *f, uint16_t value)
{
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = value;
    f->sum += value;
    f->index = (f->index + 1) % FILTER_SIZE;
    
    return (uint16_t)(f->sum / FILTER_SIZE);
}

