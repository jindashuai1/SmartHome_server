#include "show.h"
#include "oled.h"
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "link.h"


extern SemaphoreHandle_t semHandle;
extern EnvINfo gInfo;
extern uint8_t gMenu;
EnvINfo preInfo={0};
uint8_t curMenu=0;
TimeInfo pretime={0};


/** 
  * @brief 计算长度
*/
uint8_t Get_Number_Len(uint16_t number)
{
	if(number==0)
	return 1;
	int len=0;
	while(number>0)
	{
		len++;
        number/=10;
	}
    return len;
}

void Show_Environment(void)
{
	if(curMenu!=gMenu)
	OLED_Clear();
	uint8_t i,temp,humi;
	uint16_t light;
	xSemaphoreTake(semHandle,portMAX_DELAY);
	temp=gInfo.temp;
	humi=gInfo.humi;
	light=gInfo.light;
	xSemaphoreGive(semHandle);
	for(i=0;i<16;i++)
	{
		OLED_ShowChar(i*8,4,'-',16);
	}
    uint8_t len=Get_Number_Len(light);
	if(light!=preInfo.light)
	{
		for(i=0;i<4;i++)
		{
			OLED_Clearrow(i);
		}
		if(len%2)
		{
			OLED_ShowNum((8-len)/2*16+8,0,light,len,32);
		}
		else
			OLED_ShowNum((8-len)/2*16,0,light,len,32);
	}
		OLED_ShowChinese(0,6,24);
		OLED_ShowNum(18,6,temp,2,16);
		OLED_ShowChinese(34,6,10);
		OLED_ShowChinese(60,6,25);
		OLED_ShowNum(78,6,humi,2,16);
		OLED_ShowString(95,6,(uint8_t*)"%RH",16);
		preInfo.light=light;
}

void Show_timer(void)
{

	if(curMenu!=gMenu)
	{

		OLED_Clear();
		OLED_ShowChinese(16*2,0,11);
		OLED_ShowChinese(16*3,0,12);
		OLED_ShowChinese(16*4,0,13);
		OLED_ShowChinese(16*5,0,14);

		OLED_ShowChinese(80,2,15);
		OLED_ShowChinese(96,2,16);
		
	}
	uint32_t curTime=gInfo.cur_time+3600*8;
	struct tm*t=localtime(&curTime);
	uint16_t year=t->tm_year+1900;
	uint8_t month=t->tm_mon+1;
	uint8_t day=t->tm_mday;
	uint8_t hour=t->tm_hour;
	uint8_t min=t->tm_min;
	uint8_t sec=t->tm_sec;
	uint8_t week=t->tm_wday;

	
	if(year!=pretime.year||gMenu!=curMenu)
	{
		OLED_ShowNum(0,2,year,4,16);
		OLED_ShowChar(32,2,'-',16);
	}
	if(month!=pretime.month||gMenu!=curMenu)
	{
		OLED_ShowNum(40,2,month,Get_Number_Len(month),16);
		OLED_ShowChar(56,2,'-',16);
	}
	if(day!=pretime.day||gMenu!=curMenu)
	{
		OLED_ShowNum(64,2,day,Get_Number_Len(day),16);
	}
	if(week!=pretime.week||gMenu!=curMenu)
	{
		OLED_ShowChinese(112,2,week+16);
	}

	//显示时分秒
	
	if(hour!=pretime.hour||gMenu!=curMenu)
	{
		if(hour<10)
		{
			OLED_ShowNum(0,4,0,1,32);
			OLED_ShowNum(16,4,hour,1,32);
		}
		else
		{
			OLED_ShowNum(0,4,hour,2,32);
		}
		OLED_ShowChar(32,4,':',32);
	}
	
	if(min!=pretime.min||gMenu!=curMenu)
	{
		if(min<10)
		{
			OLED_ShowNum(48,4,0,1,32);
			OLED_ShowNum(64,4,min,1,32);
		}
		else
		{
			OLED_ShowNum(48,4,min,2,32);
		}
	}
	if(curMenu!=gMenu)
	{
		OLED_ShowChar(80,4,':',32);
	}
	if(sec!=pretime.sec||gMenu!=curMenu)
	{
		if(sec<10)
		{
			OLED_ShowNum(96,4,0,1,32);
			OLED_ShowNum(112,4,sec,1,32);
		}
		else
		{
			OLED_ShowNum(96,4,sec,2,32);
		}
	}
	pretime.year=year;
	pretime.month=month;
	pretime.day=day;
	pretime.hour=hour;
	pretime.min=min;
	pretime.sec=sec;
	pretime.week=week;
}

void Show_Alarm(void)
{
	uint8_t hour,min;
	Link_Find_Alarm(&hour,&min);
	if(gMenu!=curMenu)
	{
		OLED_Clear();
		OLED_ShowChinese(16*3,0,26);
		OLED_ShowChinese(16*4+8,0,27);
	}
	if(hour!=pretime.hour||gMenu!=curMenu)
	{
		if(hour<10)
		{
			OLED_ShowNum(16,3,0,1,40);
			OLED_ShowNum(36,3,hour,1,40);
		}
		else
		{
			OLED_ShowNum(16,3,hour,2,40);
		}
		OLED_ShowChar(56,3,':',32);
	}
	

	if(min!=pretime.min||gMenu!=curMenu)
	{
		if(min<10)
		{
			OLED_ShowNum(76,3,0,1,40);
			OLED_ShowNum(96,3,min,1,40);

		}
		else
		{
			OLED_ShowNum(76,3,min,2,40);
		}
		
	}
	pretime.hour=hour;
	pretime.min=min;
}	
/** 
  * @brief 显示页面函数
*/
void OLED_ShowMenu(void)
{
	OLED_Clear();
	uint16_t light;
    while(1)
    {
		xSemaphoreTake(semHandle,portMAX_DELAY);
		light=gInfo.light;
		xSemaphoreGive(semHandle);
		OLED_Light_Intcon(light);
		if(gMenu==1)
        {
			Show_timer();
			curMenu=gMenu;
		}
		else if(gMenu==2)
		{
			Show_Environment();
			curMenu=gMenu;
		}
		else if(gMenu==3)
		{
			Show_Alarm();
			curMenu=gMenu;
		}
		osDelay(500);
    }
	
}


