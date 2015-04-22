#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_BUF_SIZE 1024

int main(void)
{
    int fd;
    fd = open("SP.txt", O_RDWR);
    char* file_addr;
    struct stat stat_res;
    int fstat_res = fstat(fd, &stat_res);
    char* temp_buf;
    int cnt = 0;
    file_addr = mmap(NULL, stat_res.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, (off_t)0);
    for (cnt = 0; (cnt < stat_res.st_size)&&(cnt < MAX_BUF_SIZE); cnt++)
    {
	if (file_addr[cnt] == ' ')
	{
	    if (file_addr[cnt+1] == 'h')
	    {
		if (file_addr[cnt+2] == 'a')
		{
		    if (file_addr[cnt+3] == 't')
		    {
			if (file_addr[cnt+4] == 'e')
			{
			    if (file_addr[cnt+5] == ' ')
			    {
				file_addr[cnt+1] = 'l';
				file_addr[cnt+2] = 'o';
				file_addr[cnt+3] = 'v';
				file_addr[cnt+4] = 'e';
			    }
			}
		    }
		}
	    }
	}
    }
    //ssize_t written_bytes = write(fd, file_addr, stat_res.st_size);
    return 0;
}
