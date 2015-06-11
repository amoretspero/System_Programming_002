#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
	int fd1;
	int s = getpid() & 0x1;
	char c1, c2;
	char* fname = argv[1];
	fd1 = open(fname, O_RDONLY, 0);
	read(fd1, &c1, 1);
	if (fork())
	{
		sleep(s);
		read(fd1, &c2, 1);
		printf("Parent : c1 = %c, c2 = %c\n", c1, c2);
	}
	else
	{
		sleep(1-s);
		read(fd1, &c2, 1);
		printf("Child : c1 = %c, c2 = %c\n", c1, c2);
	}
	return 0;
}
	
