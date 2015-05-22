#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd = open(argv[2], O_WRONLY);
    dup2(fd, STDOUT_FILENO);
    close(fd);
    execl(argv[1], argv[1], NULL);
    
    return 0;
}
