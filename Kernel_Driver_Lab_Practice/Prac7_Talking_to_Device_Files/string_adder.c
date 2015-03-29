#include <stdio.h>
#include <stdlib.h>

char* string_adder(char* str1, char* str2)
{
    int len1 = 0;
    int len2 = 0;
    while(str1[len1] != '\0')
    {
	len1++;
    }
    while(str2[len2] != '\0')
    {
	len2++;
    }
    printf("length of str1 : %d, length of str2 : %d\n", len1, len2);
    int total_len = len1+len2;
    int cnt;
    char* res = (char*)malloc(sizeof(char)*(total_len+1));
    for(cnt = 0; cnt < len1; cnt++)
    {
	res[cnt] = str1[cnt];
    }
    for(cnt = 0; cnt < len2; cnt++)
    {
	res[cnt + len1] = str2[cnt];
    }
    res[total_len] = '\0';
    return res;
}

char* int_to_str(int num)
{
    char* res = "";
    while(num > 9)
    {
	char* temp = (char*)malloc(sizeof(char));
	*temp = (num%10)+48;
	res = string_adder(res, temp);
	num = num/10;
    }
    char* last_digit = (char*)malloc(sizeof(char));
    *last_digit = num+48;
    res = string_adder(res, last_digit);
}

int main()
{
    char* test_str1 = "Hello,";
    char* test_str2 = " World!";
    char* helloworld_str = string_adder(test_str1, test_str2);
    printf("\"%s\" + \"%s\" = \"%s\"\n", test_str1, test_str2, helloworld_str);
    printf("777 is %s\n", int_to_str(777));
    return 0;
}
