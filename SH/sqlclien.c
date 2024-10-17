#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void *recv_handle(void *arg)
{
  char msg[128]={0};
  int fd=*(int*)arg;
  while(1)
  {
    memset(msg,0,128);
    if(recv(fd,msg,128,0)==0)
    {
      break;
    }
    printf("%s\n",msg);
  }
}

int main()
{
  int sockfd=socket(AF_INET,SOCK_STREAM,0);
  struct sockaddr_in server_info;
  int len=sizeof(server_info);
  
  server_info.sin_family=AF_INET;
  server_info.sin_port=htons(1884);
  server_info.sin_addr.s_addr=inet_addr("172.29.19.230");
  
  //Á¬½Ó
  if(connect(sockfd,(struct sockaddr *)&server_info,len)==-1)
  {
    perror("connect");
    return -1;
  }
  pthread_t tid;
  pthread_create(&tid,NULL,recv_handle,&sockfd);
  char msg[1024]={0};
  const char *json="{\"username\":\"0002\",\"password\":\"1112\"}";
  msg[0]=0x60;
  memcpy(msg+1,json,strlen(json));
  
  send(sockfd,msg,strlen(json)+1,0);
  while(1);
  
  
  return 0;
}