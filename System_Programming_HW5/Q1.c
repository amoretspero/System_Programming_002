#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

pid_t pid;
int counter = 1;

void handler1(int sig)
{
    counter = 5;
    printf("%d", counter);
    fflush(stdout);
    exit(0);
}

int main()
{
    signal(SIGUSR1, handler1);
    printf("%d", counter);
    fflush(stdout);

    if((pid = fork()) == 0)
    {
	while(1){};
    }
    kill(pid, SIGUSR1);
    waitpid(-1, NULL, 0);
    counter = counter + 1;
    printf("%d%d", counter, counter);
    exit(0);
}
