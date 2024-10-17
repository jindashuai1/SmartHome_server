#include<sqlite3.h>
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern sqlite3 *ppDb;

void Init_sqlite3()
{
  if(sqlite3_open("userinfo.db",&ppDb)!=SQLITE_OK)
  {
    printf("sqlite3_open error\%s\n",sqlite3_errmsg(ppDb));
    exit(1);
  }
  char sql[128]={0};
  sprintf(sql,"create table if not exists user (username text,password text)");
  if(sqlite3_exec(ppDb,sql,NULL,NULL,NULL)!=SQLITE_OK)
  {
    printf("sqlite3_exec error %s\n",sqlite3_errmsg(ppDb));
    return;
  }
}

//匹配用户名
int sqlite3_user_exist(const char *name)
{
  int row,column,flag=0;
  const char *sql="select username from user;";
  char **result;
  char *errmsg;
  if(sqlite3_get_table(ppDb,sql,&result,&row,&column,&errmsg)!=SQLITE_OK)
  {
    printf("sqlite3_get_table error %s\n",errmsg);
    return 1;
  }
  for(int i=0;i<row;i++)
  {
    if(!strcmp(result[i+column],name))//相等等于零
    flag=1;
  }
  sqlite3_free_table(result);
  return flag;
}

//客户端注册用户
void database_insert_user(const char *u,const char *p)
{
  char sql[128]={0};
  sprintf(sql,"insert into user values('%s','%s');",u,p);
  if(sqlite3_exec(ppDb,sql,NULL,NULL,NULL)!=SQLITE_OK)
  {
    printf("sqlite3_exec error %s\n",sqlite3_errmsg(ppDb));
  }
}
//匹配用户密码
int sqlite3_MatPassword(const char *u,const char *p)
{
  int row,column,flag=0;
  char sql[128]={0};
  char **result;
  char *errmsg;
  sprintf(sql,"select password from user where username='%s';",u);
  if(sqlite3_get_table(ppDb,sql,&result,&row,&column,&errmsg)!=SQLITE_OK)
  {
    printf("sqlite3_get_table error %s\n",errmsg);
    return 1;
  }
    if(!strcmp(result[1],p))//相等等于零
    {
      flag=1;
    }
    
  sqlite3_free_table(result);
  return flag;
}





