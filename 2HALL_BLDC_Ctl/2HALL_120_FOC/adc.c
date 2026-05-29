#include "includes.h"


uint32_t adc_value[2];

/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dma_config(void)
{
    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
    
	  // 使能 DMA 时钟
    rcu_periph_clock_enable(RCU_DMA0);
    /* ADC_DMA_channel deinit */
    dma_deinit(DMA0, DMA_CH0);
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr = (uint32_t)(adc_value);
    dma_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_32BIT;
    dma_data_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number = 2;
    dma_data_parameter.priority = DMA_PRIORITY_HIGH;  
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);
  
    dma_circulation_enable(DMA0, DMA_CH0);
  
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);  
}

/* ADC初始化 */
void hall_adc_init(void)
{
    /* 使能时钟 */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
	  rcu_periph_clock_enable(RCU_ADC1);
    
    /* 配置GPIO为模拟输入模式 */
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_1|GPIO_PIN_5);
    
    /* ADC配置 */
    adc_deinit(ADC0);
    adc_mode_config(ADC_DAUL_REGULAL_PARALLEL);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_special_function_config(ADC1, ADC_SCAN_MODE, ENABLE);
	
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
	  adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
	
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
	  adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 1);
    
    /* 采样时间配置 */
	  adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_11, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_15, ADC_SAMPLETIME_55POINT5);
    
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T2_TRGO);
		adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);

	  /* ADC external trigger enable */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
		adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);
	
    /* ADC时钟配置 */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);
    
    /* 使能并校准ADC */
    adc_enable(ADC0);
    delay_1ms(1);
    adc_calibration_enable(ADC0);
		
		adc_enable(ADC1);
    delay_1ms(1);
    adc_calibration_enable(ADC1);
		
    // 8. DMA配置需要调整，以读取32位的ADC0_DATA寄存器
    //    这个寄存器的高16位是ADC1数据，低16位是ADC0数据
//    dma_memory_width_config(DMA0, DMA_CH0, DMA_MEMORY_WIDTH_32BIT);
		/* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
}

void adc_config_init(void)
{
	  dma_config();
	  // 使能 DMA 传输完成中断
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel0_IRQn, 0, 0);
	  hall_adc_init();
}


