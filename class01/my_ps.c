#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
//#include <sys/types.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
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
                 printf("%s ", dir->d_name);

                 // Open and read from /proc/[PID]/comm
                 char comm_path[256+11];
                 sprintf(comm_path, "/proc/%s/comm", dir->d_name);
                 int fd = open(comm_path, O_RDONLY);

                 if (fd < 0) {
                     printf(" \n");
                     continue;
                 }

                 int bytes_read = read(fd, comm_path, 256);

                 if (bytes_read > 0) {
                     comm_path[bytes_read] = '\0';
                     printf("%s", comm_path);
                 } else {
                     printf(" \n");
                 }

                 if (close(fd) != 0)
                     fputs("Error in closing file!", stderr);
            }
        }

        closedir(proc_dir);
    }

    /* if (chdir("/proc") == -1) {
        fputs("Can not change directory to /proc!\n", stderr);
        return -1;
    } */

    return 0;
}

// The start time of the process is located at /proc/PID/stat column 22.
