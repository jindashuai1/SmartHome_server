#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "main.h"
#include "epoll.h"
#include "link.h"
#include <sqlite3.h>
#include "mysqlite3.h"


tLink *head=NULL;
sqlite3 *ppDb=NULL;

int main()
{
	int ret;
  Init_link();
  
  Init_sqlite3();
	//创建socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		exit(1);
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//绑定信息
	struct sockaddr_in server_info;
	
	memset(&server_info, 0, sizeof(server_info));
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(PORT);
	server_info.sin_addr.s_addr = inet_addr(IP);

	ret = bind(sockfd, (struct sockaddr *)&server_info, sizeof(server_info));
	if (-1 == ret)
	{
		perror("bind");
		exit(2);
	}

	//设置监听队列
	ret = listen(sockfd, 10);
	if (-1 == ret)
	{
		perror("listen");
		exit(3);
	}

  
	printf("等待客户端的连接....\n");
  epoll_handle(sockfd);
  struct sockaddr_in client_info;
	 /*
   int len=sizeof(client_info);
   int fd=accept(sockfd,(struct sockaddr *)&client_info,&len);
   if(-1==fd)
   {
     perror("accept");
     exit(4);
     }
     printf("客户端已连接：%d\n",fd);*/
 

	return 0;
}

