#include "csapp.h"

char* ctos(char c)
{
	char* res = (char*)malloc(sizeof(char) * 2);
	res[0] = c;
	res[1] = '\0';
	//printf("ctos - res : %s\n", res);
	return res;
}

int host_checker (char* buf)
{
	printf("buf : %s\n", buf);
	if (buf == NULL || strcmp(buf, "\n") == 0)
	{
		return -1;
	}
	else
	{
		int cnt = 0;
		int len = strlen(buf);
		char res[128];
		strcpy(res, "");
		int dot_cnt = 0;
		for (cnt = 0; cnt < len && cnt < 15; cnt++)
		{
			//printf("res : %s, buf[cnt] : %s\n", res, ctos(buf[cnt]));
			if (buf[cnt] != ' ' && buf[cnt] != '\n')
			{
				char* tmp = ctos(buf[cnt]);
				strcat(res, tmp);
				//printf("after strcat\n");
			}
			else
			{
				break;
			}

			if (buf[cnt] == '.')
			{
				dot_cnt++;
			}
		}

		printf("res - host_checker : %s\n", res);

		if (strcmp(res, "localhost") == 0)
		{
			printf("res is localhost!\n");
			return 1;
		}
		else
		{
			printf("res is not localhost!\n");
			if (dot_cnt == 3)
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
	}
}

int port_checker (char* buf)
{
	printf("buf(port_checker) : %s\n", buf);
	if (buf == NULL || strcmp(buf, "\n") == 0)
	{
		return -1;
	}
	else
	{
		int cnt = 0;
		int space_cnt = 0;
		char res[128];
		strcpy(res, "");
		int len = strlen(buf);
		for(cnt = 0; cnt < len && cnt < 21; cnt++)
		{
			printf("cnt : %d, res : %s\n", cnt, res);
			if (space_cnt == 1)
			{
				if (buf[cnt] == '0' || buf[cnt] == '1' || buf[cnt] == '2' || buf[cnt] == '3' || buf[cnt] == '4' || buf[cnt] == '5' || buf[cnt] == '6' || buf[cnt] == '7' || buf[cnt] == '8' || buf[cnt] == '9')
				{
					char* temp = ctos(buf[cnt]);
					strcat(res, temp);
				}
				else
				{
					break;
				}
			}
			if (buf[cnt] == ' ')
			{
				space_cnt++;
			}
		}

		printf("res - port_checker : %s\n", res);

		if (res != NULL || strcmp(res, "") != 0)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

}

int main(int argc, char** argv)
{
	char* buf = "255.255.255.255 12345 Hello, World!";
	printf("buf : %s\n", buf);
	int host_res = host_checker(buf);
	int port_res = port_checker(buf);
	printf("host_check : %d, port_check : %d\n", host_res, port_res);
	return 0;
}
