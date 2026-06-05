#ifndef __SENSOR_ANGLE_UPDATE_H__
#define __SENSOR_ANGLE_UPDATE_H__

#include "includes.h"


void Hall_Sensor_Init(Sensor_BLDC_Para *sensor);
void Sensor_Para_Init(Sensor_BLDC_Para *sensor, int Mot_Num);
void Sensor_Angle_SensorInit(Sensor_BLDC_Para *sensor);
void Sensor_BLDC_Update(Sensor_BLDC_Para *sensor);
float Sensor_BLDC_GetAngle(Sensor_BLDC_Para *sensor);
float Sensor_BLDC_GetMechanicalAngle(Sensor_BLDC_Para *sensor);
float Sensor_BLDC_GetElectricityAngle(Sensor_BLDC_Para *sensor);
float Sensor_BLDC_GetVelocity(Sensor_BLDC_Para *sensor);

#endif

