#include<stdio.h>#include<stdlib.h>#include<string.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/msg.h>
#define BUFSIZE 255
#define KEY	1993822typedef struct my_msg
{
		long int msg_type;
		char data[BUFSIZE];
}MSG;
//消息队列字符串发送int main(){    int msgid;    int msgtype=0;    int runflag=1;    MSG msgbuf;    msgid=msgget((key_t)KEY,0666|IPC_CREAT);    if(msgid==-1)    {        perror("msg get error!\n");        exit(-1);    }    while(runflag)    {		printf("msgtype:");		scanf("%d",&msgtype);        if(msgrcv(msgid,(void*)&msgbuf,BUFSIZE,msgtype,0)==-1)        {            perror("msgrcv error!\n");            exit(-1);        }        printf("msg_type:%d data:%s\n",msgbuf.msg_type,msgbuf.data);        if(strcasecmp(msgbuf.data,"quit")==0)            runflag=0;    }    if(msgctl(msgid,IPC_RMID,0)==-1)    {        perror("msg remove ID error\n");        exit(-1);    }    printf("success.\n");    return 0;}