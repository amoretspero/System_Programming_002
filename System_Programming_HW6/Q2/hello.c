#include <stdlib.h>

int main(int argc, char** argv)
{
    int len;
    char buf[10];
    write(1, "What is your student ID? :\n", 27);
    len = read(2, buf, 10);
    write(1, "SID : ", 6);
    write(1, buf, len);
    exit(0);
}
