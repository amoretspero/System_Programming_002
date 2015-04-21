#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void h1(int sig)
{
    printf("Signal Handler #1 - Sig : %d\n", sig);

}

int main(int argc, char** argv)
{
    signal(SIGSEGV, h1);
    while(1)
    {
	printf("Returned to main function.\n");
	int *p = 0;
	*p = 3;
    }
    return 0;
}
