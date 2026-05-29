#include <Arduino.h> 
#include "AS5600.h"
#include "lowpass_filter.h"
#include "pid.h"

#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
float voltage_power_supply;
float Ualpha,Ubeta=0,Ua=0,Ub=0,Uc=0;
#define _3PI_2 4.71238898038f
float zero_electric_angle=0;
int PP=1,DIR=1;
int pwmA = 32;
int pwmB = 33;
int pwmC = 25;

//???????
LowPassFilter M0_Vel_Flt = LowPassFilter(0.01); // Tf = 10ms   //M0???
//PID
PIDController vel_loop_M0 = PIDController{.P = 2, .I = 0, .D = 0, .ramp = 100000, .limit = voltage_power_supply/2};
PIDController angle_loop_M0 = PIDController{.P = 2, .I = 0, .D = 0, .ramp = 100000, .limit = 100};

//AS5600
Sensor_AS5600 S0=Sensor_AS5600(0);
TwoWire S0_I2C = TwoWire(0);

//=================PID ????=================
//??PID
void DFOC_M0_SET_VEL_PID(float P,float I,float D,float ramp)   //M0???PID??
{
  vel_loop_M0.P=P;
  vel_loop_M0.I=I;
  vel_loop_M0.D=D;
  vel_loop_M0.output_ramp=ramp;
}
//??PID
void DFOC_M0_SET_ANGLE_PID(float P,float I,float D,float ramp)   //M0???PID??
{
  angle_loop_M0.P=P;
  angle_loop_M0.I=I;
  angle_loop_M0.D=D;
  angle_loop_M0.output_ramp=ramp;
}


//M0??PID??
float DFOC_M0_VEL_PID(float error)   //M0???
{
   return vel_loop_M0(error);
   
}
//M0??PID??
float DFOC_M0_ANGLE_PID(float error)
{
  return angle_loop_M0(error);
}


//?????????
#define _constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
//????????????,???????????
//????,???????? _constrain,?????? amt?low ? high,?????????????????????????????????,?? amt ???? low ??? high,???????????,???????
//????,?? amt ?? low,??? low;?? amt ?? high,??? high;???? amt???,_constrain(amt, low, high) ??? amt ??? [low, high] ?????1


// ?????? [0,2PI]
float _normalizeAngle(float angle){
  float a = fmod(angle, 2*PI);   //???????????,??????????
  return a >= 0 ? a : (a + 2*PI);  
  //????????:condition ? expr1 : expr2 
  //??,condition ??????????,??????,??? expr1 ??,???? expr2 ????????????? if-else ????????
  //fmod ????????????????,? angle ??????,??????? _2PI ??????????,?? angle ???? 0 ? _2PI ?????,? fmod(angle, _2PI) ????????
  //??,? angle ??? -PI/2,_2PI ??? 2PI ?,fmod(angle, _2PI) ??????????????,???????????? _2PI ???????? [0, 2PI] ????,?????????????
}


// ??PWM??????
void setPwm(float Ua, float Ub, float Uc) {
  // ????
  Ua = _constrain(Ua, 0.0f, voltage_power_supply);
  Ub = _constrain(Ub, 0.0f, voltage_power_supply);
  Uc = _constrain(Uc, 0.0f, voltage_power_supply);
  // ?????
  // ??????0?1
  float dc_a = _constrain(Ua / voltage_power_supply, 0.0f , 1.0f );
  float dc_b = _constrain(Ub / voltage_power_supply, 0.0f , 1.0f );
  float dc_c = _constrain(Uc / voltage_power_supply, 0.0f , 1.0f );

  //??PWM?PWM 0 1 2 ??
  ledcWrite(0, dc_a*255);
  ledcWrite(1, dc_b*255);
  ledcWrite(2, dc_c*255);
}

void setTorque(float Uq,float angle_el) {
  S0.Sensor_update(); //???????
  Uq=_constrain(Uq,-(voltage_power_supply)/2,(voltage_power_supply)/2);
  float Ud=0;
  angle_el = _normalizeAngle(angle_el);
  // ?????
  Ualpha =  -Uq*sin(angle_el); 
  Ubeta =   Uq*cos(angle_el); 

  // ??????
  Ua = Ualpha + voltage_power_supply/2;
  Ub = (sqrt(3)*Ubeta-Ualpha)/2 + voltage_power_supply/2;
  Uc = (-Ualpha-sqrt(3)*Ubeta)/2 + voltage_power_supply/2;
  setPwm(Ua,Ub,Uc);
}

void DFOC_Vbus(float power_supply)
{
  voltage_power_supply=power_supply;
  pinMode(pwmA, OUTPUT);
  pinMode(pwmB, OUTPUT);
  pinMode(pwmC, OUTPUT);
  ledcSetup(0, 30000, 8);  //pwm??, ??, ??
  ledcSetup(1, 30000, 8);  //pwm??, ??, ??
  ledcSetup(2, 30000, 8);  //pwm??, ??, ??
  ledcAttachPin(pwmA, 0);
  ledcAttachPin(pwmB, 1);
  ledcAttachPin(pwmC, 2);
  Serial.println("??PWM?????");

  //AS5600
  S0_I2C.begin(19,18, 400000UL);
  S0.Sensor_init(&S0_I2C);   //??????0
  Serial.println("???????");

  //PID ??
  vel_loop_M0 = PIDController{.P = 2, .I = 0, .D = 0, .ramp = 100000, .limit = voltage_power_supply/2};
 }


float _electricalAngle(){
  return  _normalizeAngle((float)(DIR *  PP) * S0.getMechanicalAngle()-zero_electric_angle);
}


void DFOC_alignSensor(int _PP,int _DIR)
{ 
  PP=_PP;
  DIR=_DIR;
  setTorque(3, _3PI_2);  //??
  delay(1000);
  S0.Sensor_update();  //????,?????????
  zero_electric_angle=_electricalAngle();
  setTorque(0, _3PI_2);  //??(????)
  Serial.print("0???:");Serial.println(zero_electric_angle);
}

float DFOC_M0_Angle()
{
  return DIR*S0.getAngle();
}

//???
//float DFOC_M0_Velocity()
//{
//  return DIR*S0.getVelocity();
//}

//???
float DFOC_M0_Velocity()
{
  //?????????
  float vel_M0_ori=S0.getVelocity();
  float vel_M0_flit=M0_Vel_Flt(DIR*vel_M0_ori);
  return vel_M0_flit;   //????
}

//==============????==============
float motor_target;
int commaPosition;
String serialReceiveUserCommand() {
  
  // a string to hold incoming data
  static String received_chars;
  
  String command = "";

  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the string buffer:
    received_chars += inChar;

    // end of user input
    if (inChar == '\n') {
      
      // execute the user command
      command = received_chars;

      commaPosition = command.indexOf('\n');//?????????
      if(commaPosition != -1)//????????????
      {
          motor_target = command.substring(0,commaPosition).toDouble();            //????
          Serial.println(motor_target);
      }
      // reset the command buffer 
      received_chars = "";
    }
  }
  return command;
}


float serial_motor_target()
{
  return motor_target;
}



//================??????================
void DFOC_M0_set_Velocity_Angle(float Target)
{
 setTorque(DFOC_M0_VEL_PID(DFOC_M0_ANGLE_PID((Target-DFOC_M0_Angle())*180/PI)),_electricalAngle());   //????
}

void DFOC_M0_setVelocity(float Target)
{
  setTorque(DFOC_M0_VEL_PID((serial_motor_target()-DFOC_M0_Velocity())*180/PI),_electricalAngle());   //????
}

void DFOC_M0_set_Force_Angle(float Target)   //??
{
  setTorque(DFOC_M0_ANGLE_PID((Target-DFOC_M0_Angle())*180/PI),_electricalAngle());
}

void DFOC_M0_setTorque(float Target)
{
  setTorque(Target,_electricalAngle());
}
