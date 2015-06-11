#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* t_routine (void* vargp)
{
	printf("Peer thread started!\n");
	//sleep(1);
	printf("Hello world from peer!\n");
	exit(0);
	return NULL;
}

int main(int argc, char** argv)
{
	printf("Main thread started!\n");

	pthread_t pid;
	pthread_create(&pid, NULL, t_routine, NULL);
	sleep(1);
	printf("After 1 sec - main thread\n");
	return 0;
	//pthread_join(pid, NULL);
	//exit(0);
}

