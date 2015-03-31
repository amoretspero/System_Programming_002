#include <stdio.h>
#include <unistd.h>

int main()
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
	printf("pid : %d of 1st fork - parent\n", pid);
	pid = fork();
	if (pid == 0)
	{
	    printf("pid : %d of 2nd fork - parent\n", pid);
	    printf("Hello, World!\n");
	}
    }
    else
    {
	printf("pid : %d of 1st fork - child\n", pid);
	printf("Hello, World!\n");
    }

    pid = fork();
    if(pid > 0)
    {
	printf("pid : %d of 3rd fork - child\n", pid);
	printf("Hello, World!\n");
    }

    return 0;
}
