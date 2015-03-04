#include <stdlib.h>

int sum2(int a, int b, int c, int d, int e, int f, int g, int h)
{
	int s1 = a + b;
	int s2 = c + d;
	int s3 = e + f;
	int s4 = g + h;

	s1 = s1 + s2;
	s3 = s3 + s4;
	s1 = s1 + s3;

	return s1;
}

int main(int argc, char** argv)
{
	int res = sum2(1, 2, 3, 4, 5, 6, 7, 8);
	system("pause");
	return 0;
}