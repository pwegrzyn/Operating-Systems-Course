/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE 500

#define _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED || _POSIX_C_SOURCE >= 200112L

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <ftw.h>

// Represents the date relation
enum time_diff {EARLIER, LATER, SAME};

// Global variables required to parametrize the mapping function passed to nftw(3)
time_t ref_date_global;
enum time_diff date_relation_global;

// Helper function for printing errors;
void sig_arg_err(void)
{
    printf("Wrong argument format.\n"
           "Usage: program [directory] [<|>|=] [date] [nftw|no-nftw]\n"
           "[date] field in format: Mar 19 12:48:31 2009\n");
    exit(1);
}

// Used for prepending to a string
void prepend(char *str, const char *pre)
{
    size_t len = strlen(pre);

    memmove(str + len, str, strlen(str) + 1);

    for (size_t i = 0; i < len; i++)
        str[i] = pre[i];
}

// Finds and prints to stdout all normal files found in a given directory
// which have a given modification time frame. Prints the size in bytes,
// non-relative path, access rights (in ls -l format) and the date of the last
// modification. Uses functions likes opendir, readdir, and stat.
void find_files_no_nftw(const char *dir_path, time_t ref_date, enum time_diff date_relation)
{
    DIR *dir;
    struct dirent *entry;
    char file_name[1024];
    char full_path[1024];
    char *last_mod_date;
    char rights[10] = "";
    char link_path[1024];
    int size, lnerr;
    struct stat stat_buf;
    struct stat stat_buf_tmp;

    dir = opendir(dir_path);
    if(dir == NULL) return;

    while((entry = readdir(dir)) != NULL)
    {
        strcpy(file_name, dir_path);
        strcat(file_name, "/");
        strcat(file_name, entry->d_name);
        lstat(file_name, &stat_buf);
        if(lstat(file_name, &stat_buf) == -1) continue;
        if(S_ISREG(stat_buf.st_mode))
        {
            if((date_relation == EARLIER && (difftime(ref_date, stat_buf.st_mtime) > 0))
              || (date_relation == LATER && (difftime(ref_date, stat_buf.st_mtime) < 0))
              || (date_relation == SAME && (difftime(ref_date, stat_buf.st_mtime) == 0)))
            {
                realpath(file_name, full_path);
                size = stat_buf.st_size;
                last_mod_date = ctime(&(stat_buf.st_mtime));
                if(stat_buf.st_mode & S_IRUSR)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IWUSR)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IXUSR)
                    strcat(rights, "x");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IRGRP)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IWGRP)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IXGRP)
                    strcat(rights, "x");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IROTH)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IWOTH)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf.st_mode & S_IXOTH)
                    strcat(rights, "x");
                else strcat(rights, "-");

                printf("Found file:\n%s\nSize: %d bytes\n"
                       "Access rights: %s\nLast modification date: %s\n",
                       full_path, size, rights, last_mod_date);

                rights[0] = '\0';
            }

        }
        else if(S_ISDIR(stat_buf.st_mode))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char tmp_path[1024];
            snprintf(tmp_path, sizeof(tmp_path), "%s/%s", dir_path, entry->d_name);
            find_files_no_nftw(tmp_path, ref_date, date_relation);
        }
        else if(S_ISLNK(stat_buf.st_mode))
        {
            lnerr = readlink(file_name, link_path, 1024);
            if(lnerr == -1) continue;
            else if(lstat(link_path, &stat_buf_tmp) == -1) continue;
            else if(S_ISREG(stat_buf_tmp.st_mode))
            {
                if((date_relation == EARLIER && (difftime(ref_date, stat_buf_tmp.st_mtime) > 0))
                   || (date_relation == LATER && (difftime(ref_date, stat_buf_tmp.st_mtime) < 0))
                   || (date_relation == SAME && (difftime(ref_date, stat_buf_tmp.st_mtime) == 0)))
                {
                    realpath(link_path, full_path);
                    size = stat_buf_tmp.st_size;
                    last_mod_date = ctime(&(stat_buf_tmp.st_mtime));
                    if(stat_buf_tmp.st_mode & S_IRUSR)
                        strcat(rights, "r");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IWUSR)
                        strcat(rights, "w");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IXUSR)
                        strcat(rights, "x");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IRGRP)
                        strcat(rights, "r");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IWGRP)
                        strcat(rights, "w");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IXGRP)
                        strcat(rights, "x");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IROTH)
                        strcat(rights, "r");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IWOTH)
                        strcat(rights, "w");
                    else strcat(rights, "-");
                    if(stat_buf_tmp.st_mode & S_IXOTH)
                        strcat(rights, "x");
                    else strcat(rights, "-");

                    printf("Found file:\n%s\nSize: %d bytes\n"
                           "Access rights: %s\nLast modification date: %s\n",
                           full_path, size, rights, last_mod_date);

                    rights[0] = '\0';
                }
            }
        }
        else continue;
    }
    closedir(dir);
}

// Used as the function which nftw() maps all files in the tree with
static int nftw_mapper(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    char full_path[1024];
    char *last_mod_date;
    char rights[10] = "";
    char link_path[1024];
    int size, lnerr;
    struct stat stat_buf_tmp;

    if(S_ISREG(sb->st_mode))
    {
        if((date_relation_global == EARLIER && (difftime(ref_date_global, sb->st_mtime) > 0))
            || (date_relation_global == LATER && (difftime(ref_date_global, sb->st_mtime) < 0))
            || (date_relation_global == SAME && (difftime(ref_date_global, sb->st_mtime) == 0)))
        {
            realpath(fpath, full_path);
            size = sb->st_size;
            last_mod_date = ctime(&(sb->st_mtime));
            if(sb->st_mode & S_IRUSR)
                strcat(rights, "r");
            else strcat(rights, "-");
            if(sb->st_mode & S_IWUSR)
                strcat(rights, "w");
            else strcat(rights, "-");
            if(sb->st_mode & S_IXUSR)
                strcat(rights, "x");
            else strcat(rights, "-");
            if(sb->st_mode & S_IRGRP)
                strcat(rights, "r");
            else strcat(rights, "-");
            if(sb->st_mode & S_IWGRP)
                strcat(rights, "w");
            else strcat(rights, "-");
            if(sb->st_mode & S_IXGRP)
                strcat(rights, "x");
            else strcat(rights, "-");
            if(sb->st_mode & S_IROTH)
                strcat(rights, "r");
            else strcat(rights, "-");
            if(sb->st_mode & S_IWOTH)
                strcat(rights, "w");
            else strcat(rights, "-");
            if(sb->st_mode & S_IXOTH)
                strcat(rights, "x");
            else strcat(rights, "-");

            printf("Found file:\n%s\nSize: %d bytes\n"
                   "Access rights: %s\nLast modification date: %s\n",
                   full_path, size, rights, last_mod_date);

            rights[0] = '\0';
        }

    }
    else if(S_ISLNK(sb->st_mode))
    {
        lnerr = readlink(fpath, link_path, 1024);
        if(lnerr == -1) return 0;
        else if(lstat(link_path, &stat_buf_tmp) == -1) return 0;
        else if(S_ISREG(stat_buf_tmp.st_mode))
        {
            if((date_relation_global == EARLIER && (difftime(ref_date_global, stat_buf_tmp.st_mtime) > 0))
                || (date_relation_global == LATER && (difftime(ref_date_global, stat_buf_tmp.st_mtime) < 0))
                || (date_relation_global == SAME && (difftime(ref_date_global, stat_buf_tmp.st_mtime) == 0)))
            {
                realpath(link_path, full_path);
                size = stat_buf_tmp.st_size;
                last_mod_date = ctime(&(stat_buf_tmp.st_mtime));
                if(stat_buf_tmp.st_mode & S_IRUSR)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IWUSR)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IXUSR)
                    strcat(rights, "x");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IRGRP)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IWGRP)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IXGRP)
                    strcat(rights, "x");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IROTH)
                    strcat(rights, "r");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IWOTH)
                    strcat(rights, "w");
                else strcat(rights, "-");
                if(stat_buf_tmp.st_mode & S_IXOTH)
                    strcat(rights, "x");
                else strcat(rights, "-");

                printf("Found file:\n%s\nSize: %d bytes\n"
                       "Access rights: %s\nLast modification date: %s\n",
                        full_path, size, rights, last_mod_date);

                rights[0] = '\0';
            }
        }
    }
    return 0;
}

// MAIN function
int main(int argc, char **argv)
{
    if (argc != 5) sig_arg_err();
    const char *date_str, *dir_path_nr;
    time_t ref_date;
    struct tm ref_date_tm;
    char tmp[1024];
    int path_len;
    int use_nftw = -1;
    enum time_diff date_relation;

    if(strcmp(argv[2], "<") == 0)
        date_relation = EARLIER;
    else if(strcmp(argv[2], ">") == 0)
        date_relation = LATER;
    else if(strcmp(argv[2], "=") == 0)
        date_relation = SAME;
    else
        sig_arg_err();
    
    date_relation_global = date_relation;

    if(strcmp(argv[4], "nftw") == 0)
        use_nftw = 1;
    else if(strcmp(argv[4], "no-nftw") == 0)
        use_nftw = 0;
    if(use_nftw == -1) sig_arg_err();

    strcpy(tmp, argv[1]);
    path_len = strlen(tmp);
    if(tmp[0] != '/')
    {
        if((path_len > 2 && tmp[0] != '.' && tmp[1] != '.')
           || (path_len > 1 && tmp[0] != '.'))
           prepend(tmp, "./");
    }
    path_len = strlen(tmp);
    if(tmp[path_len - 1] != '/')
    {
        tmp[path_len] = '/';
        tmp[path_len + 1] = '\0';
        ++path_len;
    }
    dir_path_nr = tmp;

    date_str = argv[3];
    if(strptime(date_str, "%b%n%d%n%T%n%Y", &ref_date_tm) == NULL)
        sig_arg_err();
    ref_date_tm.tm_isdst = -1;
    ref_date = mktime(&ref_date_tm);
    ref_date_global = ref_date;
    if(ref_date == -1 )
    {
        printf("Wrong date format. Exiting...\n");
        exit(1);
    }

    int nftw_flags = 0;
    nftw_flags |= FTW_PHYS;

    if(!use_nftw) find_files_no_nftw(dir_path_nr, ref_date, date_relation);
    else if (nftw(dir_path_nr, nftw_mapper, 16, nftw_flags) != 0)
    {
        fprintf(stderr, "An error occured while traversing the tree "
                "with nftw(): %s", strerror(errno));
        exit(1);
    }

    return 0;
}