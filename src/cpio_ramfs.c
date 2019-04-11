// !c++
//      ------------------------------
//              MIT-License 0x7e3
//       <ergotamin.source@gmail.com>
//      ------------------------------
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <archive.h>
#include <archive_entry.h>

char **files = NULL;

void free_files(void)
{
    free((void *)files);
    files = NULL;
}

char *append(const char *fname)
{
    return strdup(fname);
}


static int
__extract_file(struct archive *ar, struct archive *aw)
{
    int r;
    size_t size;
    const void *block;
    int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &block, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r != ARCHIVE_OK)
            exit(1);
        r = (int)archive_write_data_block(aw, block, size, offset);
        if (r != ARCHIVE_OK)
            return r;
    }
}

static int write_ramfs(void)
{
    DIR *dir;
    unsigned int n;
    struct dirent *dent;

    char ofile[128];
    char **files = (char **)calloc(1, sizeof(char **));

    time_t t = time(NULL);
    struct tm *now = localtime(&t);

    strftime(ofile, sizeof(ofile), "ramdisk.img-%M-%m-%y", now);

    dir = opendir(".");

    if (dir) {
        dent = readdir(dir);

        for (n = 0; dent != NULL; dent = readdir(dir)) {
            if (!(strcmp(".", dent->d_name))
                || !(strcmp("..", dent->d_name)))
                continue;
            else
                files[n++] = append(dent->d_name);
        }
        files[n] = NULL;
        closedir(dir);
    } else {
        perror("could not open directory.\n");
        exit(EXIT_FAILURE);
    }

    n = 0;

    struct archive *a = archive_write_new();
    struct archive_entry *entry;
    struct stat st;
    char buff[8192];
    int len;
    int fd;

    archive_write_add_filter_gzip(a);
    archive_write_set_format_by_name(a, "newc");
    archive_write_open_filename(a, ofile);

    while (files[n]) {
        entry = archive_entry_new();

        stat(files[n], &st);

        archive_entry_set_pathname(entry, files[n]);
        archive_entry_copy_stat(entry, &st);
        archive_entry_set_uid(entry, 0);
        archive_entry_set_gid(entry, 0);
        archive_write_header(a, entry);

        fd = open(files[n], O_RDONLY);
        len = read(fd, buff, sizeof(buff));

        while (len > 0) {
            archive_write_data(a, buff, len);
            memset(buff, 0, sizeof(buff));
            len = read(fd, buff, sizeof(buff));
        }

        close(fd);
        memset(buff, 0, sizeof(buff));
        archive_entry_free(entry);

        n++;
    }
    archive_write_close(a);
    free_files();
    return archive_write_free(a);
}

static int extract_ramfs(const char *imgfile)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    char *dest = strdup(imgfile);
    char *tr = strrchr(dest, '.');
    int flags;
    int r;

    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
    flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    flags |= ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS;

    a = archive_read_new();
    archive_read_support_format_cpio(a);
    archive_read_support_filter_gzip(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, imgfile, 10240)))
        exit(0);

    *tr = '\0';
    mkdir(dest, 0755);

    if (0 != chdir(dest)) {
        free((void *)dest);
        perror("could not change to working directory.\n");
        exit(EXIT_FAILURE);
    }

    free((void *)dest);

    for (;;) {
        r = archive_read_next_header(a, &entry);

        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN)
            exit(1);

        archive_entry_set_uid(entry, 0);
        archive_entry_set_gid(entry, 0);

        r = archive_write_header(ext, entry);

        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
        } else if (archive_entry_size(entry) > 0) {
            r = __extract_file(a, ext);

            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                exit(1);
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    r = archive_write_free(ext);

    return r;
}

int ramfs(int argc, char **argv)
{
    --argc;
    if (argc) {
        if (!strcmp(*argv, "pack"))
            return write_ramfs();
        if (!strcmp(*argv, "unpack")) {
            --argc;
            argv++;
            if (!argc) {
                perror("missing file parameter.\n");
                return 1;
            } else {
                if (!access(*argv, F_OK)) {
                    return extract_ramfs(*argv);
                } else {
                    perror("no such file.\n");
                    return 1;
                }
            }
        }
    }
    printf("Usage:"
           "\tramfs [pack (PATH)|unpack FILE]\n"
           "\t\tpack         - packs a ramfs from current dir.\n"
           "\t\tunpack FILE  - unpacks FILE.\n"
           "\t\t                   (ONLY ramfs-files.)\n"
           );
    return 1;
}
