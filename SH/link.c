#include "link.h"
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include <stdio.h>
#include "mqtt.h"

extern tLink *head;

void Init_link()
{
  head=(tLink *)malloc(sizeof(tLink));
  if(NULL==head)
  {
    perror("malloc");
    return;
  }
  head->next=NULL;
}

void add_link(int fd,const char *t)
{
  tLink *p=head;
  while(p->next)
    p=p->next;
    
  tLink *newNode=(tLink *)malloc(sizeof(tLink));
  if(NULL==newNode)
  {
    perror("malloc");
    return;
  }
  newNode->fd=fd;
  strcpy(newNode->topic,t);
  newNode->next=NULL;
  
  p->next=newNode;
}
/**
  * 发送节点中的内容
*/
void send_message(const char *topic,const char *msg,int MsgLen)
{
  tLink *p=head->next;
  while(p)
  {
    if(!strcmp(p->topic,topic))
    {
      message_publish(p->fd,topic,msg,MsgLen);
    }
    p=p->next;
  }
}
/**
  * 删除链表节点
*/
void delete_link(int fd)
{
  tLink *p=head;
  while(p->next)
  {
    if(p->next->fd==fd)
    {
      tLink *TempNode=p->next;
      p->next=TempNode->next;
      free(TempNode);
    }
    p=p->next;
  }
}

