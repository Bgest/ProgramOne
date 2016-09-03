#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

#include"make_log.h"

#define MAXLINE 80

int main(int argc, char *argv[])
{
	LOG("log3", "main_logs", "%s", "main start");
	pid_t pid;
	int pipefd[2] , ret;
	char line[MAXLINE];
	ret = pipe(pipefd);
	if(ret != 0)
	{
		LOG("log3", "main_logs", "%s", "func pipe error");
		perror("pipe error");
		exit(1);
	}
	if((pid = fork()) < 0)
	{
		LOG("log3", "main_logs", "%s", "func fork error");
		exit(1);
	}
	if(pid>0)
	{
		LOG("log3", "main_logs", "%s", "father pro fork");
		close(pipefd[1]);
		wait(NULL);//等待儿子执行完毕再去读
		printf("%s" ,"line");
		read(pipefd[1], line, MAXLINE);
		printf("%s" ,line);
		LOG("log3", "main_logs", "file_id = [%s]", line);
		
	}
	else
	{
		LOG("log3", "main_logs", "%s", "child pro fork");
		close(pipefd[0]);//关闭读端
		
		dup2(pipefd[1], STDOUT_FILENO);
		execlp("fdfs_upload_file", "fdfs_upload_file", "./conf/client.conf", argv[1], NULL);
		LOG("log3", "main_logs", "%s", "func execlp error");
	}

	return 0;
}