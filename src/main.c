// !c++
//      ------------------------------
//              MIT-License 0x7e3
//       <ergotamin.source@gmail.com>
//      ------------------------------
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//
extern const char *__progname;
extern int ramfs(int argc, char **argv);
extern int simg2img(int argc, char **argv);
extern int img2simg(int argc, char **argv);
extern int mkbootimg(int argc, char **argv);
extern int unpackbootimg(int argc, char **argv);
//
unsigned int get_opt(const char *arg, int num, ...)
{
    int r = -1;
    va_list args;
    char *def = NULL;
    unsigned int opt = 0;

    va_start(args, num);
    ;
    for (--num; 0 <= num; num--) {
        ++opt;
        ;
        def = strdup(va_arg(args, const char *));
        r = strcmp(arg, def);
        free((void *)def);
        ;
        def = NULL;
        ;
        if (0 == r)
            break;
    }
    ;
    va_end(args);

    if (num == -1)
        return (unsigned int)num;
    else
        return opt;
}
//
int main(int argc, char **argv)
{
    --argc;
    argv++;
    int r = -1;
    if (argc) {
        unsigned int opt = get_opt(*argv, 5,
                                   "mkbootimg",
                                   "unpackbootimg",
                                   "img2simg",
                                   "simg2img",
                                   "ramfs");
        switch (opt) {
        case 1:
            argv++;
            r = mkbootimg(argc, argv);
            break;

        case 2:
            argv++;
            r = unpackbootimg(argc, argv);
            break;

        case 3:
            argv++;
            r = img2simg(argc, argv);
            break;

        case 4:
            argv++;
            r = simg2img(argc, argv);
            break;

        case 5:
            argv++;
            r = ramfs(argc, argv);
            break;

        default:
            break;
        }
    } else {
        printf("Usage:\n"
               "     %s [mkbootimg|unpackbootimg|img2simg|simg2img|ramfs]\n"
               "        - calls an anroid-image-tool.\n",
               __progname);
    }
    return r;
}
