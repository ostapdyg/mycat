#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>


static int ERRNO_buf;

ssize_t my_read(int fd, char *buf, size_t count)
{
    size_t to_read = count;
    while (to_read)
    {
        ssize_t res = read(fd, buf, to_read);
        switch (res)
        {
        case 0:
            *buf = 0;
            return count - to_read;
        case -1:
            if (errno != EINTR)
            {
                ERRNO_buf = errno;
                return -1;
            }
            break;
        default:
            to_read -= res;
            buf += res;
        }
    }
    *buf = 0;
    return count - to_read;
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

size_t my_strlen(const char *s)
{
    const char *p;
    for (p = s; *p != 0; p++)
        ;
    return (p - s);
}

inline int my_print(int fd, const char *s)
{
    return my_write(fd, s, my_strlen(s));
}



char to_hex_chr(char i)
{   
    i &= 0xf;
    return i + '0' + ('A'-'9'-1)*(!!(i > 9));
}

char* to_hex(char i){
    static char hexcode[] = "\\x00";
    hexcode[2] = to_hex_chr(i >> 4);
    hexcode[3] = to_hex_chr(i);
    return hexcode;
}

int my_write_fmt(int fd, const char *buf, size_t count)
{
    const char *b_s = buf;
    size_t bsize = 0;
    char hexcode[] = "\\x00";
    while (count)
    {
        while (isprint(b_s[bsize]) || isspace(b_s[bsize]))
            bsize++;
        my_write(fd, b_s, bsize);        
        count -= bsize;
        if(!count){
            return 0;
        }
        my_write(fd, to_hex(b_s[bsize]), sizeof(hexcode) - 1);
        count -= 1;
        b_s = b_s + bsize + 1;
        bsize=0;
    }
    return my_write(fd, buf, count);
    return 0;
}

int __my_cat_inner(int *fdp_s, int filenum, int (*out_func)(int, const char *, size_t));

int my_cat(const char **f_s, int filenum, int A_f)
{
    int *fdp_s = (int *)malloc(filenum*sizeof(int*));
    int *fdp_e;

    const char **filename;
    int ret = 0;

    for (filename = f_s, fdp_e = fdp_s; *filename; filename++, fdp_e++)
    {
        if ((*fdp_e = open(*filename, O_RDONLY)) == -1)
        {
            my_print(STDERR_FILENO, "mycat: ");
            my_print(STDERR_FILENO, *filename);
            my_print(STDERR_FILENO, ": No such file or directory\n");
            ret = errno;
        }
    }

    if (!ret)
    {
        if (A_f)
        {
            __my_cat_inner(fdp_s, filenum, my_write_fmt);
        }
        else
        {
            __my_cat_inner(fdp_s, filenum, my_write);
        }
    }


    for (int *fdp = fdp_s; fdp < fdp_e; fdp++)
    {
        if (*fdp != -1)
        {
            close(*fdp);
        }
    }
    free(fdp_s);
    return ret;
}

int __my_cat_inner(int *fdp_s, int filenum, int (*out_func)(int, const char *, size_t))
{
    static char buf[255];
    ssize_t r;

    for (int *fdp = fdp_s; fdp - fdp_s < filenum; fdp++)
    {

        while ((r = my_read(*fdp, buf, sizeof(buf))))
        {
            if (r == -1)
            {
                return ERRNO_buf;
            }
            if ((r = (*out_func)(STDOUT_FILENO, buf, r)))
            {
                return r;
            }
        }
    }
    return 0;
}

#include <stdio.h>

static const char *help_str =
    "Usage: mycat [-h|--help] [-A] <file1> <file2> ... <fileN>\n\
Concatenate file(s) to standart input.\n\
-A            show invisible non-whitespace characters as their hexidecimal codes \\xAB\n\
-h, --help      show this message and exit\n";

int main(int argc, char **argv)
{
    int opt;
    int h_f = 0;
    int A_f = 0;

    struct option long_opts[] =
        {{"help", no_argument, 0, 'h'},
         {0, 0, 0, 0}};

    char **filenames = (char **)malloc(argc*sizeof(char*)); //Is enough

    int filenum = 0;
    while ((opt = getopt_long(argc, argv, ":hA", long_opts, 0)) != -1)
    {
        switch (opt)
        {
        case 'h':
            h_f = 1;
            break;
        case 'A':
            A_f = 1;
            break;
        case '?':
        default: //Not sure if those shuld be interpreted as filenames
            // filenames[filenum] = argv[optind - 1];
            // filenum += 1;
            my_print(STDOUT_FILENO, "Unrecognized command line option:");
            my_print(STDOUT_FILENO, argv[optind - 1]);
            my_print(STDOUT_FILENO, "\n");
            my_print(STDOUT_FILENO, help_str);
            return -1;
        }
    }
    if (h_f)
    {
        my_print(STDOUT_FILENO, help_str);
        return 0;
    }

    for (; optind < argc; optind++)
    {

        filenames[filenum] = argv[optind];
        filenum += 1;
    }
    filenames[filenum] = 0;

    int err;
    if ((err = my_cat((const char **)filenames, filenum, A_f)))
    {
        my_print(STDERR_FILENO, "mycat: Non-zero return code:");
        my_print(STDERR_FILENO, to_hex(err));
        my_print(STDERR_FILENO, "\n");

        return err;
    }
    free(filenames);
    return 0;
}
