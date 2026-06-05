#include "includes.h"



void moving_avg_init(MovingAverageFilter *f)
{
	  uint16_t i = 0;  
			
    f->index = 0;
	  f->count = 0;
    f->sum = 0;
    for (i = 0; i < FILTER_SIZE; i++) {
        f->buffer[i] = 0;
    }
}

float moving_avg_update(MovingAverageFilter *f, uint16_t value)
{
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = value;
    f->sum += value;
    f->index = (f->index + 1) % FILTER_SIZE;
	  if(f->count < FILTER_SIZE)
		{
				f->count++;
			  return (float)(f->sum / f->count);
		}
    
    return (float)(f->sum / FILTER_SIZE);
}

