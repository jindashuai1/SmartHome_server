#ifndef _ESP8266_H_
#define _ESP8266_H_
#include "stm32f4xx_hal.h"

void ESP8266_Init(void);
static uint8_t ESP8266_Response(const char *msg,uint32_t timeout);
void ESP8266_SetBaud(uint32_t baud);
void ESP8266_Send_Cmd(const char *cmd);
static void ESP8266_Refresh_Buffer(void);
uint8_t ESP8266_Connect_AP(const char *ssid,const char*passwd);
uint8_t ESP8266_Connect_MQTT(const char *ip, int port, const char *user,const char *password);
void ESP8266_MQTTPUB(const char *topic,const char *msg);
uint8_t ESP8266_MQTTSUB(const char *topic);
//void ESP8266_Connect_TCP(const char *ip,int port);
//void ESP8266_Ping(const char *ip);

#endif
