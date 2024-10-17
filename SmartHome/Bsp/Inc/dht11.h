#ifndef _DHT11_H_
#define _DHT11_H_

#include "stm32f4xx_hal.h"

void DHT11_Start(void);
void DHT11_Read_Data(uint8_t *pData);
#endif
