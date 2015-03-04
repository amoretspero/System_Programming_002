int sum1(int a, int b, int c, int d, int e, int f, int g, int h)
{
	int s = a;
	s += b;
	s += c;
	s += d;
	s += e;
	s += f;
	s += g;
	s += h;

	return s;
}

int main(int argc, char** argv)
{
	int res = sum1(1, 2, 3, 4, 5, 6, 7, 8);
	return 0;
}