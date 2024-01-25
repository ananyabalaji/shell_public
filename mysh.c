#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 100

int last_command_exit_status = 0;

// Function to parse a command line into tokens
void parse_command(char *command, char *args[])
{
    char *token;
    int i = 0;

    // Skip over the initial "mysh>" prompt
    if (strncmp(command, "mysh>", 5) == 0)
    {
        command += 5;
    }

    token = strtok(command, " \t\n");

    while (token != NULL)
    {
        // Checking for the wildcard token
        if (strchr(token, '*') != NULL)
        {
            // Token containing a wildcard
            glob_t glob_res;
            int glob_flags = 0;

            // Append expanded file names to args array
            if (glob(token, glob_flags, NULL, &glob_res) == 0)
            {
                for (size_t j = 0; j < glob_res.gl_pathc; ++j)
                {
                    args[i++] = strdup(glob_res.gl_pathv[j]);
                }
                // Free dynamically allocated storage
                globfree(&glob_res);
            }
            else
            {
                // No matches found for the wildcard; add the token as is
                args[i++] = strdup(token);
            }
        }
        else
        {
            args[i++] = strdup(token);
        }

        token = strtok(NULL, " \t\n");
    }

    args[i] = NULL; // Null terminates the array
}

// Free the argument memory
void free_args(char *args[])
{
    for (int i = 0; args[i] != NULL; ++i)
    {
        free(args[i]);
    }
}

void print_args(char *args[])
{
    for (int i = 0; args[i] != NULL; ++i)
    {
        printf("args[%d]:%s\n", i, args[i]);
    }
}
// Shift argument memory
void del_arg(int curr, char *args[])
{
    for (int i = curr; args[i] != NULL; ++i)
    {
        free(args[i]);
        if (args[i + 1] != NULL)
            args[i] = strdup(args[i + 1]);
        else
            args[i] = NULL;
    }
}

// Function to handle input and output redirection
void perform_redirection(char *input_file, char *output_file)
{
    if (input_file != NULL)
    {
        int input_fd = open(input_file, O_RDONLY);
        if (input_fd == -1)
        {
            perror("open");
            // exit(EXIT_FAILURE);
        }
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            // exit(EXIT_FAILURE);
        }
        close(input_fd);
        free(input_file);
    }

    if (output_file != NULL)
    {
        int output_fd;
        // Handle the case where '>' or '>>' is followed by no output file
        if (strcmp(output_file, ">") == 0 || strcmp(output_file, ">>") == 0)
        {
            fprintf(stderr, "mysh: syntax error near unexpected token `%s'\n", output_file);
            // exit(EXIT_FAILURE);
        }
        else
        {
            output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (output_fd == -1)
            {
                perror("open");
                // exit(EXIT_FAILURE);
            }
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2");
                // exit(EXIT_FAILURE);
            }
            close(output_fd);
            free(output_file);
        }
    }
}

int isNonNameToken(char *token)
{
    if (strstr(token, "<") != NULL)
        return 1;
    if (strstr(token, ">") != NULL)
        return 1;
    return 0;
}

void handle_which(char **args){
    // Handle the which command
        if (args[1] == NULL)
        {
            fprintf(stderr, "which: missing argument\n");
            return;
            // exit(EXIT_FAILURE);
        }

        // Iterate through all arguments after "which"
        for (int i = 1; args[i] != NULL; ++i)
        {

            // Search for the program in the specified directories
            char *directories[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL};
            char program_path[MAX_INPUT_SIZE];
            int found = 0;

            for (int j = 0; directories[j] != NULL; j++)
            {
                snprintf(program_path, sizeof(program_path), "%s/%s", directories[j], args[i]);
                if (access(program_path, X_OK) == 0)
                {
                    printf("%s\n", program_path);
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                fprintf(stderr, "which: %s: not found\n", args[i]);
                return;
                // exit(EXIT_FAILURE);
            }
        }
}

void handle_pwd(char **args){
        // Handle the pwd command
        if (args[1] != NULL)
        {
            fprintf(stderr, "pwd: ignoring the arguments\n");
        }

        char current_directory[MAX_INPUT_SIZE];
        if (getcwd(current_directory, sizeof(current_directory)) == NULL)
        {
            perror("getcwd");
            return;
            // exit(EXIT_FAILURE);
        }
        printf("%s\n", current_directory);
}

void handle_cd(char **args){
            // Handle the cd command
        if (args[1] == NULL)
        {
            fprintf(stderr, "cd: missing argument\n");
            return;
            // exit(EXIT_FAILURE);
        }
        else if (args[2] != NULL)
        {
            fprintf(stderr, "cd: too many arguments\n");
            return;
            // exit(EXIT_FAILURE);
        }
        if (chdir(args[1]) == -1)
        {
            perror("chdir");
            return;
            // exit(EXIT_FAILURE);
        }
}

void handle_execv(char **args, char *input_file, char *output_file){
            // Fork a new process for command execution
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");
        }
        else if (pid == 0)
        {
            // Code executed in the child process

            // Execute the command using execv
            execv(args[0], args);

            // If execv fails, attempt to find the command in the specified directories
            char *directories[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL};
            char program_path[MAX_INPUT_SIZE];
            int found = 0;

            for (int i = 0; directories[i] != NULL; i++)
            {
                snprintf(program_path, sizeof(program_path), "%s/%s", directories[i], args[0]);
                if (access(program_path, X_OK) == 0)
                {
                    // print_args(args);
                    execv(program_path, args);
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                perror("execv");
                return;
                // exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Code executed in the parent process

            int status;
            // Wait for the child process to finish
            waitpid(pid, &status, 0);

            // Update the last command exit status
            last_command_exit_status = WEXITSTATUS(status);

            // Print the status message only when not in redirection
            if (input_file == NULL && output_file == NULL)
            {
                if (WIFEXITED(status))
                {
                    // printf("Child process exited with status %d\n\n", WEXITSTATUS(status));
                }
                else if (WIFSIGNALED(status))
                {
                    // printf("Child process terminated by signal %d\n", WTERMSIG(status));
                }
            }
        }
}

void restore_stdin_stdout(char **args, char *input_file, char *output_file){
     // Restore standard input and output
    if (input_file != NULL || output_file != NULL)
    {
        int input_fd = open("/dev/tty", O_RDONLY);
        if (input_fd == -1)
        {
            perror("open");
            return;
            // exit(EXIT_FAILURE);
        }
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2");
            return;
            // exit(EXIT_FAILURE);
        }
        close(input_fd);

        int output_fd = open("/dev/tty", O_WRONLY);
        if (output_fd == -1)
        {
            perror("open");
            return;
            // exit(EXIT_FAILURE);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            return;
            // exit(EXIT_FAILURE);
        }
        close(output_fd);
    }
}

// Function to handle built-in commands (cd, pwd, which)
void handle_builtin_command(char *args[])
{
    char *input_file = NULL;
    char *output_file = NULL;

    if (strcmp(args[0], "then") == 0)
    {
        // Handle the then command
        if (last_command_exit_status == 0)
        {
            // parse_command(args[1], args);
            del_arg(0, args);
            handle_builtin_command(args);
            // free_args(args);
        }
        return;
    }
    else if (strcmp(args[0], "else") == 0)
    {
        // Handle the else command
        if (last_command_exit_status != 0)
        {
            // parse_command(args[1], args);
            del_arg(0, args);
            handle_builtin_command(args);
            // free_args(args);
        }
        return;
    }

    // Iterate through command arguments and handle redirection
    for (int i = 0; args[i] != NULL; ++i)
    {
        if (strcmp(args[i], "<") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "redirection: no arguments\n");
                return;
                // exit(EXIT_FAILURE);
            }
            else if (isNonNameToken(args[i + 1]))
            {
                fprintf(stderr, "syntax error with non-name token\n");
                return;
            }
            else
            {
                // Exclude '<' and the input file from args
                del_arg(i, args);
                input_file = strdup(args[i]);
                //  printf("input_file:%s\n", input_file);
                del_arg(i, args);
                i--;
            }
        }
        else if (strcmp(args[i], ">") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "redirection: no arguments\n");
                return;
            }
            else if (isNonNameToken(args[i + 1]))
            {
                fprintf(stderr, "syntax error with non-name token\n");
                return;
            }
            else
            {
                // Exclude '<' and the output file from args
                del_arg(i, args);
                output_file = strdup(args[i]);
                // printf("output file:%s\n", output_file);
                del_arg(i, args);
                i--;
            }
        }
        else if (strcmp(args[i], "<>") == 0 || strcmp(args[i], "><") == 0)
        {
            fprintf(stderr, "syntax error with non-name token\n");
            return;
        }
    }

    if (args[0] == NULL)
    {
        fprintf(stderr, "syntax error with non-name token\n");
        return;
    }

    // If redirection is present, handle it
    perform_redirection(input_file, output_file);

    // Check if the command is a built-in command
    if (strcmp(args[0], "cd") == 0)
    {
        handle_cd(args);
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        handle_pwd(args);
    }
    else if (strcmp(args[0], "which") == 0)
    {
        handle_which(args);
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        // Handle the exit command
        printf("mysh: exiting\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        handle_execv(args, input_file, output_file);
    }

   restore_stdin_stdout(args, input_file, output_file);
}

// Function to create a pipeline between two commands
void create_pipeline(char *command1, char *command2)
{
    // Create pipe
    FILE *pipe_fp = popen(command1, "r");
    if (pipe_fp == NULL)
    {
        perror("popen");
        // exit(EXIT_FAILURE);
    }

    // Fork a child process for the second command
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        // exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Code executed in the child process

        // Redirect input to come from the pipe
        if (dup2(fileno(pipe_fp), STDIN_FILENO) == -1)
        {
            perror("dup2");
            // exit(EXIT_FAILURE);
        }

        // Close the read end of the pipe
        fclose(pipe_fp);

        // Execute the second command using system
        if (system(command2) == -1)
        {
            perror("system");
            // exit(EXIT_FAILURE);
        }

        // Exit the child process
        exit(EXIT_SUCCESS);
    }

    // Code executed in the parent process

    // Close the write end of the pipe
    pclose(pipe_fp);

    // Wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);

    // Print the status message
    if (WIFEXITED(status))
    {
        // printf("Child process exited with status %d\n\n", WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        // printf("Child process terminated by signal %d\n", WTERMSIG(status));
    }
}

int main(int argc, char *argv[])
{
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    if (argc == 1)
    {
        // INTERACTIVE MODE
        printf("\nWelcome to My Shell!\n");

        while (1)
        {
            printf("\nmysh> ");
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                break; // End of input (Ctrl-D)
            }

            // Remove newline character
            input[strcspn(input, "\n")] = 0;

            // Check for pipeline command
            if (strstr(input, "|") != NULL)
            {
                // Separate the two commands
                char *command1 = strtok(input, "|");
                char *command2 = strtok(NULL, "|");
                if (command1 != NULL && command2 != NULL)
                {
                    create_pipeline(command1, command2);
                }
                else
                {
                    fprintf(stderr, "Invalid pipeline command!\n");
                }
            }
            else
            {
                parse_command(input, args);
                handle_builtin_command(args);
                free_args(args);
            }
        }
    }
    else if (argc == 2)
    {
        // BATCH MODE
        FILE *file = fopen(argv[1], "r");
        if (file == NULL)
        {
            perror("mysh");
            return EXIT_FAILURE;
        }

        while (fgets(input, sizeof(input), file) != NULL)
        {
            // Remove any newline characters
            input[strcspn(input, "\n")] = 0;

            // Check for pipeline command
            if (strstr(input, "|") != NULL)
            {
                // Separate the two commands
                char *command1 = strtok(input, "|");
                char *command2 = strtok(NULL, "|");
                if (command1 != NULL && command2 != NULL)
                {
                    create_pipeline(command1, command2);
                }
                else
                {
                    fprintf(stderr, "Invalid pipeline command!\n");
                }
            }
            else
            {
                parse_command(input, args);
                handle_builtin_command(args);
                free_args(args);
            }
        }

        fclose(file);
    }
    else
    {
        fprintf(stderr, "Usage: %s [script]\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}