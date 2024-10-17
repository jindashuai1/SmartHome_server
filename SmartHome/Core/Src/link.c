#include "link.h"
#include "main.h"
#include "FreeRTOS.h"
#include "stdio.h"

AlarmNode *pxAlarmHead=NULL;
extern EnvINfo gInfo;
int8_t minFlag=-1;

void Init_Link(void)
{
	pxAlarmHead = pvPortMalloc(sizeof(AlarmNode));
	if(NULL==pxAlarmHead)
		return;
	
	pxAlarmHead->next=NULL;
}
void Add_Alarm_Link(uint8_t hour, uint8_t min)
{
    AlarmNode *p=pxAlarmHead->next;
	while(p)
	{
		if(p->alarm_type==ALARMTYPE)
		break;
		p=p->next;
	}
	if(p)
	{
		p->hour=hour;
		p->min=min;
		
	}
	else{
		p=pxAlarmHead;
		while(p->next)
			p=p->next;
		AlarmNode *newNode=pvPortMalloc(sizeof(AlarmNode));
		if(newNode==NULL)
			return;
		newNode->hour=hour;
		newNode->min=min;
		newNode->alarm_type=ALARMTYPE;
		newNode->next=NULL;
		p->next=newNode;
	}
}
void Link_Find_Alarm(uint8_t *h,uint8_t *m)
{
	AlarmNode *p=pxAlarmHead->next;
	while(p)
	{
		if(p->alarm_type==ALARMTYPE)
		{
			*h=p->hour;
			*m=p->min;
			return;
		}
		p=p->next;
	}
	if(!p)
	{
		*h=0;
		*m=0;
	}
}
uint8_t Link_Alarm_Isset(void)
{
	uint32_t curTime=gInfo.cur_time+3600*8;
	struct tm*t=localtime(&curTime);
	uint8_t hour=t->tm_hour;
	uint8_t min=t->tm_min;
	if(minFlag==min)
	{
		return 0;
	}
	else
	{
		minFlag=-1;
	}
	AlarmNode *p=pxAlarmHead->next;
	while(p)
	{
		if(p->hour==hour&&p->min==min)
		{
			minFlag=min;
			return 1;
		}
		p=p->next;
	}
	return 0;
}
uint8_t Link_Alarm_timeout(void)
{
	uint32_t curTime=gInfo.cur_time+3600*8;
	struct tm*t=localtime(&curTime);
	uint8_t hour=t->tm_hour;
	uint8_t min=t->tm_min;
	AlarmNode *p=pxAlarmHead->next;
	while(p)
	{
		if(p->hour==hour&&p->min!=min)
		{
			return 1;
		}
		p=p->next;
	}
	return 0;
}
	
