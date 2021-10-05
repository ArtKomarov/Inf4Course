#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
//#include <sys/types.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>

char* getCommandName(char* command, const int comm_len, const char* pid, const char* source);

int main() {
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
	
    puts("PID CMD");

    while ((dir = readdir(proc_dir)) != NULL) {
        if (regexec(&regex, dir->d_name, 0, NULL, 0) != 0)
            continue;
		
        printf("%s ", dir->d_name);
		
		const int comm_len = 256 + 11;
        char command[comm_len];
		
		if (getCommandName(command, comm_len, dir->d_name, "cmdline") == NULL) {
			if (getCommandName(command, comm_len, dir->d_name, "comm") == NULL) {
				puts(" ");
				continue;
			}
		}
		
		printf("%s\n", command);
    }

    if (closedir(proc_dir) != 0) {
		perror("closedir(\"/proc\")");
		return -1;
	}

    return 0;
}

char* getCommandName(char* command, const int comm_len, const char* pid, const char* source) {
    // Open and read from /proc/[PID]/{source}
	command[comm_len - 1] = '\0';
	int read_bytes = snprintf(command, comm_len - 1, "/proc/%s/%s", pid, source);
	
	if (read_bytes <= 0) {
		perror(command);
		return NULL;
	}
	
	int fd = open(command, O_RDONLY);

	if (fd < 0) {
		perror(command);
		return NULL;
	}

	int bytes_read = read(fd, command, comm_len - 1);
	
	if (close(fd) != 0) {
		fputs("Error in closing file. ", stderr);
		perror("close");
	}

	if (bytes_read > 0) {
		command[bytes_read] = '\0';

		char* space = strchr(command, ' ');
		if (space != NULL)
			*space = '\0';
		else if ((space = strchr(command, '\n')) != NULL)
            *space = '\0';

		return command;
	} else 
		return NULL;
	
	return command;
}

// The start time of the process is located at /proc/PID/stat column 22.
