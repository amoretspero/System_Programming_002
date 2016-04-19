#include "csapp.h"

/* Destructively modify string to be upper case */
void upper_case(char *s)
{
  while (*s) {
    *s = toupper(*s);
    s++;
  }
}

int bytecnt = 0;

void echo(int connfd, char *prefix) 
{
    size_t n; 
    char buf[MAXLINE]; 
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	bytecnt += n;
	//printf("buf : %d %d %d %d %d\n", (int)buf[0], (int)buf[1], (int)buf[2], (int)buf[3], (int)buf[4]);
	printf("%sreceived %d bytes (%d total)\n", prefix, (int) n, bytecnt);
	upper_case(buf);
	Rio_writen(connfd, buf, n);
    }
}
