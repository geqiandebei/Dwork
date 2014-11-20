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

#define MAXBUF 254

int  main (int argc, char *argv[]) 
{
	int fd;
	int datanum;
	struct sockaddr_in server;
	char send_buf[MAXBUF];
	
	if( argc !=3 )
	{
			fprintf(stderr,"useage %s ip port ",argv[0]);
			exit(-1);
	}
	fd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	if(! inet_aton(argv[1],&server.sin_addr))	//失败时返回 0
	{
		fprintf(stderr,"ipaddr tranfer failed \n");
		exit(-1);
	}
	if(connect(fd,(struct sockaddr*)&server,sizeof(struct sockaddr))<0)
	{
		fprintf(stderr,"connect to server error \n");
		exit(-1); 
	}
	printf("----connect to server succeed!---\n");
	while(1)
	{
			bzero(send_buf,MAXBUF);
			scanf("%s",&send_buf);
			datanum=send(fd,send_buf,strlen(send_buf),0);
	}
	
	return 0;
}
