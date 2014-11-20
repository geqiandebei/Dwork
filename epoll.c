#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <errno.h>  
#include <sys/socket.h>  
#include <netdb.h>  
#include <fcntl.h>  
#include <sys/epoll.h>  
#include <string.h>  
#include <arpa/inet.h>
#include <sys/epoll.h>
#define MAXEVENTS 64  
#define MAXBUF 254
//端口由参数argv[1]指定  
void set_nonblock(int fd);

int  main (int argc, char *argv[])  
{  
	int serfd,epfd;
	int num,i,datanum,tmp;
	struct sockaddr_in server;
	struct epoll_event eve;
	struct epoll_event* revent=NULL;
	
	char recv_buf[MAXBUF];
	char send_buf[MAXBUF];
	if( argc !=3 )
	{
			fprintf(stderr,"useage %s ip port ",argv[0]);
			exit(-1);
	}

	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	if(! inet_aton(argv[1],&server.sin_addr))	//失败时返回 0
	{
		fprintf(stderr,"ipaddr tranfer failed \n");
		exit(-1);
	}
	if((serfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
			fprintf(stderr,"socket failed \n");
			exit(-1);
	}
	if(bind(serfd,(struct sockaddr*)&server,sizeof(struct sockaddr)))
	{
			fprintf(stderr,"bind error\n");
			exit(-1);
	}
	printf("socket:%d bind already succeed!\n",serfd);
	listen(serfd,12);
	epfd=epoll_create(64);
	if(epfd<0)
	{
			fprintf(stderr,"epoll_creat error\n");
			exit(-1);
	}
	set_nonblock(serfd);
	eve.events=EPOLLIN|EPOLLET;
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,serfd,&eve)<0) exit(-1);
	
	revent=calloc(sizeof(struct epoll_event),MAXEVENTS);
	
	int len=sizeof(struct sockaddr);
	while(1)
	{
			num=epoll_wait(epfd,revent,MAXEVENTS,-1);
			for(i=0;i<num;i++)
			{
					if(revent[i].data.fd==serfd)
					{
						int clientfd;
						struct sockaddr_in client;
						if((clientfd=accept(serfd,(struct sockaddr*)&client,&len))<0)
						{
								fprintf(stderr,"accpet error\n");
								exit(-1);
						}
						eve.events=EPOLLIN|EPOLLET;	//
						set_nonblock(clientfd);
						epoll_ctl(epfd,EPOLL_CTL_ADD,clientfd,&eve);
					}
					else
					{
						bzero(recv_buf,MAXBUF);
						tmp=revent[i].data.fd;
						datanum=recv(tmp,recv_buf,MAXBUF,0);
						printf("[%d] send %s\n",tmp,recv_buf);
					}
			}
			sleep(1);
		
	}
	close(serfd);
	close(epfd);
}  

void set_nonblock(int fd)
{
		int flag;
		flag=fcntl(fd,F_GETFL,0);
		flag|=O_NONBLOCK;
		fcntl(fd,F_SETFL,flag);
}
