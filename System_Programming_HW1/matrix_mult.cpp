#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <string>

#pragma warning(disable : 4996)
using namespace std;

double mat_mul(int size, int* a, int* b, int*c)
{
	clock_t start = clock();
	auto start_time = chrono::high_resolution_clock::now();
	int i, j;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			int sum = 0;
			int cnt = 0;
			for (cnt = 0; cnt < size; cnt++)
			{
				sum += a[i * size + cnt] * b[cnt * size + j];
			}
			c[i * size + j] = sum;
		}
	}
	clock_t end = clock();
	auto end_time = chrono::high_resolution_clock::now();
	return (chrono::duration_cast<chrono::nanoseconds>(end_time - start_time).count());
}

void random_int(int* mat, int size)
{
	int cnt = 0;
	for (cnt = 0; cnt < size; cnt++)
	{
		mat[cnt] = rand();
	}
}

int main(void)
{
	int mat_size=1;
	int size_max = 0;
	int cont = 0;
	printf("continuous = 0, specific = 1, interval = 2\n");
	scanf_s("%d", &cont);
	
	double prev[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	if (cont == 0)
	{
		printf("Insert maximum matrix size : \n");
		scanf_s("%d", &size_max);
		for (mat_size = 1; mat_size <= size_max; mat_size++)
		{
			srand((unsigned int)mat_size);
			int* mat_a;
			int* mat_b;
			int* mat_c;
			mat_a = (int*)malloc(sizeof(int) * mat_size * mat_size);
			mat_b = (int*)malloc(sizeof(int) * mat_size * mat_size);
			mat_c = (int*)malloc(sizeof(int) * mat_size * mat_size);
			random_int(mat_a, mat_size*mat_size);
			random_int(mat_b, mat_size*mat_size);
			random_int(mat_c, mat_size*mat_size);
			double res = mat_mul(mat_size, mat_a, mat_b, mat_c);
			double prev_0 = prev[0];
			double prev_1 = prev[1];
			double prev_2 = prev[2];
			double prev_3 = prev[3];
			double prev_4 = prev[4];
			double prev_5 = prev[5];
			double prev_6 = prev[6];
			double prev_7 = prev[7];
			double prev_8 = prev[8];
			double prev_9 = prev[9];
			prev[0] = prev_1;
			prev[1] = prev_2;
			prev[2] = prev_3;
			prev[3] = prev_4;
			prev[4] = prev_5;
			prev[5] = prev_6;
			prev[6] = prev_7;
			prev[7] = prev_8;
			prev[8] = prev_9;
			prev[9] = res;
			printf("matrix size = %d * %d, Bytes Used : %d(Bytes), Used time per element(ns/elem) : %lf, Ratio to prev10 : %lf\n", mat_size, mat_size, mat_size*mat_size * 4 * 3, res / (mat_size*mat_size), res / ((prev[0] + prev[1] + prev[2] + prev[3] + prev[4] + prev[5] + prev[6] + prev[7] + prev[8] + prev[9]) / 10));
		}
	}
	else if (cont == 1)
	{
		printf("Insert maximum matrix size : \n");
		scanf_s("%d", &size_max);
		mat_size = size_max;
		srand((unsigned int)mat_size);
		int* mat_a;
		int* mat_b;
		int* mat_c;
		mat_a = (int*)malloc(sizeof(int) * mat_size * mat_size);
		mat_b = (int*)malloc(sizeof(int) * mat_size * mat_size);
		mat_c = (int*)malloc(sizeof(int) * mat_size * mat_size);
		random_int(mat_a, mat_size*mat_size);
		random_int(mat_b, mat_size*mat_size);
		random_int(mat_c, mat_size*mat_size);
		double res = mat_mul(mat_size, mat_a, mat_b, mat_c);
		printf("matrix size = %d * %d, Bytes Used : %d(Bytes), Used time per element(ns/elem) : %lf\n", mat_size, mat_size, mat_size*mat_size * 4 * 3, res / (mat_size*mat_size));
	}
	else
	{
		int interval_min, interval_max;
		printf("Insert interval\n");
		scanf_s("%d %d", &interval_min, &interval_max);
		int interval = 0;
		printf("Insert interval_gap\n");
		scanf_s("%d", &interval);
		int cnt;
		FILE* fp;
		fp = fopen("result.txt", "a");
		for (cnt = interval_min; cnt <= interval_max; cnt = cnt + interval)
		{
			mat_size = cnt;
			srand((unsigned int)mat_size);
			int* mat_a;
			int* mat_b;
			int* mat_c;
			mat_a = (int*)malloc(sizeof(int) * mat_size * mat_size);
			mat_b = (int*)malloc(sizeof(int) * mat_size * mat_size);
			mat_c = (int*)malloc(sizeof(int) * mat_size * mat_size);
			random_int(mat_a, mat_size*mat_size);
			random_int(mat_b, mat_size*mat_size);
			random_int(mat_c, mat_size*mat_size);
			double res = mat_mul(mat_size, mat_a, mat_b, mat_c);
			printf("matrix size = %d * %d, Bytes Used : %d(Bytes), Used time per element(ns/elem) : %lf\n", mat_size, mat_size, mat_size*mat_size * 4 * 3, res / (mat_size*mat_size));
			string str = to_string(mat_size) + "," + to_string(res / (mat_size*mat_size)) + "\n";
			fprintf(fp, "%d", mat_size);
			fprintf(fp, "%s", ",");
			fprintf(fp, "%lf", (res / (mat_size*mat_size)));
			fprintf(fp, "%s", "\n");
		}
		fclose(fp);
	}
	system("pause");
	return 0;
}