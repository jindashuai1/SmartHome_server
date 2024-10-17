#include "Upload.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#include "cJSON.h"
#include "esp8266.h"

extern SemaphoreHandle_t semHandle;
extern EnvINfo gInfo;

void UploadHandle(void)
{
	xSemaphoreTake(semHandle,portMAX_DELAY);
	uint16_t light=gInfo.light;
	uint8_t temp=gInfo.temp;
	uint8_t humi=gInfo.humi;
	xSemaphoreGive(semHandle);
	
	

	//create json object
	cJSON *sendObject=cJSON_CreateObject();
	
	cJSON_AddNumberToObject(sendObject,"light",light);
	cJSON_AddNumberToObject(sendObject,"temp",temp);
	cJSON_AddNumberToObject(sendObject,"humi",humi);

	
	
	//object to string
	char *sendstr=cJSON_Print(sendObject);
	
	char *p=sendstr;
	char output[256]={0};
	int i=0;
	while(*p!='\0')
	{
		if(*p=='"'||*p==',')
		{
			output[i++]='\\';
			output[i++]=*p;
		}
		else
			output[i++]=*p;
		p++;
	}
	
	ESP8266_MQTTPUB("0001Upload",output);
	
	cJSON_Delete(sendObject);
	vPortFree(sendstr);
}
