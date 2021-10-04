#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    printf("pid \t inode \t\t\t pathname\n");
    regex_t regex;
    if (regcomp(&regex, "[0-9]", 0) != 0) { // pid dirs
        fputs("Regular expression compilation error!\n", stderr);
        return -1;
    }

    // We will open /proc directory and read info
    DIR* proc_dir;
    struct dirent *dir;
    proc_dir = opendir("/proc");

    if (proc_dir) {
        while ((dir = readdir(proc_dir)) != NULL) {
            if (regexec(&regex, dir->d_name, 0, NULL, 0) == 0) {
                 // Open and read from /proc/[PID]/maps
                 char mem_path[256+11];
                 sprintf(mem_path, "/proc/%s/maps", dir->d_name);
                 FILE* fd = fopen(mem_path, "r");

                 if (fd == NULL) {
                     continue;
                 }

                 char mem_line[1024];

                 while (fgets(mem_line, 2047, fd) != NULL) {
                     char* space = mem_line;

                     int i;
                     // Skip to inode
                     for (i = 0; i < 4; ++i)
                         space = strchr(space, ' ') + 1;

                     char* next_space = strchr(space, ' ');
                     *next_space = '\0';

                     if (strcmp(space, "0") == 0)
                         continue;

                     // Write info on console
                     printf("%s ", dir->d_name);
                     printf("%s ", space);
                     printf("%s", next_space + 1);
                 }

                 if (fclose(fd) != 0)
                     fputs("Error in closing file!", stderr);
            }
        }
        closedir(proc_dir);
    }

    return 0;
}
