#include <sys/stat.h>
#include <sys/types.h>


#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>

int my_read(int fd, char *buf, size_t count)
{

    while (count)
    {
        int res = read(fd, buf, count);
        switch(res){
            case 0:
                *buf=0;
                return 0;
            case -1:
                if (errno != EINTR)
                {
                    return errno;
                }
                break;
            default:
                count -= res;
                buf += res;
        }
    }
    *buf = 0;
    return count;
}

int my_write(int fd, const char *buf, size_t count)
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

int my_write_fmt(int fd, const char* buf, size_t count){
    return my_write(fd, buf, count);
    return 0;
}


int my_cat(const char** f_s, int filenum, int A_f){
    int* fdp_s = (int*)malloc(filenum); 
    int* fdp_e;
    int fd;
    const char** f_e;
    int ret = 0;
    static char buf[255];

    for(f_e = f_s, fdp_e = fdp_s; *f_e; f_e++, fdp_e++){
        if((*fdp_e = open(*f_e, O_RDONLY)) == -1){
            my_print(STDERR_FILENO, "mycat: ");
            my_print(STDERR_FILENO, *f_e);
            my_print(STDERR_FILENO, ": No such file or directory\n");
            ret=-1;
        }
    }
    if(!ret){
        for(int *fdp=fdp_s; fdp<fdp_e; fdp++){
            
        }
    }
    for(int *fdp=fdp_s; fdp<fdp_e; fdp++){
        if(*fdp!=-1){
            close(*fdp);
        }
    }
    free(fdp_s);
    return ret;
} 



#include <stdio.h>

static const char* help_str=\
"Usage: mycat [-h|--help] [-A] <file1> <file2> ... <fileN>\n\
Concatenate file(s) to standart input.\n\
-A            show invisible non-whitespace characters as their hexidecimal codes \\xAB\n\
-h, --help      show this message and exit\n";


int main(int argc, char **argv)
{
    int opt;
    int h_f = 0;
    int A_f = 0;

    struct option long_opts[] =\
    {{"help", no_argument, 0, 'h'},
     {0,0,0,0}
    };

    char** filenames = (char**)malloc(argc);  //Is enough
    int filenum=0;
    while((opt=getopt_long(argc, argv, ":hA", long_opts, 0)) != -1){
        switch(opt){
            case 'h':
                h_f=1;
                // my_print(STDOUT_FILENO, "Option: h:\n");s
                break;
            case 'A':
                A_f=1;
                // my_print(STDOUT_FILENO, "Option: A:\n");
                break;
            case '?':
            default:                            //Not sure if those shuld be interpreted as filenames
                filenames[filenum]=argv[optind-1];
                filenum += 1;
                // my_print(STDOUT_FILENO, "Filename: ");
                // my_print(STDOUT_FILENO, argv[optind-1]);
                // my_print(STDOUT_FILENO, "\n");

        }
    }

    if(h_f){
        my_print(STDOUT_FILENO, help_str);
        exit(EXIT_SUCCESS);
    }

    for(;optind<argc;optind++){
        // my_print(STDOUT_FILENO, "Filename: ");
        // my_print(STDOUT_FILENO, argv[optind]);
        // my_print(STDOUT_FILENO, "\n");
        filenames[filenum]=argv[optind];
        filenum += 1;
    }
    filenames[filenum]=0;
    
    my_print(STDOUT_FILENO, "Filenames: ");
    for(char** f = filenames;*f;f++){
        my_print(STDOUT_FILENO, *f);
        my_print(STDOUT_FILENO, ";");
    }
    my_print(STDOUT_FILENO, "\n");

    my_cat(filenames, filenum, A_f);
    free(filenames);
    return 0;
}
