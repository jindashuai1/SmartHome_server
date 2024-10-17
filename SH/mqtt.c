#include "mqtt.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "link.h"
#include <stdlib.h>
#include <json-c/json.h>
#include "mysqlite3.h"

void mqtt_connect(int fd,const char *packet)
{
  unsigned char ReplyData[64]={0};
  printf("CONNECT\n");
  
  //printf("剩余长度 %d\n",packet[1]);
  //printf("协议名字 %s\n",&packet[4]);
  ReplyData[0]=CONNACK;
  ReplyData[1]=0x02;
  ReplyData[2]=0x00;
  ReplyData[3]=0x00;
  
  if(send(fd,ReplyData,4,0)==-1)
  {
    perror("send\n");
  }
}
void mqtt_subscribe(int fd,const char *packet)
{
  printf("SUBSCRIBE");
  char IdenMSB=packet[2];
  char IdenLSB=packet[3];
  char topic[32]={0};
  char ReplyData[16]={0};
  //printf("报文标识符MSB %x LSB %x \n",IdenMSB,IdenLSB);
  
  short TopicLen=((short)packet[4]<<8)+packet[5]; //订阅主题的长度
  memcpy(topic,&packet[6],TopicLen);  //主题
  
  printf("主题长度%d,主题 %s\n",TopicLen,topic);
  
  ReplyData[0]=SUBACK;
  ReplyData[1]=0x03;
  ReplyData[2]=IdenMSB;
  ReplyData[3]=IdenLSB;
  ReplyData[4]=0x00;
  
  if(send(fd,ReplyData,5,0)==-1)
  {
    perror("send");
    
  }
   
  add_link(fd,topic);
  
}
void mqtt_publish(int fd, const char *packet)
{
  printf("PUBLISH");
  int remain=packet[1];  //总长度
  int TopicLen=((short)packet[2]<<8)+packet[3];//主题长度
  char topic[32]={0};
  char msg[256]={0};
  
  memcpy(topic,&packet[4],TopicLen);
 
  
  int MsgLen=remain-2-TopicLen; //内容长度
  memcpy(msg,&packet[8],MsgLen);
  printf("主题名%s 内容 %s\n",topic,msg);
  
  send_message(topic,msg,MsgLen);
  
}
//发布的内容通过链表传给订阅内容的客户端
void message_publish(int fd,const char *topic,const char *msg,int MsgLen)
{
  char ReplyData[256]={0};
  int index=0;
  int TopicLen=strlen(topic);
  
  ReplyData[index++]=PUBLISH;
  ReplyData[index++]=3+TopicLen+MsgLen;
  ReplyData[index++]=0;
  ReplyData[index++]=TopicLen;
  memcpy(&ReplyData[index],topic,TopicLen);
  index+=TopicLen;
  ReplyData[index++]=0x00;
  memcpy(&ReplyData[index],msg,MsgLen);
  
  if(send(fd,ReplyData,5+TopicLen+MsgLen,0)==-1)
  {
    perror("send");
  }
  
}
/**
  *  用户注册账号
  */
void mqtt_register(int fd,const char *packet)
{
  struct json_object *obj=json_tokener_parse(packet+1);
  if(NULL==obj)
  {
    printf("[ERROR] Not A Json Object\n");
    return;
  }
  struct json_object *value;
  json_object_object_get_ex(obj,"username",&value);
  const char *username=json_object_get_string(value);
  json_object_object_get_ex(obj,"password",&value);
  const char *password=json_object_get_string(value);
  
  //用户存在，添加用户失败
  if(sqlite3_user_exist(username))
  {
    struct json_object *ReplyObject=json_object_new_object();
    json_object_object_add(ReplyObject,"cmd",json_object_new_string("register"));
    json_object_object_add(ReplyObject,"result",json_object_new_string("exist"));
    const char *SendString=json_object_to_json_string(ReplyObject);
    if(send(fd,SendString,strlen(SendString),0)==-1)//发送
    {
      perror("send");
    }
    json_object_put(ReplyObject);
  }
  else
  {
    //用户不存在，添加用户成功
    database_insert_user(username,password);
    struct json_object *ReplyObject=json_object_new_object();
    json_object_object_add(ReplyObject,"cmd",json_object_new_string("register"));
    json_object_object_add(ReplyObject,"result",json_object_new_string("SUCCESS"));
    const char *SendString=json_object_to_json_string(ReplyObject);
    if(send(fd,SendString,strlen(SendString),0)==-1)//发送
    {
      perror("send");
    }
    json_object_put(ReplyObject);
  }
  json_object_put(obj);
}
/**
  *  用户登录账号
  */
void mqtt_login(int fd,const char *packet)
{
  struct json_object *obj=json_tokener_parse(packet+1);
  if(NULL==obj)
  {
    printf("[ERROR] Not A Json Object\n");
    return;
  }
  struct json_object *value;
  json_object_object_get_ex(obj,"username",&value);
  const char *username=json_object_get_string(value);
  json_object_object_get_ex(obj,"password",&value);
  const char *password=json_object_get_string(value);
  
  if(!sqlite3_user_exist(username))
  {
    //用户不存在，登录失败
    struct json_object *ReplyObject=json_object_new_object();
    json_object_object_add(ReplyObject,"cmd",json_object_new_string("login"));
    json_object_object_add(ReplyObject,"result",json_object_new_string("notexist"));
    const char *SendString=json_object_to_json_string(ReplyObject);
    if(send(fd,SendString,strlen(SendString),0)==-1)//发送
    {
      perror("send");
    }
    json_object_put(ReplyObject);
    json_object_put(obj);
    return;
  }
  if(!sqlite3_MatPassword(username,password))
  {
    //密码错误
    struct json_object *ReplyObject=json_object_new_object();
    json_object_object_add(ReplyObject,"cmd",json_object_new_string("login"));
    json_object_object_add(ReplyObject,"result",json_object_new_string("Fail"));
    const char *SendString=json_object_to_json_string(ReplyObject);
    if(send(fd,SendString,strlen(SendString),0)==-1)//发送
    {
      perror("send");
    }
    json_object_put(ReplyObject);
  }
  else
  {
    //密码正确
    struct json_object *ReplyObject=json_object_new_object();
    json_object_object_add(ReplyObject,"cmd",json_object_new_string("login"));
    json_object_object_add(ReplyObject,"result",json_object_new_string("SUCCESS"));
    const char *SendString=json_object_to_json_string(ReplyObject);
    if(send(fd,SendString,strlen(SendString),0)==-1)//发送
    {
      perror("send");
    }
    json_object_put(ReplyObject);
  }
  
  
  json_object_put(obj);
}



