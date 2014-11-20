#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_aton 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

void main()
{
	int fd;
	if((fd=socket(AF_INET,SOCK_STREAM,0))==-1 )
	{
			printf("socket error ");
			exit(-1);
	}
	struct sockaddr_in server;
	server.sin_port=8888;
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htons(INADDR_ANY);
	
	if(bind(fd,(struct sockaddr*)&server,sizeof(server))<0)
	{
			printf("bind");
			exit(-1);
	}
	listen(fd,8);
	while(1)
	{
			int cfd;
			int len=sizeof(struct sockaddr);
			cfd=accept(fd,NULL,0);
	}
	
	
	}
