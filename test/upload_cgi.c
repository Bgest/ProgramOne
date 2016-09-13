#include "fcgi_config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <hiredis.h>
#include "redis_op.h"
#include "make_log.h"
#include "fcgi_stdio.h"
#include "util_cgi.h"
#define FILE_PATH "/home/test01/program_second/source/"
#define FDFS_TEST_MUDULE "logs"
#define FDFS_TEST_PROC   "logss"
#define FILE_ID_LEN     256
#define GROUP_NAME    "/home/yuqing/fastdfs/fastdfs0/data/"
#define TIME_STRING_LEN   25



/*#define jpg,"png","txt","js","exe"*/



int UpFastDFS(const char* file_path, char* remote_path);
int str_replaces(char *p_result,char* p_source,char* p_seach,char *p_repstr);
int InitRedis(char* redis_ip, char * redis_port, redisContext **conn);
int main ()
{
    char *file_buf = NULL;
    char boundary[256] = {0};
    char content_text[256] = {0};
    char filename[256] = {0};
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
    char path_str[256] = {0};
    while (FCGI_Accept() >= 0) {
    		char *redis_value_buf = (char*)malloc(1024);
    		memset(redis_value_buf, 0, 1024);
    		char *url = (char*)malloc(1024);
    		memset(url, 0, 1024);
    		time_t now;
    		now = time(NULL);
    		char create_time[25];
    		int fd;
    		char suffix[10];
    		
    		char *remote_path = (char*)malloc(FILE_ID_LEN);
    		memset(remote_path, 0, FILE_ID_LEN);
    		char *p_result = (char*)malloc(FILE_ID_LEN);
    		memset(p_result, 0, FILE_ID_LEN);    
    		redisContext *conn;
    		int ret = 0;
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;
        printf("Content-type: text/html\r\n"
                "\r\n");

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }
        LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	

        if (len <= 0) {
            printf("No data from standard input\n");
        }
        else {
            int i, ch;
            char *begin = NULL;
            char *end = NULL;
            char *p, *q, *k;
       			 LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	

            //==========> ���ٴ���ļ��� �ڴ� <===========

            file_buf = malloc(len);
            if (file_buf == NULL) {
                printf("malloc error! file size is to big!!!!\n");
                return -1;
            }
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            begin = file_buf;
            p = begin;
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                //putchar(ch);
                *p = ch;
                p++;
            }
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            //===========> ��ʼ����ǰ�˷��͹�����post���ݸ�ʽ <============
            //begin deal
            end = p;

            p = begin;

            //get boundary
            p = strstr(begin, "\r\n");
            if (p == NULL) {
                printf("wrong no boundary!\n");
                goto END;
            }
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            strncpy(boundary, begin, p-begin);
            boundary[p-begin] = '\0';
            //printf("boundary: [%s]\n", boundary);

            p+=2;//\r\n
            //�Ѿ�������p-begin�ĳ���
            len -= (p-begin);
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            //get content text head
            begin = p;

            p = strstr(begin, "\r\n");
            if(p == NULL) {
                printf("ERROR: get context text error, no filename?\n");
                goto END;
            }
    		    LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            strncpy(content_text, begin, p-begin);
            content_text[p-begin] = '\0';
            //printf("content_text: [%s]\n", content_text);
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            p+=2;//\r\n
            len -= (p-begin);

            //get filename
            // filename="123123.png"
            //           ��
            q = begin;
            q = strstr(begin, "filename=");
            
            q+=strlen("filename=");
            q++;
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            k = strchr(q, '"');
            strncpy(filename, q, k-q);
            filename[k-q] = '\0';

            trim_space(filename);
            //printf("\r\nfilename: [%s]\r\n", filename);
				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            //get file
            begin = p;     
            p = strstr(begin, "\r\n");
            p+=4;//\r\n\r\n
            len -= (p-begin);

            begin = p;
            // now begin -->file's begin
            //find file's end
            p = memstr(begin, len, boundary);
            if (p == NULL) {
                p = end-2;    //\r\n
            }
            else {
                p = p -2;//\r\n
            }
            //begin---> file_len = (p-begin)

            //=====> ��ʱbegin-->p����ָ����������post���ļ�����������
            strcpy(path_str,FILE_PATH);
            strcat(path_str,filename);
            printf("\r\npath_str: [%s]\r\n", path_str);
            printf("\r\nlen=: [%d]\r\n", p - begin);
            //======>������д���ļ���,�����ļ���Ҳ�Ǵ�post���ݽ�������  <===========
            fd = open(path_str, O_RDWR|O_CREAT, 0777);
            if(fd < 0)
            {
            	printf("\r\nopen error\r\n");
            	goto END;
            }
 				LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
           	if(write(fd, begin, p - begin) < 0)
            {
            	printf("\r\nwrite error\r\n");
            	goto END;
            }
            close(fd);
            //===============> �����ļ�����fastDFS��,���õ��ļ���file_id <============
         LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	   
            if( UpFastDFS(path_str, remote_path)<0)
            {
            	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "UpFastDFS error");
            	goto END;
            }
            //ȥ��/n
            remote_path[strlen(remote_path)-1] = '\0';
            LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "remote_path=%s",remote_path);
           	//===============> �ļ������ַ���ƴ�� <============
           	strftime(create_time, TIME_STRING_LEN-1, "%Y-%m-%d %H:%M:%S", localtime(&now));//ʱ���ʽ��
           	get_file_suffix(filename, suffix);//�õ��ļ���׺�ַ���
           	sprintf(url, "http://%s/%s", getenv("SERVER_ADDR"),remote_path);//�õ�url
           	sprintf(redis_value_buf,"%s|%s|%s|%s|%s|%s", remote_path,url,filename,create_time,"user",suffix);
           	printf("\r\nredis_value_buf=%s\r\n", redis_value_buf);
            LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "redis_value_buf complete");
            LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            
						//===============> ����Redis���ݿ� <============
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "log test");
           	if(InitRedis(NULL, 0, &conn) != 0)
           	{
           		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "InitRedis error");
           		printf("InitRedis error");
           		goto END;
           	}
           	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
           	ret = rop_is_key_exist(conn, FILE_INFO_LIST);
           	if(ret == 0)
           	{
           		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "create FILE_INFO_LIST");
           		
           	}else if (ret == -1)
           	{
           		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "FILE_INFO_LIST error");
           		goto END;
           	}
           	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
           	

           	//����
           	/*------------------------------------------.
						| ������ļ��� (ZSET) |
						| Key: FILE_HOT_ZSET |
						| Member: file_id |
						| Score: pv |
						| redis ��� |
						| ZINCRBY key increment member |
						`------------------------------------------*/
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "log test2");
						ret = rop_zset_increment(conn, FILE_HOT_ZSET, remote_path);
						printf(FILE_HOT_ZSET);
						printf(remote_path);
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
           	if(ret < 0)
           	{
           		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "rop_zset_increment error");
           	//	goto END;	
           	}
           	
           	if(rop_list_push(conn, FILE_INFO_LIST, redis_value_buf)<0)
           	{
           		LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "rop_list_push error");
           		ret = -1;
           		goto END;
           	}
           	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
           	rop_disconnect(conn);

            //================ > �õ��ļ������storage��host_name <=================
END:
						LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            memset(boundary, 0, 256);
            memset(content_text, 0, 256);
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);
						free(p_result);
            free(remote_path);
            free(file_buf);
            free(redis_value_buf);
            free(url);
            LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "!!!!log test!!!!!!");	
            
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */

    return 0;
}

int UpFastDFS(const char* file_path, char* remote_path)
{
	if (file_path == NULL && remote_path == NULL) {
       printf("usage ./a.out file-name");
       LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "file_path == NULL && remote_path == NULL");
       exit(0);
   }
	pid_t pid;
	int pfd[2];
	if (pipe(pfd) < 0) {
        LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "[errror], pipe error");
        exit(1);
    }
  pid = fork();
  if (pid == 0) {
       //chlid
       //�رն���
       close(pfd[0]);

       //����׼��� �ض��򵽹ܵ���
       dup2(pfd[1], STDOUT_FILENO);

       //exec
       execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", file_path, NULL);
       LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "exec fdfs_upload_file error");
   }
  else {
   //parent
   //�ر�д��
   close(pfd[1]);

   wait(NULL);

   //�ӹܵ���ȥ������
   if(read(pfd[0], remote_path, FILE_ID_LEN)<0)
   	{
    	LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "read error");
    	exit(1);
   	}
	

  
	}
	return 0;
	
}


//��ʼ��redis
int InitRedis(char* redis_ip, char* redis_port, redisContext **conn)
{
	//Redis����
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



/*-----------------------------------------------------------.
| �ļ���Ϣ��(LIST) |
| Key: FILE_INFO_LIST |
| Value: file_id||url||filename||create time||user||type |
| redis ��� |
| ���� LPUSH key value |
| ��ѯ���� LLEN key |
| ���������Ҫ�ض� LTRIM key 0 max-1 |
| ��ѯ�������� LRANGE key 0 max-1 |
`-----------------------------------------------------------*/






/*------------------------------------------.
| ������ļ��� (ZSET) |
| Key: FILE_HOT_ZSET |
| Member: file_id |
| Score: pv |
| redis ��� |
| ZINCRBY key increment member |
`------------------------------------------*/

























/*
 str_replaces(p_result,remote_path,"group1/M",GROUP_NAME);  
            //����Զ��·��
            printf("\r\nremote_path = %s\r\n", p_result);
            free(p_result);
            free(remote_path);
            free(redis_value_buf);
*/




/*
//�ַ����滻����  
******************************************************************** 
*  Function��  my_strstr() 
*  Description: ��һ���ַ����в���һ���Ӵ�; 
*  Input��      ps: Դ;      pd���Ӵ� 
*  Return :    0��Դ�ַ�����û���Ӵ�; 1��Դ�ַ��������Ӵ�; 
*********************************************************************/
/*
char * my_strstr(char * ps,char *pd)  
{  
    char *pt = pd;  
    int c = 0;  
    while(*ps != '\0')  
    {  
        if(*ps == *pd)  
        {  
            while(*ps == *pd && *pd!='\0')  
            {  
                ps++;  
                pd++;  
                c++;  
            }  
        }else  
        {  
            ps++;  
        }  
        if(*pd == '\0')  
        {  
            return (ps - c);  
        }  
        c = 0;  
        pd = pt;  
    }  
    return 0;  
}
*/
/******************************************************************** 
*  Function��  str_replaces() 
*  Description: ��һ���ַ����в���һ���Ӵ������Ұ����з��ϵ��Ӵ��� 
��һ���滻�ַ����滻�� 
*  Input��      p_source:Ҫ���ҵ�ĸ�ַ����� p_seachҪ���ҵ����ַ���; 
p_repstr���滻���ַ���; 
*  Output��      p_result:��Ž��; 
*  Return :      �����滻�ɹ����Ӵ�����; 
*  Others:      p_resultҪ�㹻��Ŀռ��Ž�����������������Ҫ��\0����; 
*********************************************************************/  
/*
int str_replaces(char *p_result,char* p_source,char* p_seach,char *p_repstr)  
{
    int c = 0;
    int repstr_leng = 0;
    int searchstr_leng = 0;

    char *p1;
    char *presult = p_result;
    char *psource = p_source;
    char *prep = p_repstr;
    char *pseach = p_seach;
    int nLen = 0;
  
    repstr_leng = strlen(prep);
    searchstr_leng = strlen(pseach);  
  
    do{   
        p1 = strstr(psource,p_seach);  
  
        if (p1 == 0)  
        {  
            strcpy(presult,psource);  
            return c;  
        }  
        c++;  //ƥ���Ӵ�������1;  
        printf("���:%s\r\n",p_result);  
        printf("Դ�ַ�:%s\r\n",p_source);  

        // ������һ���滻�����һ���滻���м���ַ���  
        nLen = p1 - psource;
        memcpy(presult, psource, nLen);

        // ������Ҫ�滻���ַ���
        memcpy(presult + nLen,p_repstr,repstr_leng);

        psource = p1 + searchstr_leng;
        presult = presult + nLen + repstr_leng;
    }while(p1);  

    return c;
}
*/