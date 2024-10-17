#ifndef _LINK_H
#define _LINK_H

typedef struct TopicLink
{
  int fd;
  char topic[16];
  struct TopicLink *next;
}tLink;

void Init_link();
void add_link(int fd,const char *t);
void send_message(const char *topic,const char *msg,int MsgLen);

void delete_link(int fd);  //É¾³ýÁ´±í½Úµã
#endif
