#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void err_sys(const char *errmsg);

int main(int argc, char **argv)
{
    struct servent *service;
    int i;    
    
    if (argc != 3) {
        fprintf(stderr, "usage: ./p/1 servname protoname\n");
        exit(1);
    }

    /* port argument must be is network byte order */
    service = getservbyport(htons(atoi(argv[1])), argv[2]);    
    if (service == NULL)
        err_sys("getservbyport");
    
    printf("official service name: %s\n", service->s_name);
    for (i = 0; service->s_aliases[i] != NULL; i++)    
        printf("alias: %s\n", service->s_aliases[i]);
    printf("port number = %d\n", ntohs(service->s_port));
    printf("protocol name: %s\n", service->s_proto);
    
    exit(0);
}

void err_sys(const char *errmsg)
{
    perror(errmsg);
    exit(1);
} 
