#include <sys/stat.h>
#include <sys/types.h>


#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "operations/operations.hpp"

int my_read(int fd, void *buf, size_t count)
{

    while (count)
    {
        int res = read(fd, buf, count);
        if (res == -1)
        {
            if (errno != EINTR)
            {
                return errno;
            }
        }
        else
        {
            count -= res;
            buf += res;
        }
        *(char *)(buf) = 0;
    }
    return 0;
}

int my_write(int fd, const void *buf, size_t count)
{
    while (count)
    {
        int res = write(fd, buf, count);
        if (res == -1)
        {
            if (errno != EINTR)
            {
                return errno;
            }
        }
        else
        {
            count -= res;
            buf += res;
        }
    }
    return 0;
}

size_t my_strlen(const char* s){
    const char* p;
    for(p=s; *p !=0; p++);
    // printf("Size: %d\n", (p-s));
    return (p-s);
}


int my_print(int fd, const char* s){
    return my_write(fd, s, my_strlen(s));
}



int main(int argc, char **argv)
{
    int input_file = open("samples/book1.txt", O_RDONLY);
    // // // int output_file = open("stdout", O_WRONLY);
    // // char *buf[100];
    // // int res = my_read(input_file, buf, 5);
    // // // buf[29] = 0;
    // // res = my_write(STDIN_FILENO, buf, 5);
    // // close(input_file);
    // // // printf("%s\n", buf); 
    // my_print(STDIN_FILENO, "Hello, world!\n");



    return 0;
}
