#include "esp8266.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart2;
uint8_t g_UART_Buf[512]={0};
uint16_t g_Index;

void ESP8266_Init(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_Delay(20);
	ESP8266_Response("csum 0xde\r\n", 500);
	
	ESP8266_SetBaud(115200);
	
	ESP8266_Response("ready\r\n", 500);
	
	ESP8266_Send_Cmd("ATE0");
	ESP8266_Refresh_Buffer();
}
//等待接收字符存入缓冲区
static uint8_t ESP8266_Response(const char *msg,uint32_t timeout)
{
	uint32_t Cur_Tick=HAL_GetTick();
	uint8_t data,i=0;
	while(1)
	{
		if(HAL_GetTick()-Cur_Tick>=timeout)
			return 0;
		if(HAL_UART_Receive(&huart2,&data,1,10)==HAL_TIMEOUT)
			continue;
		g_UART_Buf[g_Index++]=data;
		if(data==msg[i])
		{
			i++;
			if(i==strlen(msg))
				return 1;
		}
		else{
			i=0;
		}
	}
}
//转换波特率
void ESP8266_SetBaud(uint32_t baud)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = baud;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	
	HAL_UART_Init(&huart2);
}
//发送缓冲区里的数据,清空缓冲区
static void ESP8266_Refresh_Buffer(void)
{
	printf("%s\r\n", g_UART_Buf);
	
	g_Index = 0;
	memset(g_UART_Buf, 0, sizeof(g_UART_Buf));
}
//给ESP8266发送指令
void ESP8266_Send_Cmd(const char *cmd)
{
	HAL_UART_Transmit(&huart2,(uint8_t*)cmd,strlen(cmd),HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2,(uint8_t*)"\r\n",2,HAL_MAX_DELAY);

}
//连接AP
uint8_t ESP8266_Connect_AP(const char *ssid,const char*passwd)
{
	char cmd[128]={0};
	sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"", ssid, passwd);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",10000))
	{
		ESP8266_Refresh_Buffer();
		return 1;
	}
	else
	{
		ESP8266_Refresh_Buffer();
		printf("CONNECT WIFI FAILURE\r\n");
		return 0;
	}
}
//连接服务器
uint8_t ESP8266_Connect_MQTT(const char *ip, int port, const char *user,const char *password)
{
	char cmd[128]={0};
	sprintf(cmd,"AT+MQTTUSERCFG=0,1,\"0001\",\"%s\",\"%s\",0,0,\" \"",user,password);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",5000))
	{
		ESP8266_Refresh_Buffer();
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONFIG MQTT FALLURE\r\n");
	}
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"AT+MQTTCONN=0,\"%s\",%d,1",ip,port);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK\r\n",10000))
	{
		ESP8266_Refresh_Buffer();
		return 1;
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONNECT MQTT FALLURE\r\n");
		return 0;
	}
}
//发布消息
void ESP8266_MQTTPUB(const char *topic,const char *msg)
{
	char cmd[256]={0};
	sprintf(cmd,"AT+MQTTPUB=0,\"%s\",\"%s\",0,0",topic,msg);
	ESP8266_Send_Cmd(cmd);
	// if(ESP8266_Response("OK\r\n",5000))
	// {
	// 	ESP8266_Refresh_Buffer();
	// }
	// else{
	// 	ESP8266_Refresh_Buffer();
	// 	printf("MQTTPUB FALLURE\r\n");
	// }

}
//订阅消息
uint8_t ESP8266_MQTTSUB(const char *topic)
{
	char cmd[128]={0};
	sprintf(cmd,"AT+MQTTSUB=0,\"%s\",0",topic);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK\r\n",1000))
	{
		ESP8266_Refresh_Buffer();
		return 1;
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("MQTTSUB FALLURE\r\n");
		return 0;
	}

}
#if 0
//连接TCP
void ESP8266_Connect_TCP(const char *ip,int port)
{
	char cmd[128]={0};
	sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%d",ip,port);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",1000))
	{
		ESP8266_Refresh_Buffer();
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONFIG TCP FALLURE\r\n");
	}
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"AT+CIPSEND=4");
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",1000))
	{
		ESP8266_Refresh_Buffer();
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONNECT SEND FALLURE\r\n");
	}
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"ABCD");
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",1000))
	{
		ESP8266_Refresh_Buffer();
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONNECT ABCD FALLURE\r\n");
	}
	
}
//获取IP地址
void ESP8266_GetIp()
{
	char cmd[128]={0};
	sprintf(cmd,"AT+CIFSR");
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",1000))
	{
		ESP8266_Refresh_Buffer();
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("CONNECT CIFSR FALLURE\r\n");
	}
}
//ping主机
void ESP8266_Ping(const char *ip)
{
	char cmd[128]={0};
	sprintf(cmd,"AT+PING=\"%s\"",ip);
	ESP8266_Send_Cmd(cmd);
	if(ESP8266_Response("OK",10000))
	{
		ESP8266_Refresh_Buffer();
	}
	else if(ESP8266_Response("ERROR",10000))
	{
		ESP8266_Refresh_Buffer();
		printf("ERROR\r\n");
	}
	else{
		ESP8266_Refresh_Buffer();
		printf("PING FALLURE\r\n");
	}
}
#endif
