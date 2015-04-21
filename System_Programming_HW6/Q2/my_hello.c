int main(int argc, char** argv)
{
    int len;
    char buf[10];
    my_write(1, "What is your student ID? :\n", 27);
    len = my_read(2, buf, 10);
    my_write(1, "SID : ", 6);
    my_write(1, buf, len);
    my_exit(0);
}
