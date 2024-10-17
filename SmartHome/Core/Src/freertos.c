/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266.h"
#include "oled.h"
#include "stdio.h"
#include "string.h"
#include "dht11.h"
#include "semphr.h"
#include "bh1750.h"
#include "show.h"
#include "timers.h"
#include "Upload.h"
#include "control.h"
#include "queue.h"
#include "link.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

EnvINfo gInfo={.cur_time=1728620218,}; //初始化温湿度，亮度，时间结构体
xQueueHandle uartHanldle;
SemaphoreHandle_t semHandle;  //创建信号量
TimerHandle_t dataTimerHand;
TimerHandle_t BuzTimerHand;

uint8_t gMenu=1; 
uint8_t gBuzzer=0;
uint8_t gUartBuf[256];

//创建接收服务器任务
osThreadId_t ReceiveTaskHandle;
const osThreadAttr_t ReceiveTask_attributes = {
  .name = "ReceiveTask",
  .stack_size = 128 * 4*4,
  .priority = (osPriority_t) osPriorityNormal,
};
//创建温湿度获取任务
osThreadId_t TempTaskHandle;
const osThreadAttr_t TempTask_attributes = {
  .name = "TempTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t LightTaskHandle;
const osThreadAttr_t LightTask_attributes = {
  .name = "LightTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
  .name = "OLDETask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};

osThreadId_t UploadTaskHandle;
const osThreadAttr_t UploadTask_attributes = {
  .name = "UploadTask",
  .stack_size = 128 * 4*4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void ReceiveTask(void *arg);
void GetTempTask(void *arg);
void GetLightTask(void *arg);
void OLED_ShowTask(void *arg);
uint8_t Key_Scan(void);
void Data_Increase_Callback(TimerHandle_t xTimer);
void Upload_Task(void *arg);
void Buzzer_Callback(TimerHandle_t xTimer);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  semHandle=xSemaphoreCreateBinary();
  xSemaphoreGive(semHandle);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  int timeid=1;
  dataTimerHand=xTimerCreate("data",1000,pdTRUE,&timeid,Data_Increase_Callback);
  int timeid2=2;
  BuzTimerHand=xTimerCreate("Buzzer",1,pdTRUE,&timeid2,Buzzer_Callback);
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  uartHanldle=xQueueCreate(5,256);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	OLED_Init();
	OLED_Clear();
	OLED_ShowString(0,0,(uint8_t*)" START WORKING",16);
	ESP8266_Init();
	if(ESP8266_Connect_AP(SSID,PASSWORD))
		OLED_ShowString(0,2,(uint8_t*)" WIFI 0K",16);
	else
		OLED_ShowString(0,2,(uint8_t*)" WIFI ERR",16);
	if(ESP8266_Connect_MQTT(IP, PORT, "test","test"))
		OLED_ShowString(0,4,(uint8_t*)" MQTT 0K",16);
	else
		OLED_ShowString(0,4,(uint8_t*)" MQTT ERR",16);
  if(ESP8266_MQTTSUB(DEVICEID))
    OLED_ShowString(0,6,(uint8_t*)"MQTT SUB OK",16);
  else
    OLED_ShowString(0,6,(uint8_t*)"MQTT SUB ERR",16);
  osDelay(1000);

  //启动timer
  xTimerStart(dataTimerHand,portMAX_DELAY);


  HAL_UARTEx_ReceiveToIdle_IT(&huart2,gUartBuf,sizeof(gUartBuf));
  //创建任务
  ReceiveTaskHandle=osThreadNew(ReceiveTask,NULL,&ReceiveTask_attributes);
  TempTaskHandle=osThreadNew(GetTempTask,NULL,&TempTask_attributes);
  LightTaskHandle=osThreadNew(GetLightTask,NULL,&LightTask_attributes);
  OLEDTaskHandle=osThreadNew(OLED_ShowTask,NULL,&OLEDTask_attributes);
  UploadTaskHandle=osThreadNew(Upload_Task,NULL,&UploadTask_attributes);
  /* Infinite loop */
  for(;;)
  {
    if(Key_Scan()==1)
    {
      if(gBuzzer==1)
      {
        Buzzer_Stop();
        gBuzzer=0;
      }
      else
      {
        gMenu++;
        if(gMenu==4)
          gMenu=1;
      }
    }
    if(gBuzzer==0&&Link_Alarm_Isset())
      {
        Buzzer_Start();
        gBuzzer=1;
      }
    else if(gBuzzer==1&&Link_Alarm_timeout())
    {
      Buzzer_Stop();
      gBuzzer=0;
    }
      
    

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void ReceiveTask(void *arg)
{
   
  uint8_t msg[256]={0};
  while(1)
  {
    xQueueReceive(uartHanldle,msg,portMAX_DELAY);
    if(strlen((char*)msg)>10)
		{
			Parse_Message((char*)msg);
		}
      memset(msg,0,sizeof(msg));
  }
}
void GetTempTask(void *arg)
{
  uint8_t data[5] ={0};
//  OLED_Clear();
  while(1)
  {
    vTaskSuspendAll();  //关闭调度器
    DHT11_Read_Data(data);
    xTaskResumeAll();   //开启调度器
    xSemaphoreTake(semHandle,portMAX_DELAY);
    gInfo.temp=data[2];
    gInfo.humi=data[0];
    xSemaphoreGive(semHandle);
    osDelay(500);
  }
}
void GetLightTask(void *arg)
{
  uint8_t pdata[2]={0};
  uint16_t light=0;
  while(1)
  {
    BH1750_Send_CMD(0x20);
    osDelay(150);
    BH1750_Read_Data(pdata);
    light=BH1750_Data_To_Lx(pdata);
    xSemaphoreTake(semHandle,portMAX_DELAY);
    gInfo.light=light;
    xSemaphoreGive(semHandle);
    osDelay(350);
  }
}
/** 
  * @brief 显示OLED
  * @param *arg
  * @return None
*/
void OLED_ShowTask(void *arg)
{
  OLED_ShowMenu();
}

/** 
  * @brief 按键翻转页面函数
*/
uint8_t Key_Scan(void)
{
  if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)
  {
    while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0);
    return 1;
  }
  return 0;
}

/** 
  * @brief timer task  超时时间为1s
*/
void Data_Increase_Callback(TimerHandle_t xTimer)
{
  
  xSemaphoreTake(semHandle,portMAX_DELAY);
  HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
  gInfo.cur_time++;
  xSemaphoreGive(semHandle);
}
/** 
  * @brief 定时器蜂鸣器
*/
void Buzzer_Callback(TimerHandle_t xTimer)
{
  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_8);
}
/** 
  * @brief 上传数据
*/
void Upload_Task(void *arg)
{
	while(1)
	{
		UploadHandle();
		osDelay(1000);
	}
}
/* USER CODE END Application */

