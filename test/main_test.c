#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<hiredis.h>
#include"redis_op.h"
#include"redis_keys.h"

#define LOG_MAIN_TEST "test"
#define LOG_MAIN_PROC "mian-test"



int main(int argc, char **argv)
{
		redisContext *conn = NULL;
		char *ip = "127.0.0.1";
		char *port = "6379";
		char *value = NULL;
		char *name = "name";
		
		conn = rop_connectdb_nopwd(ip, port);
		if(conn == NULL){
			LOG(LOG_MAIN_TEST, LOG_MAIN_PROC, "conn redis server error");
			exit(1);
		}
		printf("connect server succ!\n");
		
		rop_redis_get(conn,name, value);
		return 0;
}