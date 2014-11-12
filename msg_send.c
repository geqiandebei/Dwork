#include<stdio.h>#include<stdlib.h>#include<string.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/msg.h>
#define BUFSIZE 255
#define KEY	1993822
typedef struct my_msg
{
		long int msg_type;
		char data[BUFSIZE];
}MSG;
//消息队列字符串发送
int main()
{
	int msgid;    int runflag=1;    MSG msgbuf;
	msgid=msgget((key_t)KEY,0666|IPC_CREAT);
	if(msgid==-1)
	{
            perror("msgget failed!\n");
            exit(-1);    }    printf("msgid is %d\n",msgid);    printf("please start inputting...\n");
	while(runflag)    {		printf("msgtype:");		scanf("%d",&msgbuf.msg_type);		printf("msgdata:");        scanf("%s",&msgbuf.data);        if(msgsnd(msgid,(void*)&msgbuf,BUFSIZE,0)==-1)        {            perror("msgsnd error! \n");            exit(-1);        }        if(strcasecmp(msgbuf.data,"QUIT")==0)        runflag=0;        fflush(stdin);    }
        return 0;}
