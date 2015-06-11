/*
 * proxy.c - CS:APP Web proxy
 *
 * Student ID: 2013-11438 
 *         Name: Hahm, Jiung
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

/* The name of the proxy's log file */
#define PROXY_LOG "proxy.log"

/* Undefine this if you don't want debugging output */
#define DEBUG

/*
 * Functions to define
 */
void *process_request(void* vargp);
int open_clientfd_ts(char *hostname, int port, sem_t *mutexp);
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);

void* thread(void* vargp);

char* get_host(char* str)
{
	char* host = strtok(str, " ");
	return host;
}

char* get_port(char* str)
{
	char* port = strtok(str, " ");
	port = strtok(port, " ");
	return port;
}

char* get_message(char* str)
{
	char* msg = strtok(str, " ");
	msg = strtok(msg, " ");
	msg = strtok(msg, " ");
	return msg;
}

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

		int clientfd, serverfd;
		int port_proxy;
		int port_server;
		char* host;
		char buf[MAXLINE];
		rio_t rio;

		port_proxy = atoi(argv[1]);

		struct sockaddr_in clientaddr;
		int clientlen = sizeof(clientaddr);
		int listenfd = Open_listenfd(port_proxy);

		int proxy_fd;

		pthread_t tid;

		while(1)
		{
			/*int* connfdp = Malloc(sizeof(int));
			struct hostent* hp;
			char* haddrp;
			int client_port;
			char* msg;
			rio_t rio;

			*connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen);

			// Establish connection.
			hp = Gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
			haddrp = inet_ntoa(clientaddr.sin_addr);
			client_port = ntohs(clientaddr.sin_port);
			//printf("Proxy Server connected to %s (%s), port %d\n", hp->h_name, haddrp, client_port);
			
			printf("DEBUG --- before Rio_readn_w\n");
			Rio_readn_w(*connfdp, buf, MAXLINE);
			printf("Proxy received from client : %s\n", buf);

			// Send to server.
			//clientaddr.sin_addr.s_addr = get_host(buf);
			port_server = atoi(get_port(buf));
			msg = get_message(buf);

			proxy_fd = Open_clientfd(get_host(buf), port_server);
			Rio_readinitb(&rio, proxy_fd);

			Rio_writen(proxy_fd, msg, strlen(msg));
			Rio_readlineb(&rio, msg, MAXLINE);
			printf("Proxy received from server : %s\n", msg);

			// Retrun to client. */
			int* connfd = (int*)malloc(sizeof(int));
			*connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
			Pthread_create(&tid, NULL, thread, connfd);

		}



    exit(0);
}

void* thread(void* vargp)
{
	Pthread_detach(pthread_self());
	
	int connfd = *((int*)vargp);

	Free(vargp);

	char buf[MAXLINE];
	Rio_readn_w(connfd, buf, MAXLINE);

	char* host; int port_server; char* msg; 
	rio_t rio;
	host = get_host(buf);
	port_server = atoi(get_port(buf));
	msg = get_message(buf);
	char* msg_received = (char*)malloc(sizeof(char) * strlen(msg));

	int proxy_fd = Open_clientfd(host, port_server);
	Rio_readinitb(&rio, proxy_fd);
	
	Rio_writen_w(proxy_fd, msg, strlen(buf));
	Rio_readlineb_w(&rio, msg_received, MAXLINE);

	close(proxy_fd);

	Rio_writen_w(connfd, msg_received, strlen(msg_received));

	Close(connfd);

	return NULL;
}



int open_clientfd_ts(char* hostname, int port, sem_t* mutexp)
{
	int clientfd;
	int len = 16;
	int ghn_res = -1;
	int err;
	struct hostent* hp;
	struct hostent hbuf;
	struct sockaddr_in serveraddr;
	char* buf = (char*)malloc(sizeof(char) * len);

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}

	while ((ghn_res = gethostbyname_r(hostname, &hbuf, buf, len, &hp, &err)) == ERANGE)
	{
		len = len * 2;
		buf = realloc(buf, len);
		if (NULL == buf)
		{
			perror("realloc");
		}
	}

	if (hp == NULL)
	{
		return -2;
	}

	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char*)hp->h_addr_list[0], (char*)&serveraddr.sin_addr.s_addr, hp->h_length);
	serveraddr.sin_port = htons(port);

	if (connect(clientfd, (SA*)&serveraddr, sizeof(serveraddr)) < 0)
	{
		return -1;
	}

	return clientfd;
}

ssize_t Rio_readn_w(int fd, void* ptr, size_t nbytes)
{
	ssize_t n;
	
	if ((n = rio_readn(fd, ptr, nbytes)) < 0)
	{
		fprintf(stderr, "%s: %s\n", "Rio_readn_w error", strerror(errno));
		return 0;
	}
	return n;
}

ssize_t Rio_readlineb_w(rio_t* rp, void* usrbuf, size_t maxlen)
{
	ssize_t rc;
	if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
	{
		fprintf(stderr, "%s: %s\n", "Rio_readlineb error", strerror(errno));
		return 0;
	}
	return rc;
}

void Rio_writen_w (int fd, void* usrbuf, size_t n)
{
	if (rio_writen(fd, usrbuf, n) != n)
	{
		fprintf(stderr, "%s: %s\n", "Rio_writen error", strerror(errno));
	}
	return;
}


