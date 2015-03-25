#include <stdio.h>

int test = 1000;

void print_anything();
void print_final();

void print_something ()
{
    print_anything();
    printf("Hello, World!\n");
    print_final();
}
