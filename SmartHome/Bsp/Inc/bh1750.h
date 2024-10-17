#ifndef __BH1750_H_
#define __BH1750_H_

#include "stm32f4xx_hal.h"


#define BH1750_WRITE_ADDR   0x46
#define BH1750_READ_ADDR    0x47

void BH1750_Send_CMD(uint8_t cmd);
void BH1750_Read_Data(uint8_t *pData);
uint16_t BH1750_Data_To_Lx(uint8_t *pData);

#endif
