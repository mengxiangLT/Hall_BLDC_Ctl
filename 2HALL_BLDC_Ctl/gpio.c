#include "includes.h"


/**
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
  */
void gpio_config(void)
{
    rcu_periph_clock_enable(MOTOR0_GPIO_CLK);
    rcu_periph_clock_enable(MOTOR1_GPIO_CLK);
	  rcu_periph_clock_enable(MOTOR0_EN_GPIO_CLK);
	  rcu_periph_clock_enable(MOTOR1_EN_GPIO_CLK);
	
    /*Configure timer0 PA8(TIMER0_CH0) PA9(TIMER0_CH1) PA10(TIMER0_CH2)as alternate function*/
    //gpio_init(MOTOR0_GPIO_GROUP, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MOTOR0_U_CTR_PIN | MOTOR0_V_CTR_PIN | MOTOR0_W_CTR_PIN);
	  /*Configure timer1 PA1(TIMER1_CH1) PA2(TIMER1_CH2) PA3(TIMER1_CH3)as alternate function*/
	  gpio_init(MOTOR1_GPIO_GROUP, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, MOTOR1_U_CTR_PIN | MOTOR1_V_CTR_PIN | MOTOR1_W_CTR_PIN);
	
	  /*Configure Moto0 U\V\W enable and Moto1 U\V\W enable function*/
	  gpio_init(MOTOR0_EN_GPIO_GROUP, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MOTOR0_EN_CTL_PIN);
	  gpio_init(MOTOR1_EN_GPIO_GROUP, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, MOTOR1_EN_CTL_PIN);
	
	  
}


