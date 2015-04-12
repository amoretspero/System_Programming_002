/*
 *  ioctl.c - the process to use ioctl's to control the kernel module
 *
 *  Until now we could have used cat for input and output.  But now
 *  we need to do ioctl's, which require writing our own process.
 */

/* 
 * device specifics, such as ioctl numbers and the
 * major device file. 
 */
#include "chardev.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */



char* string_formatter(char* str)
{
    int len = 0;
    while(str[len] != '\0')
    {
	len++;
    }
    char* res = (char*)malloc(sizeof(char)*4096);
    int res_cnt = 0;
    int line_cnt = 0;
    int cnt;
    int line_start = 0;
    for(cnt = 0; cnt < len+1; cnt++)
    {
	if ((str[cnt] == '\n')||(str[cnt] == '\0'))
	{
	    int cnt_temp;
	    for(cnt_temp = line_start; cnt_temp <= cnt; cnt_temp++)
	    {
		res[res_cnt] = str[cnt_temp];
		//printf("%c\n", res[res_cnt]);
		res_cnt++;
	    }
	    int cnt_temp1;
	    for(cnt_temp1 = 0; cnt_temp1 <= line_cnt; cnt_temp1++)
	    {
		//printf("cnt_temp : %d\n", cnt_temp1);
		res[res_cnt] = ' ';
		res_cnt++;
	    }
	    line_start = cnt+1;
	    line_cnt++;
	}
    }
    return res;
}


/* 
 * Functions for the ioctl calls 
 */

ioctl_set_msg(int file_desc, char *message)
{
	int ret_val;

	ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);

	if (ret_val < 0) {
		printf("ioctl_set_msg failed:%d\n", ret_val);
		exit(-1);
	}
}

ioctl_get_msg(int file_desc)
{
	int ret_val;
	char message[1024];

	/* 
	 * Warning - this is dangerous because we don't tell
	 * the kernel how far it's allowed to write, so it
	 * might overflow the buffer. In a real production
	 * program, we would have used two ioctls - one to tell
	 * the kernel the buffer length and another to give
	 * it the buffer to fill
	 */
	ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);

	if (ret_val < 0) {
		printf("ioctl_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("\n%s\n", string_formatter(message));
}

ioctl_get_nth_byte(int file_desc)
{
	int i;
	char c;

	printf("get_nth_byte message:");

	i = 0;
	do {
		c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);

		if (c < 0) {
			printf
			    ("ioctl_get_nth_byte failed at the %d'th byte:\n",
			     i);
			exit(-1);
		}

		putchar(c);
	} while (c != 0);
	putchar('\n');
}


/* 
 * Main - Call the ioctl functions 
 */
main()
{
	int file_desc, ret_val;
	char *msg = "Live today as if there is no tomorrow.\n";

	file_desc = open(DEVICE_FILE_NAME, 0);
	if (file_desc < 0) {
		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}

	printf("=============================================\n");
	ioctl_set_msg(file_desc, msg);
	ioctl_get_msg(file_desc);
	//ioctl_get_nth_byte(file_desc);
	printf("=============================================\n");

	close(file_desc);
}
