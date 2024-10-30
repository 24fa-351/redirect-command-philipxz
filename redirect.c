#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char **split_command(const char *cmd) {
    char **commands = malloc(64 * sizeof(char *));
    int i = 0;
    char *cmd_copy = strdup(cmd);
    char *token = strtok(cmd_copy, " ");

    while (token != NULL && i < 63) {
        commands[i++] = token;
        token = strtok(NULL, " ");
    }
    commands[i] = NULL; 
    return commands;
}

char *find_command_path(const char *command) {
    char *path = getenv("PATH");
    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");
    static char cmd_path[512];
    while (dir != NULL) {
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", dir, command);
        if (access(cmd_path, X_OK) == 0) {
            free(path_copy);
            return cmd_path;
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        return 1;
    }

    char *inp = argv[1];
    char *cmd = argv[2];
    char *out = argv[3];

    // Split the cmd string into separet arguments
    char **command = split_command(cmd);

    // Find the absolute path of the command
    char *cmd_path = find_command_path(command[0]);
    if (!cmd_path) {
        fprintf(stderr, "Command not found: %s\n", command[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (strcmp(inp, "-") != 0) {
            int input_fd = open(inp, O_RDONLY);
            if (input_fd < 0) {
                perror("open inp");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (strcmp(out, "-") != 0) {
            int output_fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("open out");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        execvp(command[0], command);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
