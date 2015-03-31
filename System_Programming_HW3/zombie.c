#include <unistd.h>

void main(void)
{
    /*pid_t pid[10];

    int i;

    for (i = 9; i >= 0;--i)
    {
	pid[i] = fork();
	if (pid[i] == 0)
	{
	    sleep(i+1);
	    _exit(0);
	}
	printf("pid%d : %d\n", i, pid[i]);
    }

    for(i = 9; i >= 0; --i)
    {
	waitpid(pid[i], NULL, 0);
    }*/

    pid_t pid1, pid2;

    pid1 = fork();
    if (pid1 == 0)
    {
	sleep(10);
	_exit(0);
    }
    pid2 = fork();
    if (pid2 == 0)
    {
	sleep(5);
	_exit(0);
    }
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return;
}    
