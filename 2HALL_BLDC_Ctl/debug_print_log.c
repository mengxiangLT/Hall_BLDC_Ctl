#include "includes.h"


float Record_Buffer[2][MAX_RECORD_BUFF] = {0, 0};

void DebugPrint_log(uint8_t en, uint16_t print_delay, float a, float b)
{
	  static uint16_t count = 0;
	
	  if(en) {
				count++;
				if(count >= print_delay) {
						printf("\r\n a = %lf, b = %lf \r\n", a, b);
						count = 0;
				}
		}
}

void RecordPrintLog(uint8_t en, uint16_t num, uint8_t ctl_flag, float a, float b)
{
	  static uint16_t count = 0;
	  uint16_t i = 0;
	
	  if(en) {
			  if(num > MAX_RECORD_BUFF)
					 num = MAX_RECORD_BUFF;
				if(count < num && ctl_flag == 1) {
					 Record_Buffer[0][count] = a;
					 Record_Buffer[1][count] = b;
					 count++;
				} else if(count > 0 && ctl_flag == 2) {
					 count = 0;
					 ctl_flag = 0;
					 for(i = 0;i < num;i++) {
							printf("\r\n a[%d] = %f, b[%d] = %f \r\n",i, (float)Record_Buffer[0][i], i, (float)Record_Buffer[1][i]);
					 }
				}
		}
}

