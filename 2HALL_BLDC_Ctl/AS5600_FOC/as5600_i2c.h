#ifndef __AS5600_I2C_H__
#define __AS5600_I2C_H__

#include "gd32f30x.h"
#include "includes.h"

//#define SET_ANGLE_360

//i2c命令的等待超时参数
#define MAX_TIMEOUT		1000
#define MAX_RETRY_CNT	5

/* AS5600 I2C ?? - ?????????? */
#define AS5600_SCL_PORT     GPIOB
#define AS5600_SCL_PIN      GPIO_PIN_6
#define AS5600_SDA_PORT     GPIOB
#define AS5600_SDA_PIN      GPIO_PIN_7

/* AS5600 I2C ?? (7???,????1?) */
#define AS5600_ADDR         0x6C

/* AS5600 ????? */
#define AS5600_RAW_ANGLE_H  0x0C
#define AS5600_RAW_ANGLE_L  0x0D
#define AS5600_ANGLE_H      0x0E
#define AS5600_ANGLE_L      0x0F
#define AS5600_STATUS       0x0B
#define AS5600_AGC          0x1A
#define AS5600_MAGNITUDE_H  0x1B
#define AS5600_MAGNITUDE_L  0x1C

#define _2PI 6.28318530718f



/* AS5600 ???? */
void as5600_i2c_init(void);
uint16_t as5600_read_raw_angle(void);    /* ?????? 0-4095 */
float as5600_read_angle_degree(void);    /* ???? 0-360? */
uint8_t as5600_read_status(void);        /* ???? */
uint8_t as5600_check_magnet(void);       /* ??????? */

/* ???? */
void Sensor_AS5600_Init(Sensor_BLDC_Para *sensor, int Mot_Num);
void Sensor_AS5600_SensorInit(Sensor_BLDC_Para *sensor);
double Sensor_AS5600_GetSensorAngle(Sensor_BLDC_Para *sensor);
void Sensor_AS5600_Update(Sensor_BLDC_Para *sensor);
float Sensor_AS5600_GetMechanicalAngle(Sensor_BLDC_Para *sensor);
float Sensor_AS5600_GetAngle(Sensor_BLDC_Para *sensor);
float Sensor_AS5600_GetVelocity(Sensor_BLDC_Para *sensor);

#endif

