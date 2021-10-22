/*
    ? Что находится в файле /proc/PID/auxv и
    ?     как выглядит стек процесса сразу после вызова execve()?

    + /proc/PID/auxv contains the contents of the ELF interpreter
    +     information passed to the process at exec time.  The
    +     format is one unsigned long ID plus one unsigned long
    +     value for each entry.

    + current program replaced by new program, stack of the calling process
    +     overwritten according to the contents of the newly loaded program
*/

/*
    ? Напишите программу, которая спрячет первый аргумент командной строки
    ? из /proc/PID/cmdline
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    char buf[2048], *tmp;
    FILE *f;
    int i;
    unsigned long arg_start;

    // Open stat file to find ARG_START
    f = fopen("/proc/self/stat", "r");
    if (!f) {
        fprintf(stderr, "Error while oppening the stat file!\n");
        return -1;
    }

    tmp = fgets(buf, sizeof(buf), f);

    if (fclose(f) != 0) {
        perror("fclose stat");
    }

    if (!tmp) {
        return -1;
    }

    // Skip the first 47 fields, column 48 are ARG_START
    tmp = strchr(buf, ' ');
    for (i = 0; i < 46; ++i) {
        if (!tmp)
            return -1;
        tmp = strchr(tmp + 1, ' ');
    }

    if (!tmp)
        return -1;

    i = sscanf(tmp, "%lu", &arg_start);
    if (i != 1) {
        return -1;
    }

    // Hide first argument (set second argument address)
    if (prctl(PR_SET_MM, PR_SET_MM_ARG_START, arg_start + strlen(argv[0]) + 1, 0, 0) < 0) {
        fprintf(stderr, "Error while executing prctl! (It should be run with sudo)\n");
        return -1;
    }

    // Check cmdline
    char system_str[128];
    system_str[127] = '\0';
    snprintf(system_str, 128 - 1, "cat /proc/%d/cmdline", getpid());

    if (system(system_str) == -1) {
        perror("system");
        return -1;
    }

    puts("");

    return 0;
}

