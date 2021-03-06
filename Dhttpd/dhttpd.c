#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

#define WorkRoom "/home/da/Dwork/Dhttpd"   //设置工作目录
#define SERVER_STRING "Server: Dhttpd1.0\r\n"

#define ISspace(x) isspace((int)(x))

void accept_request(int);
void bad_request(int);
void cat(int, int );
void cannot_execute(int);
void error_die(const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(int);
void execute_cgi(int client, const char *path, const char *method, const char *query_string);

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client, const char *path,
                 const char *method, const char *query_string)
{
	 char buf[1024];
	 int cgi_output[2];
	 int cgi_input[2];
	 pid_t pid;
	 int status;
	 int i;
	 char c;
	 int numchars = 1;
	 int content_length = -1;
	 buf[0] = 'A'; buf[1] = '\0';
	 if (strcasecmp(method, "POST") == 0)
	 {
	  content_length=atoi(query_string); //获取post方法传输数据长度
	  if (content_length <=0) {
	   bad_request(client);
	   return;
	  }
	}
	 sprintf(buf, "HTTP/1.1 200 OK\r\n");
	 send(client, buf, strlen(buf), 0);
	 if (pipe(cgi_output) < 0) {
	  cannot_execute(client);
	  return;
	 }
	 if (pipe(cgi_input) < 0) {
	  cannot_execute(client);
	  return;
	 }

	 if ( (pid = fork()) < 0 ) {
	  cannot_execute(client);
	  return;
	 }
	 if (pid == 0)  /* child: CGI script */
	 {
	  char meth_env[255];
	  char query_env[255];
	  char length_env[255];
	  char p[255];
	  dup2(cgi_output[1], 1);
	  dup2(cgi_input[0], 0);
	  close(cgi_output[0]);
	  close(cgi_input[1]);
	  sprintf(meth_env, "REQUEST_METHOD=%s", method);
	  putenv(meth_env);
	  if (strcasecmp(method, "GET") == 0) {
	   sprintf(query_env, "QUERY_STRING=%s", query_string);
	   putenv(query_env);
	   printf("Query_string OK \n");
	  }
	  else {   /* POST */
	   sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
	   putenv(length_env);
	  }	  
	  sprintf(p,".%s",path);					//	修改可执行
	  execl(p, path, NULL);
	  exit(0);
	 } 
	 else 
	 {    /* parent */
	  close(cgi_output[1]);
	  close(cgi_input[0]);
	  if (strcasecmp(method, "POST") == 0)
	   for (i = 0; i < content_length; i++) {
		recv(client, &c, 1, 0);
		write(cgi_input[1], &c, 1);
	   }
	  while (read(cgi_output[0], &c, 1) > 0)
	   send(client, &c, 1, 0);
	  close(cgi_output[0]);
	  close(cgi_input[1]);
	  waitpid(pid, &status, 0);
	 }
 
}

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
void accept_request(int client)
{
	 char buf[1024];
	 int numchars;
	 char method[255];
	 char url[255];
	 char path[512];
	 char version[32];
	 char host[32];
	 char *query_string=NULL;
	 char length[32];
	 size_t i, j;
	 struct stat st;
	 memset(buf,0,sizeof(buf));
	 memset(method,0,sizeof(method));
	 memset(url,0,sizeof(method));
	 memset(length,0,sizeof(length));
	 
	 numchars = get_line(client, buf, sizeof(buf)); 
	 i = 0; j = 0;
	 while (!ISspace(buf[j]) && (i < sizeof(method) - 1)) 
	 {
	  method[i] = buf[j];
	  i++; j++;
	 }
	 method[i] = '\0';							// 获取Method
	 printf("%s\n",method);
	 if (!strcasecmp(method,"PUT") || !strcasecmp(method,"DELETE") ) 
	 {
	  unimplemented(client);
	  return;
	 }

	 i = 0;
	 while (ISspace(buf[j]) && (j < sizeof(buf)))
	  j++;
	 while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	 {
	  url[i] = buf[j];
	  i++; j++;
	 }
	 url[i] = '\0';							//获取URL
	 printf("%s\n",url);
	 
	 i=0;
	 while(ISspace(buf[j])&&(j<sizeof(buf)))
		j++;
	 while(!ISspace(buf[j]) && (i<sizeof(version)-1) && (j<sizeof(buf)))
		{
			version[i]=buf[j];
			i++;j++;
		}
		version[i]='\0';			//获取HTTP版本号
	printf("%s\n",version);

	memset(buf,0,sizeof(buf));
	while(get_line(client, buf,sizeof(buf)))
	{
		printf("%s",buf);
		if (strncasecmp("Content-Length:",buf,15) == 0)
		{
			query_string=&buf[16];                  //获取content-length长度，作为query_string
			printf("get length %s",query_string);
			strcpy(length,query_string);
		}
		memset(buf,0,sizeof(buf));
	}
	printf(">>>>>>>>>>>>>> 请求行内容获取完毕 : 方法 URL HTTP版本号 >>>>>>>>>>>>>>\n");
	if (strcasecmp(method, "GET") == 0)						//get方法
	{
		 if( (query_string=strchr(url,'?'))==NULL )        //不存在?号,不为CGI请求
		 {
			 sprintf(path, "webs%s", url);
			 if (path[strlen(path) - 1] == '/')
			  strcat(path, "index.html");
			 if (stat(path, &st) == -1 ||  (( st.st_mode & S_IFMT) == S_IFDIR)  )
			  {
					not_found(client);
			   }
			 else 
			   serve_file(client, path);						//文件传输
	       }
		else 				//CGI  GET
		{
			*query_string='\0';
		    sprintf(path,"%s",url);
			query_string++;				//指向?后内容
			printf("GET--->query_string:%s   path:%s\n",query_string,path);
			execute_cgi( client,path,method,query_string);
		}
	}
	else if(strcasecmp(method,"POST")==0)		//POST方法
	{
		printf("POST--->query_string:%s  path:%s\n",query_string,url);
		execute_cgi(client,url,method,length);
	}

	 close(client);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
void bad_request(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "<P>Your browser sent a bad request, ");
 send(client, buf, sizeof(buf), 0);
 sprintf(buf, "such as a POST without a Content-Length.\r\n");
 send(client, buf, sizeof(buf), 0);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client, int filefd)
{
 char buf[1024];
 int n=1,sended=0,m;
 while (( n=read(filefd,buf,sizeof(buf))) >0 )
 {
	printf("read %d bytes\n",n);
 	m=n;
	while(m)
	{
		sended=send(client,buf+sended,n-sended,0);
		printf("send %d bytes,rest %s\n",sended,buf+sended);
		if(sended==-1)
		{
			printf("send error!");
			return -1;
		}
		 m-=sended;
	}
	 memset(buf,0,sizeof(buf)); 
 }
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
 perror(sc);
 exit(1);
}
/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sock, char *buf, int size)
{
	 int i = 0;
	 char c = '\0';
	 int n;
	 while ((i < size - 1) && (c != '\n'))
	 {
		  n = recv(sock, &c, 1, 0);
		  if (n > 0)
		  {
			   if (c == '\r')
			   {
					n = recv(sock, &c, 1, MSG_PEEK);
					if ((n > 0) && (c == '\n'))
					 recv(sock, &c, 1, 0);			// Why need to recv another ?
					else
					 c = '\n';
			   }
			   buf[i] = c;		
			   i++;
		  }
		  else			
		   c = '\n';
	 }
	 buf[i] = '\0';
	 return(i-1);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
void headers(int client, const char *filename)
{
 char buf[1024];
 char type[32]; 
 char *p=NULL;
 if(p=strchr(filename,'.'))
 {
	if(strcasecmp(p+1,"jpg")==0)	strcpy(type,"application/x-jpg");
	else if(strcasecmp(p+1,"html")==0) strcpy(type,"text/html");
	else if(strcasecmp(p+1,"bmp")==0) strcpy(type,"application/x-bmp");
	else if(strcasecmp(p+1,"png")==0) strcpy(type,"application/x-png");
	printf("type: %s\n",type);
}
 strcpy(buf, "HTTP/1.0 200 OK\r\n");
 send(client, buf, strlen(buf), 0);
 strcpy(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: %s\r\n",type);
 send(client, buf, strlen(buf), 0);
 strcpy(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "your request because the resource specified\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "is unavailable or nonexistent.\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
void serve_file(int client, const char *filename)
{
	int filefd;
	int numchars = 1;
	char buf[1024];
	filefd=open(filename,O_RDONLY);
	if(filefd==-1)  not_found(client);
	else
	{
		printf("%s open .\n",filename); 
		headers(client, filename);
		cat(client, filefd);
	}
	close(filefd);
}

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
int startup(u_short *port)
{
 int httpd = 0;
 struct sockaddr_in name;

 httpd = socket(PF_INET, SOCK_STREAM, 0);
 if (httpd == -1)
  error_die("socket");
 memset(&name, 0, sizeof(name));
 name.sin_family = AF_INET;
 name.sin_port = htons(*port);
 name.sin_addr.s_addr = htonl(INADDR_ANY);
 if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
  error_die("bind");
 if (listen(httpd, 5) < 0)
  error_die("listen");
 return(httpd);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client)   // 未实现的方法
{
 char buf[1024];

 sprintf(buf, "HTTP/1.1 501 Method Not Implemented\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</TITLE></HEAD>\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(client, buf, strlen(buf), 0);
}

/**********************************************************************/

int main(int argc,char *argv[])
{
	int server_sock = -1;
	int i;
	u_short port = 8888;
	int client_sock = -1;
	struct sockaddr_in client_name;
	int client_name_len = sizeof(client_name);
	pthread_t newthread;
	pid_t pid;
	if(argc==2)
	{
	 port=atoi(argv[1]);
	}
	pid=fork();
	if(pid==-1)
	{
	 perror("dhttpd start failed! fork error...\n");
	 exit(1);
	}
	else if(pid>0) exit(0);             //kill parent process
	setsid();
	chdir(WorkRoom);
	umask(0655);
	for(i=0;i<3;i++) close(i);	//close opened file description
	server_sock = startup(&port);
	printf("httpd running on port %d\n", port);

	while (1)
	{
	client_sock = accept(server_sock,
					   (struct sockaddr *)&client_name,
					   &client_name_len);
	if (client_sock == -1)
	error_die("accept");
	/* accept_request(client_sock); */
	if (pthread_create(&newthread , NULL, (void*)accept_request, (void*)client_sock)!= 0)
	perror("pthread_create");
	printf("connect estabilshed\n");
	}

	close(server_sock);
	return(0);
}
