/*****************************
 *    Project Name:LS
 *    Author:V.Koroy
 *  Вс ноя 15 15:15:43 MSK 2020
 *****************************/
//   gcc -std=c11  -o simple_ls ls.c

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define FLD_CURRENT "."
#define FLD_PREV ".."
#define MAX_PATH 1024

// file mode to string presentation
void file_mode_to_str(char* modes, struct stat fileattrib)
{
    strcpy(modes, "-");

    int fileMode = fileattrib.st_mode;

    if (fileMode & __S_IFDIR)
        strcpy(modes, "d");
    else if (S_ISLNK(fileattrib.st_mode))
        strcpy(modes, "l");

    if ((fileMode & S_IRUSR) && (fileMode & __S_IREAD))
        strcat(modes, "r");
    else
        strcat(modes, "-");
    if ((fileMode & S_IWUSR) && (fileMode & __S_IWRITE))
        strcat(modes, "w");
    else
        strcat(modes, "-");
    if ((fileMode & S_IXUSR) && (fileMode & __S_IEXEC))
        strcat(modes, "x");
    else
        strcat(modes, "-");

    if ((fileMode & S_IRGRP) && (fileMode & __S_IREAD))
        strcat(modes, "r");
    else
        strcat(modes, "-");
    if ((fileMode & S_IWGRP) && (fileMode & __S_IWRITE))
        strcat(modes, "w");
    else
        strcat(modes, "-");
    if ((fileMode & S_IXGRP) && (fileMode & __S_IEXEC))
        strcat(modes, "x");
    else
        strcat(modes, "-");

    if ((fileMode & S_IROTH) && (fileMode & __S_IREAD))
        strcat(modes, "r");
    else
        strcat(modes, "-");
    if ((fileMode & S_IWOTH) && (fileMode & __S_IWRITE))
        strcat(modes, "w");
    else
        strcat(modes, "-");
    if ((fileMode & S_IXOTH) && (fileMode & __S_IEXEC))
        strcat(modes, "x   ");
}

// get file owner as string
void get_owner(struct stat fileattrib, char* own_name)
{
    struct passwd* pw = getpwuid(fileattrib.st_uid);
    strcpy(own_name, pw->pw_name);
}

// get group as string
void get_group(struct stat fileattrib, char* gr_name)
{
    struct group* gr = getgrgid(fileattrib.st_gid);
    strcpy(gr_name, gr->gr_name);
}

// print in different modes ,  item - filename , directory - directory of files,
// complex mode -> option = 1  , dest_buf - to out infomation
size_t print_item(size_t* col_counter, char* item, char* directory, short option, char* dest_buf)
{
    const size_t column_height = 3;
    struct stat flstat;

    if (!col_counter || !item)
        return 0;
    if (strcmp(item, FLD_CURRENT) == 0 || strcmp(item, FLD_PREV) == 0)
    {
        return 0;
    }

    char full_path[MAX_PATH] = "\0";
    strcpy(full_path, directory);
    int count_hrd_lnk = 1;

    strcat(full_path, item);

    char mode[16] = "\0";
    char group[32] = "\0";
    char user[32] = "\0";
    char time[32] = "\0";
    char fsize[36] = "\0";
    size_t size = 0;

    flstat.st_blocks = 0;
    if (stat(full_path, &flstat) != -1)
    {
        count_hrd_lnk = flstat.st_nlink;
        file_mode_to_str(mode, flstat);
        get_owner(flstat, user);
        get_group(flstat, group);
        strftime(time, 50, "%Y-%m-%d", localtime(&flstat.st_mtime));
        size = flstat.st_size;
    }
    else
    {
        ;
    }

    if (!option)
    {
        if ((*col_counter) % column_height == 0)
        {
            printf("%s", item);
            putchar('\n');
        }
        else
        {
            printf(" %-85s  ", item);
        }
        (*col_counter)++;
    }
    else
    {
        sprintf(dest_buf, "%-16s %-1d %-16s %-16s %-12zu %-10s %-85s \n", mode, count_hrd_lnk,
            group, user, size, time, item);
    }

    return size;
}
int main(int argc, char** args)
{
    const char more_info_flag[] = "-l";
    short more_info = 0;
    char path[MAX_PATH] = ".";

    if (argc == 2)
    {
        more_info = strcmp(args[1], more_info_flag) == 0;
        if (!more_info)
        {
            strcpy(path, args[1]);
        }
    }
    else
    {
        if (argc >= 3)
        {
            strcpy(path, args[2]);
            more_info = strcmp(args[1], more_info_flag) == 0;
            if (!more_info)
            {
                strcpy(path, args[1]);
                more_info = strcmp(args[2], more_info_flag) == 0;
            }
        }
    }

    size_t block_sz = 0;
    struct stat pstat;
    if (stat(path, &pstat) == -1)
    {
        printf(" Указан неверный путь \n");
        return -1;
    }

    block_sz = pstat.st_blksize;

    DIR* d;
    struct dirent* dir;
    d = opendir(path);

    size_t full_sz = 0;
    static char out_buf[8 * 1024 * 1024] = "\0";

    size_t row_cnt = 1;
    if (d)
    {
        size_t fp_last_ci = strlen(path) - 1;
        if (path[fp_last_ci] != '/' || path[fp_last_ci] != '\\')
        {
            strcat(path, "/");
        }

        while ((dir = readdir(d)) != NULL)
        {
            char loc_buf[MAX_PATH * 5] = "\0";
            full_sz += print_item(&row_cnt, dir->d_name, path, more_info, loc_buf);
            strcat(out_buf, loc_buf);
        }
        closedir(d);
        if (more_info)
            printf("\nИтого %zu\n", full_sz);
    }
    else
    {
        char cp_path[MAX_PATH];
        strcpy(cp_path, path);

        char* delim = strrchr(cp_path, '/');
        if (!delim)
            delim = strrchr(cp_path, '\\');

        char file_dir[MAX_PATH];
        char file_name[MAX_PATH];
        if (delim)
            (*delim) = '\0';
        strcpy(file_name, &delim[1]);
        strcpy(file_dir, cp_path);
        strcat(file_dir, "/");

        full_sz += print_item(&row_cnt, file_name, file_dir, more_info, out_buf);
    }

    printf("%s\n", out_buf);
    return (0);
}
