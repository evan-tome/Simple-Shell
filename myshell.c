/*
Lab 2 – A Simple Shell
*/

#define _POSIX_C_SOURCE 200809L // enable POSIX functions
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>


#define MAX_LINE 1024

//Other functions needed ...

static void reap_zombies(void) {
    // OPTIONAL (but recommended if you support '&')
    // Perhaps this will be useful: waitpid(-1, NULL, WNOHANG)
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        ; // reap all finished children
    }
}

static int process_line(char *line) {
    char *args[64];
    int i = 0;

    // Tokenize input
    args[i] = strtok(line, " \t\n");
    while (args[i] != NULL && i < 63) {
        args[++i] = strtok(NULL, " \t\n");
    }
    if (args[0] == NULL) return 1; // empty input

    // Detect redirection and clean args
    char *in_file = NULL;
    char *out_file = NULL;
    int append = 0; // 1 if >> 
    char *new_args[64];
    int j = 0;

    for (int k = 0; args[k] != NULL; k++) {
        if (strcmp(args[k], "<") == 0) {
            if (args[k + 1] == NULL) {  // check if filename exists
                fprintf(stderr, "Error: missing input file\n");
                return 1;
            }
            in_file = args[k + 1];
            k++; // skip filename
        } else if (strcmp(args[k], ">") == 0) {
            if (args[k + 1] == NULL) {
                fprintf(stderr, "Error: missing output file\n");
                return 1;
            }
            out_file = args[k + 1];
            append = 0;
            k++;
        } else if (strcmp(args[k], ">>") == 0) {
            if (args[k + 1] == NULL) {
                fprintf(stderr, "Error: missing output file\n");
                return 1;
            }
            out_file = args[k + 1];
            append = 1;
            k++;
        } else {
            new_args[j++] = args[k]; // keep normal argument
        }
    }
    new_args[j] = NULL;

    // Detect background execution
    int background = 0;
    if (j > 0 && strcmp(new_args[j - 1], "&") == 0) {
        background = 1;
        new_args[j - 1] = NULL;
        j--;
    }

    // Internal commands
    if (strcmp(new_args[0], "quit") == 0) return 0;

    if (strcmp(new_args[0], "cd") == 0) {
        char cwd[PATH_MAX];
        if (!new_args[1]) { // no argument
            if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
        } else if (chdir(new_args[1]) != 0) perror("cd");
        if (getcwd(cwd, sizeof(cwd))) setenv("PWD", cwd, 1);
        return 1;
    }

    if (strcmp(new_args[0], "clr") == 0) {
        printf("\033[H\033[J");
        fflush(stdout);
        return 1;
    }

    if (strcmp(new_args[0], "pause") == 0) {
        printf("Press Enter to continue...");
        fflush(stdout);
        while (getchar() != '\n');
        return 1;
    }

    if (strcmp(new_args[0], "echo") == 0 ||
        strcmp(new_args[0], "dir") == 0 ||
        strcmp(new_args[0], "environ") == 0 ||
        strcmp(new_args[0], "help") == 0) {

        int saved_stdout = -1;
        FILE *fp = NULL;

        // Redirect stdout if needed
        if (out_file) {
            fp = fopen(out_file, append ? "a" : "w");
            if (!fp) { perror("fopen"); return 1; }
            saved_stdout = dup(1);
            dup2(fileno(fp), 1);
        }

        // Execute internal command
        if (strcmp(new_args[0], "echo") == 0) {
            for (int k = 1; new_args[k] != NULL; k++) {
                printf("%s", new_args[k]);
                if (new_args[k + 1] != NULL) printf(" ");
            }
            printf("\n");
        } else if (strcmp(new_args[0], "dir") == 0) {
            char *path = (new_args[1] == NULL) ? "." : new_args[1];
            DIR *d = opendir(path);
            if (!d) perror("dir");
            else {
                struct dirent *entry;
                while ((entry = readdir(d)) != NULL) {
                    printf("%s\n", entry->d_name);
                }
                closedir(d);
            }
        } else if (strcmp(new_args[0], "environ") == 0) {
            extern char **environ;
            for (int k = 0; environ[k] != NULL; k++) {
                printf("%s\n", environ[k]);
            }
        } else if (strcmp(new_args[0], "help") == 0) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
            } else if (pid == 0) {
                // Child process
                execlp("more", "more", "readme", NULL);
                perror("execlp"); // only reached if exec fails
                exit(1);
            } else {
                // Parent process: wait if not background
                if (!background) waitpid(pid, NULL, 0);
            }
        }

        fflush(stdout); // Important for batch/redirection

        // Restore stdout
        if (out_file) {
            dup2(saved_stdout, 1);
            close(saved_stdout);
            fclose(fp);
        }

        return 1;
    }

    // External commands
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process

        // Input redirection
        if (in_file) {
            FILE *fin = fopen(in_file, "r");
            if (!fin) { perror("fopen"); exit(1); }
            dup2(fileno(fin), 0);
            fclose(fin);
        }

        // Output redirection
        if (out_file) {
            FILE *fout = fopen(out_file, append ? "a" : "w"); // append or write
            if (!fout) { perror("fopen"); exit(1); }
            dup2(fileno(fout), 1);
            fclose(fout);
        }

        setenv("parent", getenv("shell"), 1); // child sees parent path
        if (execvp(new_args[0], new_args) == -1) {
            fprintf(stderr, "Command not found: %s\n", new_args[0]);
        }
        exit(1);
    } else {
        // Parent process
        if (!background) waitpid(pid, NULL, 0);
        else printf("Process running in background. PID: %d\n", pid);
    }

    return 1;
}

int main(int argc, char *argv[]) {
    //TODO: set environment variable "shell" to the full path of myshell
    char shell_path[PATH_MAX];

    if (!realpath(argv[0], shell_path)) {
        perror("realpath");
        return 1;
    }
    if (setenv("shell", shell_path, 1) != 0) { // sets environment variable shell
        perror("setenv shell");
        return 1;
    }

    FILE *in = stdin;
    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (!in) { perror("fopen"); return 1; }
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [batchfile]\n", argv[0]);
        return 1;
    }

    char line[MAX_LINE]; // buffer to store a single command line
    char cwd[PATH_MAX]; // buffer to store current working directory

    while (1) {
        reap_zombies(); // clean up terminated background processes

        // only show a prompt if reading from stdin 
        if (in == stdin) {
            if (getcwd(cwd, sizeof(cwd)) == NULL) { perror("getcwd"); break; }
            printf(">myshell:%s$ ", cwd);
            fflush(stdout);
        }

        if (!fgets(line, sizeof(line), in)) break; // EOF => exit (batch requirement)

        line[strcspn(line, "\n")] = 0; // remove trailing newline

        if (process_line(line) == 0) break; // quit
    }

    if (in != stdin) fclose(in);
    return 0;
}