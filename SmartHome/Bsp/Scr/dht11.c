#include "dht11.h"

#define   GPIO_PIN_UP     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
#define   GPIO_PIN_LOW    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
#define   GPIO_PIN_DATA   HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)

void Delay_US(uint32_t us)
{	
	uint32_t old_time = SysTick->VAL;     //当前的数值
	uint32_t new_time;
	uint32_t ticks = us * 100;
	uint32_t cnt = 0;
	uint32_t load = SysTick->LOAD;
	
	while (1)
	{
		new_time = SysTick->VAL;
		
		if (new_time != old_time)
		{
			if (new_time < old_time)
				cnt += (old_time - new_time);
			else 
				cnt += (load - new_time + old_time);
			
			if (cnt >= ticks)
				break;
			
			old_time = new_time;
		}
	}
}
	
void DHT11_Start(void)
{
	GPIO_PIN_LOW;
	
	HAL_Delay(20);
	
	GPIO_PIN_UP;
	
	Delay_US(30);
}

uint8_t DHT11_Response(void)
{
	uint16_t time = 0;
	
	while (GPIO_PIN_DATA && time < 100)
	{
		Delay_US(1);
		time++;
	}
	
	if (time >= 100)
		return 1;
	
	time = 0;
	
	while (!GPIO_PIN_DATA && time < 100)
	{
		Delay_US(1);
		time++;
	}
	
	if (time >= 100)
		return 1;
	
	return 0;
}

uint8_t DHT11_Read_Bit(void)
{
	uint16_t time = 0;
	
	while (GPIO_PIN_DATA && time < 100)
	{
		Delay_US(1);
		time++;
	}
	
	if (time >= 100)
		return 2;
	
	while (!GPIO_PIN_DATA&& time < 100)
	{
		Delay_US(1);
		time++;
	}
	
	if (time >= 100)
		return 2;
	
	Delay_US(30);
	
	if (GPIO_PIN_DATA == 0)
		return 0;
	else 
		return 1;
}

uint8_t DHT11_Read_Byte(void)
{
	uint8_t data = 0;
	uint8_t i = 0;
	
	for (i = 0; i < 8; i++)
	{
		data <<= 1;
		data = data | DHT11_Read_Bit();
	}
	
	return data;
}

void DHT11_Read_Data(uint8_t *pData)
{
	DHT11_Start();
	
	if (DHT11_Response())
		return;
	
	uint8_t i;
	for (i = 0; i < 5; i++)
	{
		pData[i] = DHT11_Read_Byte();
	}
	
	if (pData[4] != pData[0] + pData[1] + pData[2] + pData[3])
	{
		for (i = 0; i < 5; i++)
		{
			pData[i] = 0;
		}
	}
}
