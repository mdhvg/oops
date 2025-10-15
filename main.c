#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int get_histfile(const char* command, char* path_buffer) {
	FILE* p = popen(command, "r");
	if (!p) {
		fprintf(stderr, "Command (%s) failed\n", command);
		return 1;
	}
	if (!fgets(path_buffer, 256, p)) {
		fprintf(stderr, "Reading command (%s) output failed\n", command);
		pclose(p);
		return 1;
	}
	pclose(p);
	return 0;
}

int remove_zsh_lines(const char* histfile_command, int lines) {
	char pathbuf[256];
	if (get_histfile(histfile_command, pathbuf) != 0) return 1;
	pathbuf[strcspn(pathbuf, "\n")] = 0; // Very Very important, filepaths have a trailing \n char

	FILE* hist = fopen(pathbuf, "r+");
	if (hist == NULL) {
		fprintf(stderr, "Can't open histfile: %s\n", pathbuf);
		return 1;
	}
	if (fseek(hist, 0, SEEK_END) != 0) {
		fprintf(stderr, "Seek error\n");
		fclose(hist);
		return 1;
	}
	size_t pos = ftell(hist);
	if (pos < 0) {
		fprintf(stderr, "fTell error\n");
		fclose(hist);
		return 1;
	}

	// Every command in zsh_history file follows this pattern \n: 1719311251:0;ls
	// So, sequence becomes [\n] -> [:] -> [ ] -> [NUM] (Can put multiple numbers here too)
	// state value = 		  0  ->  1  ->  2  ->   3
	// This sequence is parsed in reverse since we're reading the file from the end
	int initial = 3; // Storing the base case vlaue
	int state = 3;   // For checking the state in state machine

	int counted = 0;
	while (pos > 0 && counted < lines) {
		pos--;
		fseek(hist, pos, SEEK_SET);
		int c = fgetc(hist);

		switch (c) {
			// For all cases, if value is found, proceed to next state
			// otherwise reset to base state
			case ' ':
				state = state == 2 ? 1 : initial;
				break;
			case ':':
				state = state == 1 ? 0 : initial;
				break;
			case '\n':
				// Sequence is complete, means 1 line is read
				counted = state == 0 ? (counted + 1) : counted;
				// Reset state
				state = initial;
				break;
			default:
				if (isdigit(c)) {
					// If we see any number, we change the state
					state = 2;
				} else {
					// If the sequence is broken return to base state
					state = initial;
				}
		}
	}

	if (ftruncate(fileno(hist), pos + 1) != 0) { // Keep the last newline
		fprintf(stderr, "Truncate error\n");
		fclose(hist);
		return 1;
	}
	fclose(hist);
	return 0;
}

// Turns out bash isn't supported yet
// int remove_bash_lines(const char* histfile_command, int lines) {
// 	char pathbuf[256];
// 	if (get_histfile(histfile_command, pathbuf)) return 1;
// 	pathbuf[strcspn(pathbuf, "\n")] = 0; // Very Very important, filepaths have a trailing \n char

// 	FILE* hist = fopen(pathbuf, "r+");
// 	if (hist == NULL) {
// 		fprintf(stderr, "Can't open histfile: %s\n", pathbuf);
// 		return 1;
// 	}
// 	if (fseek(hist, 0, SEEK_END) != 0) {
// 		fprintf(stderr, "Seek error\n");
// 		fclose(hist);
// 		return 1;
// 	}
// 	size_t pos = ftell(hist);
// 	if (pos < 0) {
// 		fprintf(stderr, "fTell error\n");
// 		fclose(hist);
// 		return 1;
// 	}

// 	int counted = 0;
// 	while (pos > 0 && counted < lines) {
// 		pos--;
// 		fseek(hist, pos, SEEK_SET);
// 		int c = fgetc(hist);

// 		if (c == '\n') {
// 			counted++;
// 		}
// 	}

// 	if (ftruncate(fileno(hist), pos + 1) != 0) { // Keep the last newline
// 		fprintf(stderr, "Truncate error\n");
// 		fclose(hist);
// 		return 1;
// 	}
// 	fclose(hist);
// 	return 0;
// }

int main(int argc, char** argv) {
	int count = 2; // One line for this command, another is to be deleted
	if (argc == 2) {
		if (strncmp("-h", argv[1], 2) == 0) {
			printf("Usage: %s <line_count>\n", argv[0]);
			return 0;
		}
		count = atoi(argv[1]) + 1;
	}
	if (count <= 1) {
		fprintf(stderr, "Are u an idiot?\n");
		return 1;
	}

	int parent_pid = getppid();
	char cmdline_path[100];
	snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", parent_pid);

	FILE *fp = fopen(cmdline_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Couldn't read parent cmdline\n");
		return 1;
	}

	char parent[256];
	size_t read = fread(parent, 1, sizeof(parent) - 1, fp);
	fclose(fp);
	if (read < 1) {
		fprintf(stderr, "No bytes read from parent cmdline\n");
		return 1;
	}

	if (strstr(parent, "zsh")) {
		return remove_zsh_lines("zsh -i -c 'echo $HISTFILE'", count);
	} 
	// After experimentation, I found bash writes all the executed command (including oops)
	// into a buffer in memory and after quitting (bash) it writes them to the bash_history
	// file. So, deleting the last n lines this way is pointless. Need a method to delete
	// lines after bash exits. So, I'm commenting this out too.
	// else if (strstr(parent, "bash")) {
	// 	return remove_bash_lines("bash -i -c 'echo $HISTFILE'", count);
	// }
	// ...Add more shells
	else {
		fprintf(stderr, "Shell not recognised %s\n", parent);
		fprintf(stdout, "You can submit a patch for your preferred shell here: https://github.com/mdhvg/oops\n");
		return 1;
	}
}
