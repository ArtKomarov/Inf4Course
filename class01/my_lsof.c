#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void readlinks(char* pid, char* dir_name);

int main() {
    printf("pid \t pathname\n");
    regex_t regex;
    if (regcomp(&regex, "^[0-9][0-9]*$", 0) != 0) { // pid dirs
        fputs("Regular expression compilation error!\n", stderr);
        return -1;
    }

    // We will open /proc directory and read info
    DIR* proc_dir;
    struct dirent *dir;
    proc_dir = opendir("/proc");

    if (proc_dir == NULL) {
        perror("opendir(\"/proc\")");
        return -1;
    }

    while ((dir = readdir(proc_dir)) != NULL) {
        if (regexec(&regex, dir->d_name, 0, NULL, 0) != 0)
            continue;

        readlinks(dir->d_name, "fd");
        readlinks(dir->d_name, "map_files");
    }

    if (closedir(proc_dir) != 0) {
        perror("closedir(\"/proc\")");
        return -1;
    }

    return 0;
}

void readlinks(char* pid, char* dir_name) {
    // Open and read links from /proc/[PID]/dir_name
    const int files_path_len = 256 + 64;
    char files_path[files_path_len];
    const int dir_path_len = snprintf(files_path, files_path_len - 1, "/proc/%s/%s/", pid, dir_name);

    DIR* files_dir;
    struct dirent *file;
    files_dir = opendir(files_path);

    if (files_dir == NULL) {
        perror(files_path);
        return;
    }

    while ((file = readdir(files_dir)) != NULL) {
        files_path[dir_path_len] = '\0';
        char link_name[files_path_len];

        ssize_t read_bytes = readlink(strcat(files_path, file->d_name), link_name, files_path_len - 1);
        if (read_bytes <= 0)
            continue;

        link_name[read_bytes] = '\0';

        // Write info on console
        printf("%s ", pid);
        printf("%s\n", link_name);
    }

    if (closedir(files_dir) != 0) {
        fputs(pid, stderr);
        perror(": closedir");
        return;
    }
}


