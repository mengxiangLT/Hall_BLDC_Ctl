#include "includes.h"

/* 获取时间*/
extern uint32_t _micros(void);

uint16_t g_retry_cnt = MAX_RETRY_CNT;
uint16_t g_timeout = MAX_TIMEOUT;
/*==================== AS5600 ???? ====================*/

#if 1
/* ?? AS5600 ??? (???) - ?? eeprom_buffer_read ?? */
static uint8_t as5600_read_reg(uint8_t reg)
{
    uint8_t data = 0;
    
    /* ?? I2C ???? */
	  g_timeout = MAX_TIMEOUT;
    while((i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) && g_timeout--);
    
    /* ?? START ?? */
	  g_retry_cnt = MAX_RETRY_CNT;
	  do {
				g_timeout = MAX_TIMEOUT;
				i2c_start_on_bus(I2C0);
				while((!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) && g_timeout--);
				if(g_timeout > 0) {
						g_retry_cnt = 0;
					  break;
				}
		} while(g_retry_cnt--);
    
    /* ??????(???) */
		g_timeout = MAX_TIMEOUT;
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_TRANSMITTER);
    while((!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) && g_timeout--);
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ???????? */
	  g_timeout = MAX_TIMEOUT;
    while((SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)) && g_timeout--);
    
    /* ??????????? */
    i2c_data_transmit(I2C0, reg);
    
    /* ?? BTC ?(????) */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_BTC)) && g_timeout--);
    
    /* ???? START ?? */
		g_timeout = MAX_TIMEOUT;
    i2c_start_on_bus(I2C0);
    while((!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) && g_timeout--);
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_RECEIVER);
    
    /* ?????1???,??????????? STOP */
    i2c_ack_config(I2C0, I2C_ACK_DISABLE);
    
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) && g_timeout--);
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ?? STOP ?? */
    i2c_stop_on_bus(I2C0);
    
    /* ????????? */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_RBNE)) && g_timeout--);
    
    /* ???? */
    data = i2c_data_receive(I2C0);
    
    /* ?? STOP ???? */
		g_timeout = MAX_TIMEOUT;
    while((I2C_CTL0(I2C0) & 0x0200) && g_timeout--);
    
    /* ?????? */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    
    return data;
}

/* ?? AS5600 16???? (2??) - ?? eeprom_buffer_read ?? */
static uint16_t as5600_read_reg16(uint8_t reg_h)
{
    uint8_t high, low;
    uint16_t data;
    
    /* ?? I2C ???? */
	  g_timeout = MAX_TIMEOUT;
    while((i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) && g_timeout--);
    
    /* ??????? NEXT(??2????) */
    i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
    
    /* ?? START ?? */
	  g_retry_cnt = MAX_RETRY_CNT;
	  do {
				g_timeout = MAX_TIMEOUT;
				i2c_start_on_bus(I2C0);
				while((!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) && g_timeout--);
				if(g_timeout > 0) {
						g_retry_cnt = 0;
					  break;
				}
		} while(g_retry_cnt--);
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_TRANSMITTER);
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) && g_timeout--);
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ???????? */
		g_timeout = MAX_TIMEOUT;
    while((SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)) && g_timeout--);
    
    /* ?? I2C */
    i2c_enable(I2C0);
    
    /* ???????????(????) */
    i2c_data_transmit(I2C0, reg_h);
    
    /* ?? BTC ? */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_BTC)) && g_timeout--);
    
    /* ???? START ?? */
    i2c_start_on_bus(I2C0);
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) && g_timeout--);
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_RECEIVER);
    
    /* ??2???,?????? */
    i2c_ack_config(I2C0, I2C_ACK_DISABLE);
    
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) && g_timeout--);
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ?? BTC ? */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_BTC)) && g_timeout--);
    
    /* ?? STOP ?? */
    i2c_stop_on_bus(I2C0);
    
    /* ???8? */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_RBNE)) && g_timeout--);
    high = i2c_data_receive(I2C0);
    
    /* ???8? */
		g_timeout = MAX_TIMEOUT;
    while((!i2c_flag_get(I2C0, I2C_FLAG_RBNE)) && g_timeout--);
    low = i2c_data_receive(I2C0);
    
    /* ?? STOP ???? */
		g_timeout = MAX_TIMEOUT;
    while((I2C_CTL0(I2C0) & 0x0200) && g_timeout--);
    
    /* ????????????? */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
    
    data = ((uint16_t)high << 8) | low;
    return data & 0x0FFF;  /* AS5600 ??12????? */
}
#else
/* ?? AS5600 ??? (???) - ?? eeprom_buffer_read ?? */
static uint8_t as5600_read_reg(uint8_t reg)
{
    uint8_t data = 0;
    
    /* ?? I2C ???? */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    
    /* ?? START ?? */
		i2c_start_on_bus(I2C0);
		while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_TRANSMITTER);
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ???????? */
    while(SET != i2c_flag_get(I2C0, I2C_FLAG_TBE));
    
    /* ??????????? */
    i2c_data_transmit(I2C0, reg);
    
    /* ?? BTC ?(????) */
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    
    /* ???? START ?? */
    i2c_start_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_RECEIVER);
    
    /* ?????1???,??????????? STOP */
    i2c_ack_config(I2C0, I2C_ACK_DISABLE);
    
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ?? STOP ?? */
    i2c_stop_on_bus(I2C0);
    
    /* ????????? */
    while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
    
    /* ???? */
    data = i2c_data_receive(I2C0);
    
    /* ?? STOP ???? */
    while(I2C_CTL0(I2C0) & 0x0200);
    
    /* ?????? */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    
    return data;
}

/* ?? AS5600 16???? (2??) - ?? eeprom_buffer_read ?? */
static uint16_t as5600_read_reg16(uint8_t reg_h)
{
    uint8_t high, low;
    uint16_t data;
    
    /* ?? I2C ???? */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    
    /* ??????? NEXT(??2????) */
    i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
    
    /* ?? START ?? */
		i2c_start_on_bus(I2C0);
		while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_TRANSMITTER);
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ???????? */
    while(SET != i2c_flag_get(I2C0, I2C_FLAG_TBE));
    
    /* ?? I2C */
    i2c_enable(I2C0);
    
    /* ???????????(????) */
    i2c_data_transmit(I2C0, reg_h);
    
    /* ?? BTC ? */
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    
    /* ???? START ?? */
    i2c_start_on_bus(I2C0);
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    
    /* ??????(???) */
    i2c_master_addressing(I2C0, AS5600_ADDR, I2C_RECEIVER);
    
    /* ??2???,?????? */
    i2c_ack_config(I2C0, I2C_ACK_DISABLE);
    
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    
    /* ?? BTC ? */
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    
    /* ?? STOP ?? */
    i2c_stop_on_bus(I2C0);
    
    /* ???8? */
		g_timeout = MAX_TIMEOUT;
    while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
    high = i2c_data_receive(I2C0);
    
    /* ???8? */
    while(!i2c_flag_get(I2C0, I2C_FLAG_RBNE));
    low = i2c_data_receive(I2C0);
    
    /* ?? STOP ???? */
    while(I2C_CTL0(I2C0) & 0x0200);
    
    /* ????????????? */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
    i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
    
    data = ((uint16_t)high << 8) | low;
    return data & 0x0FFF;  /* AS5600 ??12????? */
}
#endif

/*==================== AS5600 ??? ====================*/
void as5600_i2c_init(void)
{
    /* ???? */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C0);
    
    /* ??????????? */
    gpio_init(AS5600_SCL_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, AS5600_SCL_PIN);
    gpio_init(AS5600_SDA_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, AS5600_SDA_PIN);
    
    /* ?? I2C ?? - ???????? */
    i2c_clock_config(I2C0, 800000, I2C_DTCY_2);  /* 100kHz */
    
    /* ?? I2C ?? (??????????,??????) */
    /* ??:????? GD32 ????????,??????????? */
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
    
    /* ?? I2C */
    i2c_enable(I2C0);
    
    /* ???? */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

uint16_t as5600_read_raw_angle(void)
{
#ifdef SET_ANGLE_360
		return as5600_read_reg16(AS5600_ANGLE_H);
#else
    return as5600_read_reg16(AS5600_RAW_ANGLE_H);
#endif

}

float as5600_read_angle_degree(void)
{
	  uint16_t raw = 0;
	
	  raw = as5600_read_raw_angle();
#ifdef SET_ANGLE_360
    return (float)raw * 360.0f / 4096.0f;
#else
	  return (float)raw;
#endif
}

uint8_t as5600_read_status(void)
{
    return as5600_read_reg(AS5600_STATUS);
}

uint8_t as5600_check_magnet(void)
{
    uint8_t status = as5600_read_status();
    return (status & 0x20) ? 1 : 0;  /* MD bit (bit5) */
}



#if 0
/* 磁传感器的精度是4096(12位,0-4095)*/
static uint16_t read_raw_angle(Sensor_BLDC_Para *sensor)
{
    uint16_t buf;
    uint16_t raw_value;
    int bit_resolution = 12;
    int bits_used_msb = 4;  /* 11-7 = 4 */
    float cpr;
    int lsb_used;
    uint8_t lsb_mask, msb_mask;
    
//    (void)sensor;  /* ??????? */
    
    /* ??2???? */
//    if(i2c_read_bytes(AS5600_ADDR, AS5600_RAW_ANGLE_H, buf, 2) != 0) {
//        return 0;
//    }
	  buf = as5600_read_angle_degree();
    
    cpr = 4096.0f;
    lsb_used = bit_resolution - bits_used_msb;
    lsb_mask = (uint8_t)((2 << lsb_used) - 1);
    msb_mask = (uint8_t)((2 << bits_used_msb) - 1);
    
    raw_value = ((buf & 0xFF00) >> 8 & lsb_mask);
    raw_value += (((buf & 0xFF) & msb_mask) << lsb_used);
    
    return raw_value;
}
#endif

/* 将磁传感器的角度值转换为弧度值(0-2PI)*/
double Sensor_AS5600_GetSensorAngle(Sensor_BLDC_Para *sensor)
{
#ifdef SET_ANGLE_360
			return (double)as5600_read_angle_degree();
#else
	  float cpr = 4096.0f;  /* 2^12 */
    uint16_t raw = as5600_read_angle_degree();
    return (raw / cpr) * _2PI;
#endif
}

/* 初始化传感器的参数 */
void Sensor_AS5600_Init(Sensor_BLDC_Para *sensor, int Mot_Num)
{
    sensor->Mot_Num = Mot_Num;
    sensor->angle_prev = 0.0f;
    sensor->angle_prev_ts = 0;
    sensor->vel_angle_prev = 0.0f;
    sensor->vel_angle_prev_ts = 0;
    sensor->full_rotations = 0;
    sensor->vel_full_rotations = 0;
}

/* 初始化传感器参数值 */
void Sensor_AS5600_SensorInit(Sensor_BLDC_Para *sensor)
{
//    Sensor_AS5600_GetSensorAngle(sensor);
    sensor->vel_angle_prev = Sensor_AS5600_GetSensorAngle(sensor);
    sensor->vel_angle_prev_ts = _micros();
    
//    Sensor_AS5600_GetSensorAngle(sensor);
    sensor->angle_prev = Sensor_AS5600_GetSensorAngle(sensor);
    sensor->angle_prev_ts = _micros();
}

/* 根据磁编码器获取的角度值，算出电机转过的圈数 */
void Sensor_AS5600_Update(Sensor_BLDC_Para *sensor)
{
    float val = Sensor_AS5600_GetSensorAngle(sensor);
    float d_angle;
    
    sensor->angle_prev_ts = _micros();
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 0, val, sensor->angle_prev);
#endif
    d_angle = val - sensor->angle_prev;
    
    /* 当角度转过一圈（2PI/360°）的80%时，就认为完成了一圈 */
#ifdef SET_ANGLE_360
		if(fabsf(d_angle) > (0.8f * 360)) { 
#else
	  if(fabsf(d_angle) > (0.8f * _2PI)) {
#endif
        if(d_angle < 0) {
            sensor->full_rotations--;
        } else {
            sensor->full_rotations++;
        }
    }
#ifdef DEBUG_PRINT
		DebugPrint_log(0, 500, sensor->full_rotations, 0);
#endif
    sensor->angle_prev = val;
}

/* 获取当前转过的角度值(0-2PI)*/
float Sensor_AS5600_GetMechanicalAngle(Sensor_BLDC_Para *sensor)
{
    return sensor->angle_prev;
}

/* 将磁传感器检测到的转动圈数转换为角度的弧度值，再加上未转满一圈的部分
 *The number of rotations detected by the magnetic sensor is converted into radians, and the portion that did not complete one rotation is added.
*/
float Sensor_AS5600_GetAngle(Sensor_BLDC_Para *sensor)
{
//	  Sensor_AS5600_Update(sensor);
#ifdef SET_ANGLE_360
	  return (float)-sensor->full_rotations * 360 + sensor->angle_prev;
#else
    return (float)-sensor->full_rotations * _2PI + sensor->angle_prev;
#endif
}

/* 获取转子的速度 */
float Sensor_AS5600_GetVelocity(Sensor_BLDC_Para *sensor)
{
    float Ts;
    float vel;
    
	/* 用当前时间减去之前的时间*10的-6次方，转换为秒*/
    Ts = (sensor->angle_prev_ts - sensor->vel_angle_prev_ts) * 1e-6f;
    
    /* 当时间小于等于零时，就直接赋值0.001s */
    if(Ts <= 0) Ts = 1e-3f;
    
	/* ((本时刻的圈数(full_rotations)-前一时刻的圈数(vel_full_rotations))*一圈对应的弧度(_2PI)+(当前角度-上次循环的角度))/过去的时间(Ts) */
#ifdef SET_ANGLE_360
		vel = ((float)(sensor->full_rotations - sensor->vel_full_rotations) * 360 
           + (-(sensor->angle_prev - sensor->vel_angle_prev))) / Ts;  //这里磁编码器顺时针转动数值是递减的，所以需要加个“-”转换一下
#else
	  vel = ((float)(sensor->full_rotations - sensor->vel_full_rotations) * _2PI 
           + (-(sensor->angle_prev - sensor->vel_angle_prev))) / Ts;  //这里磁编码器顺时针转动数值是递减的，所以需要加个“-”转换一下
#endif
#ifdef DEBUG_PRINT
if(vel < 0) {
		DebugPrint_log(0, 0, sensor->full_rotations, sensor->vel_full_rotations);
		DebugPrint_log(0, 0, sensor->angle_prev, sensor->vel_angle_prev);
		DebugPrint_log(0, 0, sensor->angle_prev_ts, sensor->vel_angle_prev_ts);
}
#endif
		/* 将当前参数赋值给前一时刻的参数，用于下次周期的计算 */
    sensor->vel_angle_prev = sensor->angle_prev;
    sensor->vel_full_rotations = sensor->full_rotations;
    sensor->vel_angle_prev_ts = sensor->angle_prev_ts;
    
    return vel;
}



