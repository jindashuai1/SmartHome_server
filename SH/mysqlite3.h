#ifndef  _MYSQLITE3_H
#define  _MYSQLITE3_H


void Init_sqlite3();
int sqlite3_user_exist(const char *name);
void database_insert_user(const char *u,const char *p);
int sqlite3_MatPassword(const char *u,const char *p);
#endif
