#include "control.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "cJSON.h"
#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "link.h"

extern uint8_t gMenu;
extern TimerHandle_t BuzTimerHand;
extern SemaphoreHandle_t semHandle;
extern EnvINfo gInfo;

void Control_Menu(cJSON *obj)
{
	cJSON *tabObject=cJSON_GetObjectItem(obj,"tab");
	if(tabObject)
	{
		if(!strcmp(tabObject->valuestring,"1"))
			gMenu=1;
		else if(!strcmp(tabObject->valuestring,"2"))
			gMenu=2;
		else if(!strcmp(tabObject->valuestring,"3"))
			gMenu=3;
	}
}
void Control_Light(cJSON *obj)
{
	cJSON *idObject=cJSON_GetObjectItem(obj,"which");
	if(!idObject)
	return;
	cJSON *stateObject=cJSON_GetObjectItem(obj,"state");
	if(!stateObject)
	return;
	int id=idObject->valueint;
	int state=stateObject->valueint;

	if(id<1||id>4||state<0||state>1)
	return ;
	switch(id)
	{
		case 1:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(GPIO_PinState)state);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,(GPIO_PinState)state);
			break;
		case 3:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,(GPIO_PinState)state);
			break;
		case 4:
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,(GPIO_PinState)state);
			break;
	}
}



void Buzzer_Start(void)
{
	xTimerStart(BuzTimerHand,portMAX_DELAY);
}
void Buzzer_Stop(void)
{
	xTimerStop(BuzTimerHand,portMAX_DELAY);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
}
void Control_Buzzer(cJSON *obj)
{
	cJSON *statobj=cJSON_GetObjectItem(obj,"state");
	if(!statobj)
	return;
	int state=statobj->valueint;
	if(state==1)
	{
		Buzzer_Start();
	}
	else if(state==0)
	{
		Buzzer_Stop();
	}

}
void Control_timer(cJSON *obj)
{
	cJSON *timeObj=cJSON_GetObjectItem(obj,"time");
	if(!timeObj)
	return;
	uint32_t time=timeObj->valueint;
	xSemaphoreTake(semHandle,portMAX_DELAY);
	gInfo.cur_time=time;
	xSemaphoreGive(semHandle);

}
void Control_Alarm(cJSON *obj)
{
	cJSON *hourObj=cJSON_GetObjectItem(obj,"hour");
	cJSON *minObj=cJSON_GetObjectItem(obj,"min");
	if(!hourObj||!minObj)
	return;
	uint8_t hour=hourObj->valueint;
	uint8_t min=minObj->valueint;
	Add_Alarm_Link(hour,min);
}


void Parse_Message(char *msg)
{
	char *p=msg, *p1,*p2;
	int comma=0,len=0;
	char str[8]={0};
	char content[256]={0};
	while(*p!='\0')
	{
		if(*p==',')
			comma++;
		if(comma==2&&*p==',')
			p1=p+1;
		if(comma==3&&*p==',')
			p2=p+1;
		p++;
	}
	memcpy(str,p1,p2-p1);
	len=atoi(str);
	
	memcpy(content,p2,len);
	cJSON *jsonObject=cJSON_Parse(content);
	if(jsonObject==NULL)
	{
		cJSON_Delete(jsonObject);
		return;
	}
	cJSON *cmdObject=cJSON_GetObjectItem(jsonObject,"cmd");
	if(!strcmp(cmdObject->valuestring,"menu"))
	{
		Control_Menu(jsonObject);
	}
	else if (!strcmp(cmdObject->valuestring,"light"))
	{
		Control_Light(jsonObject);
	}
	else if (!strcmp(cmdObject->valuestring,"buzzer"))
	{
		Control_Buzzer(jsonObject);
	}
	else if (!strcmp(cmdObject->valuestring,"data"))
	{
		Control_timer(jsonObject);
	}
	else if(!strcmp(cmdObject->valuestring,"alarm"))
	{
		Control_Alarm(jsonObject);
	}
	cJSON_Delete(jsonObject);
}
