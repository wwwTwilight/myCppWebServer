#define _GNU_SOURCE // 在linux下，默认没有包含部分的glibc中提供的额外接口和类型，需要手动包含
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
#include <sys/wait.h>
#include <stdlib.h>


#define ISspace(x) isspace((int)(x))



#define SERVER_STRING "Server: SongHao's http/0.1.0\r\n"//定义个人server名称


void *accept_request(void* client);
void bad_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(int);


//  处理监听到的 HTTP 请求
void *accept_request(void* from_client)
{
	 int client = *(int *)from_client;
	 char buf[1024];
	 int numchars;
	 char method[255];
	 char url[255];
	 char path[512];
	 size_t i, j;
	 struct stat st; 
	 int cgi = 0;     
	 char *query_string = NULL;

	 numchars = get_line(client, buf, sizeof(buf));
	

	i = 0; 
	j = 0;
	 while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
	 {
		 //提取其中的请求方式
		  method[i] = buf[j];
		  i++; 
		  j++;
	 }
	method[i] = '\0';

	 if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	 {
		  unimplemented(client);
		  return NULL;
	 }

	 if (strcasecmp(method, "POST") == 0)  cgi = 1;
	  
	 i = 0;
	 while (ISspace(buf[j]) && (j < sizeof(buf)))
	  j++;


	 while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	 {
		  url[i] = buf[j];
		  i++; j++;
	 }
	url[i] = '\0';


	//GET请求url可能会带有?,有查询参数
	 if (strcasecmp(method, "GET") == 0)
	 {
		 
		  query_string = url;
		  while ((*query_string != '?') && (*query_string != '\0'))
				query_string++;
		 
		  /* 如果有?表明是动态请求, 开启cgi */
		  if (*query_string == '?')
		  {
			   cgi = 1;
			   *query_string = '\0';
			   query_string++;
		  }


		 }

	 sprintf(path, "httpdocs%s", url);


	 if (path[strlen(path) - 1] == '/')
	 {
		 strcat(path, "test.html");

	 }
 
	if (stat(path, &st) == -1) {
		  while ((numchars > 0) && strcmp("\n", buf))  
		   numchars = get_line(client, buf, sizeof(buf));

		  not_found(client);
	}
	else
	{
	

		if ((st.st_mode & S_IFMT) == S_IFDIR)//S_IFDIR代表目录
	   //如果请求参数为目录, 自动打开test.html
		{
			strcat(path, "/test.html");
		}
	
	  //文件可执行
	  if ((st.st_mode & S_IXUSR) ||
		  (st.st_mode & S_IXGRP) ||
		  (st.st_mode & S_IXOTH))
		  //S_IXUSR:文件所有者具可执行权限
		  //S_IXGRP:用户组具可执行权限
		  //S_IXOTH:其他用户具可读取权限  
			cgi = 1;

	  if (!cgi)

			serve_file(client, path);
	  else
	   execute_cgi(client, path, method, query_string);
  
	 }

	 close(client);
	 //printf("connection close....client: %d \n",client);
	 return NULL;
}



void bad_request(int client)
{
	 char buf[1024];
	//发送400
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


void cat(int client, FILE *resource)
{
	//发送文件的内容
	 char buf[1024];
	 fgets(buf, sizeof(buf), resource);
	 while (!feof(resource))
	 {

		  send(client, buf, strlen(buf), 0);
		  fgets(buf, sizeof(buf), resource);
	 }
}


void cannot_execute(int client)
{
	 char buf[1024];
	//发送500
	 sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "Content-type: text/html\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	 send(client, buf, strlen(buf), 0);
}


void error_die(const char *sc)
{
	 perror(sc);
	 exit(1);
}


//执行cgi动态解析
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
	 //默认字符
	 buf[0] = 'A'; 
	 buf[1] = '\0';
	 if (strcasecmp(method, "GET") == 0)

		 while ((numchars > 0) && strcmp("\n", buf))
		 {
			 numchars = get_line(client, buf, sizeof(buf));
		 }
	 else    
	 {

		  numchars = get_line(client, buf, sizeof(buf));
		  while ((numchars > 0) && strcmp("\n", buf))
		  {
				buf[15] = '\0';
			   if (strcasecmp(buf, "Content-Length:") == 0)
					content_length = atoi(&(buf[16]));

			   numchars = get_line(client, buf, sizeof(buf));
		  }

		  if (content_length == -1) {
		   bad_request(client);
		   return;
		   }
	 }


	 sprintf(buf, "HTTP/1.0 200 OK\r\n");
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
	 if (pid == 0)  /* 子进程: 运行CGI 脚本 */
	 {
		  char meth_env[255];
		  char query_env[255];
		  char length_env[255];

		  dup2(cgi_output[1], 1);
		  dup2(cgi_input[0], 0);


		  close(cgi_output[0]);//关闭了cgi_output中的读通道
		  close(cgi_input[1]);//关闭了cgi_input中的写通道


		  sprintf(meth_env, "REQUEST_METHOD=%s", method);
		  putenv(meth_env);

		  if (strcasecmp(method, "GET") == 0) {
		  //存储QUERY_STRING
		   sprintf(query_env, "QUERY_STRING=%s", query_string);
		   putenv(query_env);
		  }
		  else {   /* POST */
			//存储CONTENT_LENGTH
		   sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
		   putenv(length_env);
		  }


		  execl(path, path, NULL);//执行CGI脚本
		  exit(0);
	 } 
	 else {  
		  close(cgi_output[1]);
		  close(cgi_input[0]);
		  if (strcasecmp(method, "POST") == 0)

			 for (i = 0; i < content_length; i++) 
			   {

				recv(client, &c, 1, 0);

				write(cgi_input[1], &c, 1);
			   }



		//读取cgi脚本返回数据

		while (read(cgi_output[0], &c, 1) > 0)
			//发送给浏览器
		{			
			send(client, &c, 1, 0);
		}

		//运行结束关闭
		close(cgi_output[0]);
		close(cgi_input[1]);


		  waitpid(pid, &status, 0);
	}
}


//解析一行http报文
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
				recv(sock, &c, 1, 0);
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
	return(i);
}


void headers(int client, const char *filename)
{
	 
	 char buf[1024];

	 (void)filename;  /* could use filename to determine file type */
	//发送HTTP头
	 strcpy(buf, "HTTP/1.0 200 OK\r\n");
	 send(client, buf, strlen(buf), 0);
	 strcpy(buf, SERVER_STRING);
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "Content-Type: text/html\r\n");
	 send(client, buf, strlen(buf), 0);
	 strcpy(buf, "\r\n");
	 send(client, buf, strlen(buf), 0);

}


//返回404错误页面，组装信息
void not_found(int client)
{
	 char buf[1024];
	 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
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


//如果不是CGI文件，也就是静态文件，直接读取文件返回给请求的http客户端即可
void serve_file(int client, const char *filename)
{
	 FILE *resource = NULL;
	 int numchars = 1;
	 char buf[1024];
	 buf[0] = 'A'; 
	 buf[1] = '\0';
	 while ((numchars > 0) && strcmp("\n", buf)) 
	 {
		 numchars = get_line(client, buf, sizeof(buf));
	 }
	
	 //打开文件
	 resource = fopen(filename, "r");
	 if (resource == NULL)
	  not_found(client);
	 else
	 {
	  headers(client, filename);
	  cat(client, resource);
	 }
	 fclose(resource);//关闭文件句柄
}

//启动服务端
int startup(u_short *port) 
{
	 int httpd = 0,option;
	 struct sockaddr_in name;
	//设置http socket
	 httpd = socket(PF_INET, SOCK_STREAM, 0); // 创建一个socket，PF_INET表示使用IPv4协议，SOCK_STREAM表示使用TCP协议，0表示使用默认协议，返回的socket描述符存储在httpd中
	 if (httpd == -1)
		error_die("socket");//连接失败
	
	socklen_t optlen;
	optlen = sizeof(option);
    option = 1;
    setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);// 这一段都是用于实现端口复用
	
	
	 memset(&name, 0, sizeof(name)); // memset函数用于将一块内存区域置为某个值，第一个参数是内存区域的起始地址，第二个参数是要置的值（限制int），第三个参数是内存区域的大小
	 name.sin_family = AF_INET; // AF_INET表示使用IPv4协议
	// htons和htonl函数用于将主机字节序转换为网络字节序
	// 不同的计算机体系结构（比如 x86、ARM）在存储多字节数据（如 int、short）时，字节顺序可能不同（大端/小端）。
	// 而网络协议规定，数据在网络上传输时必须用大端字节序（高位字节在前）。
	 name.sin_port = htons(*port); // 端口号转换并赋值给name.sin_port
	 name.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY表示监听所有可用的网络接口

	 if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0) // bind函数用于将socket与指定的地址和端口绑定，第一个参数是socket描述符，第二个参数是地址和端口的结构体指针，第三个参数是结构体的大小，bind就是告诉操作系统，这个socket将要监听这个地址和端口
	  error_die("bind");//绑定失败
	 if (*port == 0)  /*如果没有输入端口，则动态分配一个端口 */
	 {
	  socklen_t  namelen = sizeof(name);
	  if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1) // getsockname函数用于获取socket的地址和端口，第一个参数是socket描述符，第二个参数是地址和端口的结构体指针，第三个参数是结构体的大小，这个函数的执行结果就是将socket的地址和端口存储在name中
	   error_die("getsockname");
	  *port = ntohs(name.sin_port); // ntohs函数用于将网络字节序转换为主机字节序，将name.sin_port转换为主机字节序并赋值给port，实现动态分配端口
	 }

	 if (listen(httpd, 5) < 0) // listen函数用于将socket设置为监听状态，第一个参数是socket描述符，第二个参数是监听队列的最大长度，listen就是告诉操作系统，这个socket将要监听这个端口，并且最多可以接受5个连接请求
	  error_die("listen");
	 return(httpd);
}


void unimplemented(int client)
{
	 char buf[1024];
	//发送501说明相应方法没有实现
	 sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
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

/*****************************主函数，也就是函数入口*****************************************/

int main(void)
{
	 int server_sock = -1;
	 u_short port = 6379;//默认监听端口号 port 为6379
	 int client_sock = -1;
	 struct sockaddr_in client_name;
	 // sockaddr_in是一个结构体，它用于存储ip地址和端口号
	 socklen_t client_name_len = sizeof(client_name); // socklen_t是专门用于存储套接字地址长度的类型
	 pthread_t newthread;
	 server_sock = startup(&port);
 
	 printf("http server_sock is %d\n", server_sock);
	 printf("http running on port %d\n", port);
	 while (1)
	 {

		  client_sock = accept(server_sock,
							   (struct sockaddr *)&client_name,
							   &client_name_len); // accept函数用于接受一个连接请求，第一个参数是socket描述符，第二个参数是客户端的地址和端口的结构体指针，第三个参数是结构体的大小，accept就是告诉操作系统，这个socket将要接受一个连接请求，并且将客户端的地址和端口存储在client_name中，让服务器接收一个新的客户端连接，并为它分配一个新的 socket，后续所有和这个客户端的通信都用 client_sock 这个描述符来完成
		  
		  printf("New connection....  ip: %s , port: %d\n",inet_ntoa(client_name.sin_addr),ntohs(client_name.sin_port));
		  if (client_sock == -1)
				error_die("accept");

		 if (pthread_create(&newthread , NULL, accept_request, (void*)&client_sock) != 0)
		   perror("pthread_create");

	 }
	 close(server_sock);

	return(0);
}
