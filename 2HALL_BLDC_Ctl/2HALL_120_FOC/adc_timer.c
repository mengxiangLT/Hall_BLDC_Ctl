#include "includes.h"

/* 使用定时器触发ADC采样（例如TIMER2）*/
void hall_adc_timer_init(void)
{
	
	  timer_parameter_struct timer_initpara;
    /* 使能定时器时钟 */
    rcu_periph_clock_enable(RCU_TIMER2);
    
    /* 配置定时器：频率 = 10kHz（100us采样一次）*/
    timer_deinit(TIMER2);
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 5999;      /* 120MHz/12000 = 10kHz */
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1;         /* 1个计数周期 = 100us */
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER2, &timer_initpara);
    
    /* 配置触发输出 */
	  
    timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_UPDATE);
    
    /* 使能定时器 */
    timer_enable(TIMER2);
}

