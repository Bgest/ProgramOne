//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/wait.h>
//#include <hiredis.h>
//#include "fcgi_config.h"
//#include "redis_op.h"
//#include "make_log.h"
//#include "fcgi_stdio.h"
//#include "util_cgi.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>

#include <hiredis.h>

#include "make_log.h"
#include "hiredis.h"
#include "util_cgi.h"
#include "redis_keys.h"
#include "redis_op.h"
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include "cJSON.h"
#define FDFS_TEST_MUDULE "mount"
#define FDFS_TEST_PROC "mounts"


int GetQueryURLInfo(const char *query_string, char *cmd, char *from_id ,char *count, char *usr);
int InitRedis(char* redis_ip, char* redis_port, redisContext **conn);
int GetValueInfo(const char* values, char *file_id, char *filename,char *user, char *create_time);
void CreateJSON(char *file_id,char *filename,char *create_time,cJSON *array);

int main()
{
	while (FCGI_Accept() >= 0) {
		/*==========================获取URL信息==================================*/
		char *query_string = getenv("QUERY_STRING");
		char cmd[100] = {0};
		char from_id[100] = {0};
		char count[100] = {0};
		char usr[100] = {0};
		int values_num=0;
		int i = 0;
		char file_id[VALUES_ID_SIZE] = {0};
		char filename[VALUES_ID_SIZE] = {0};
		char user[VALUES_ID_SIZE] = {0};
		char create_time[VALUES_ID_SIZE] = {0};
		char *out = (char*)malloc(VALUES_ID_SIZE*10);
		memset(out, 0, VALUES_ID_SIZE*10);
		cJSON *roots;
		cJSON *array;

		roots = cJSON_CreateObject();
		array = cJSON_CreateArray();	
		RVALUES values = NULL;
		values = malloc(8*VALUES_ID_SIZE);
		//获取url字符串
		if(GetQueryURLInfo(query_string, cmd, from_id ,count, usr)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "GetQueryURLInfo");
			goto END;//再没进行连接时如果disconn会崩溃么
		}
		//*==========================根据获取的URL信息，拿Redis信息==================================*/
		redisContext *conn;//连接句柄
		//typedef char (*RVALUES)[VALUES_ID_SIZE];
		//初始化链接
		if(InitRedis(NULL, 0, &conn) != 0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "InitRedis error");
			printf("InitRedis error");
			goto END;
		}
		//获取文件信息链表
		if(rop_range_list(conn, FILE_INFO_LIST, atoi(from_id), (atoi(from_id)+atoi(count)-1), values, &values_num)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "rop_range_list error");
			goto END;
		}
		
		
		for (i=0; i<values_num; i++)
		{
			//	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "%dvalues_num = %s",i,values_num);
			//解析字符串
			if(GetValueInfo(*(values+i), file_id, filename, user, create_time)<0)
			{
				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "GetValueInfo error");
			}
			else 
			{
				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "\n%dth\n:  file_id   = %s\nfilename = %s\nuser     = %s\ncreate_time = %s",i , file_id, filename, user, create_time);
			}
			//封装json
			CreateJSON(file_id,filename,create_time,array);
		}
		
		//总根节点
		cJSON_AddItemToObject(roots, "games", array);
		out = cJSON_Print(roots);
		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "JSON = %s",out);
		//输出前端
		printf("%s\n",out);
END:
		free(values);
		free(out);
		rop_disconnect(conn);
	}
	return 0;
	}


void CreateJSON(char *file_id,char *filename,char *create_time,cJSON *array)
{
			cJSON* item = cJSON_CreateObject();
			cJSON_AddStringToObject(item, "id", file_id);
			cJSON_AddNumberToObject(item, "kind", 2);
			cJSON_AddStringToObject(item, "title_m", filename);
			cJSON_AddStringToObject(item, "descrip", create_time);
			cJSON_AddStringToObject(item, "picurl_m", "http://192.168.21.13/static/file_png/pdf.png");
			cJSON_AddNumberToObject(item, "pv", 1);
			cJSON_AddNumberToObject(item, "hot", 0);
			//做好一个块后放入一个数组中
			cJSON_AddItemToArray(array, item);
}

	int GetValueInfo(const char* values, char *file_id, char *filename,char *user, char *create_time)
	{
		int ret=0;
		if(values == NULL)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "values == NULL");
			goto END;
			ret = -1;
		}
		if(file_id == NULL && filename == NULL && user == NULL && create_time == NULL)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "file_id == NULL && filename == NULL && user == NULL && create_time == NULL");
			goto END;
			ret = -1;
		}
		//处理链表value值得到value
		if(get_value_by_col(values, 1, file_id, VALUES_ID_SIZE-1, 0)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "get_value_by_col file_id error");
			goto END;
			ret = -1;
		}  
		//title_m(filename)
		if(get_value_by_col(values, 3, filename, VALUES_ID_SIZE-1, 0)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "get_value_by_col filename error");
			goto END;
			ret = -1;
		}
		//title_s
		if(get_value_by_col(values, 5, user, VALUES_ID_SIZE-1, 0)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "get_value_by_col user error");
			goto END;
			ret = -1;
		}
		//time
		if(get_value_by_col(values, 4, create_time, VALUES_ID_SIZE-1, 0)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "get_value_by_col create_time error");
			goto END;
			ret = -1;
		}
END:
		return ret;	

	}


	int InitRedis(char* redis_ip, char* redis_port, redisContext **conn)
	{
		//Redis连接
		redisContext *c;
		char *hostname = (redis_ip) ? redis_ip : "127.0.0.1";
		char *port = (redis_port) ? redis_port : "6379";

		//struct timeval timeout = { 1, 500000 }; // 1.5 seconds
		c = rop_connectdb_nopwd(hostname, port);
		if (c == NULL || c->err) {
			if (c) {
				printf("Connection error: %s\n", c->errstr);
				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "conn to:%s",c->errstr);
				redisFree(c);
			} else {
				printf("Connection error: can't allocate redis context\n");
				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "Connection error: can't allocate redis context");
			}
			return -1;	
		}
		*conn = c;
		return 0;	
	}


	int GetQueryURLInfo(const char *query_string, char *cmd, char *from_id ,char *count, char *usr)
	{
		int ret = 0;
		//错误判断
		if(query_string == NULL)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "query_string == NULL");
			ret = -1;
			goto END;
		}
		if(query_string == NULL && cmd == NULL && from_id == NULL && usr == NULL)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "cmd == NULL && from_id == NULL && usr == NULL");
			ret = -1;
			goto END;
		}
		//获取cmd
		if(query_parse_key_value(query_string, "cmd", cmd, NULL)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "query_parse_key_value cmd error");
			ret = -1;
			goto END;
		}
		//获取fromid
		if(query_parse_key_value(query_string, "fromId", from_id, NULL)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "query_parse_key_value fromid error");
			ret = -1;
			goto END;
		}
		//获取count
		if(query_parse_key_value(query_string, "count", count, NULL)<0)
		{
			LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "query_parse_key_value cmd error");
			ret = -1;
			goto END;
		}
		//获取usr
		//if(query_parse_key_value(query_string, "usr", usr, NULL)<0)
		//{
		//	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "query_parse_key_value usr error");
		//	ret = -1;
		//		goto END;
		//	}
		usr = "usr";
END:
		return ret;
	}
