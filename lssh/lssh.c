#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Add includes for wait/types/stat and anything else encountered during research
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define PROMPT "lambda-shell$ "

#define MAX_TOKENS 100
#define COMMANDLINE_BUFSIZE 1024
#define DEBUG 1  // Set to 1 to turn on some debugging output, or 0 to turn off

/**
 * Parse the command line.
 *
 * YOU DON'T NEED TO MODIFY THIS!
 * (But you should study it to see how it works)
 *
 * Takes a string like "ls -la .." and breaks it down into an array of pointers
 * to strings like this:
 *
 *   args[0] ---> "ls"
 *   args[1] ---> "-la"
 *   args[2] ---> ".."
 *   args[3] ---> NULL (NULL is a pointer to address 0)
 *
 * @param str {char *} Pointer to the complete command line string.
 * @param args {char **} Pointer to an array of strings. This will hold the result.
 * @param args_count {int *} Pointer to an int that will hold the final args count.
 *
 * @returns A copy of args for convenience.
 */
char **parse_commandline(char *str, char **args, int *args_count)
{
    char *token;
    
    *args_count = 0;

    token = strtok(str, " \t\n\r");

    while (token != NULL && *args_count < MAX_TOKENS - 1) {
        args[(*args_count)++] = token;

        token = strtok(NULL, " \t\n\r");
    }

    args[*args_count] = NULL;

    return args;
}

/**
 * Main
 */
int main(void)
{
    // Holds the command line the user types in
    char commandline[COMMANDLINE_BUFSIZE];

    // Holds the parsed version of the command line
    char *args[MAX_TOKENS];

    // How many command line args the user typed
    int args_count;

    // background task hang up tracker
    int noHang = 0;
    
    // Shell loops forever (until we tell it to exit)
    while (1) {
        // Print a prompt
        printf("%s", PROMPT);
        fflush(stdout); // Force the line above to print

        // Read input from keyboard
        fgets(commandline, sizeof commandline, stdin);

        // Exit the shell on End-Of-File (CRTL-D)
        if (feof(stdin)) {
            break;
        }

        // Parse input into individual arguments
        parse_commandline(commandline, args, &args_count);

        if (args_count == 0) {
            // If the user entered no commands, do nothing
            continue;
        }

        // Exit the shell if args[0] is the built-in "exit" command
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // change directory
        else if (strcmp(args[0], "cd") == 0) {
            char *homeDir = getenv("HOME");
            char *dir = (args_count > 1) ? args[1] : homeDir;
            // error is not printing if only cd is input in command line unsure where failing
            if (chdir(dir) == -1) {
                perror("chdir");
            }

            continue;
        }

        // background task handler
        int fileOutput = 0;
        char *output;
        for (int i = 1; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                fileOutput = 1;
                output = args[i + 1];
            }
            if (fileOutput == 1) {
                args[i] = NULL;
            }
        }

        #if DEBUG

        // Some debugging output

        // Print out the parsed command line in args[]
        for (int i = 0; args[i] != NULL; i++) {
            printf("%d: '%s'\n", i, args[i]);
        }

        #endif
        
        /* Add your code for implementing the shell's logic here */
        // fork to create child process
        if (fork() == 0) {
            // child created
            if (fileOutput == 1) {
                // O_WRONLY (open for writing only) and O_CREAT include <fcntl.h>
                int fd = open(fileOutput, O_WRONLY | O_CREAT, 0777);
                dup2(fd, 1);
                close(fd);
            }

            // execvp() commands and exit
            execvp(args[0], args);

            exit(0);
        } else {
            // parent check for hangup added for background tasks stretch
            if (noHang == 1) {
                while (waitpid(-1, NULL, WNOHANG) > 0);
            } else {
                wait(NULL);
            }
        }
        
        
    }

    return 0;
}
