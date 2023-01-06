#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_LINE 80
#define MAX_ARGS (MAX_LINE/2 + 1)
#define REDIRECT_OUT_OP '>'
#define REDIRECT_IN_OP '<'
#define PIPE_OP '|'
#define BG_OP '&'

/* Holds a single command. */
typedef struct Cmd {
	/* The command as input by the user. */
	char line[MAX_LINE + 1];
	/* The command as null terminated tokens. */
	char tokenLine[MAX_LINE + 1];
	/* Pointers to each argument in tokenLine, non-arguments are NULL. */
	char* args[MAX_ARGS];
	/* Pointers to each symbol in tokenLine, non-symbols are NULL. */
	char* symbols[MAX_ARGS];
	/* The process id of the executing command. */
	pid_t pid;
    int jobNumber;
	/* TODO: Additional fields may be helpful. */

} Cmd;

/* The process of the currently executing foreground command, or 0. */
pid_t foregroundPid = 0;

/* Parses the command string contained in cmd->line.
 * * Assumes all fields in cmd (except cmd->line) are initailized to zero.
 * * On return, all fields of cmd are appropriatly populated. */
void parseCmd(Cmd* cmd) {
	char* token;
	int i=0;
	strcpy(cmd->tokenLine, cmd->line);
	strtok(cmd->tokenLine, "\n");
	token = strtok(cmd->tokenLine, " ");
	while (token != NULL) {
		if (*token == '\n') {
			cmd->args[i] = NULL;
		} else if (*token == REDIRECT_OUT_OP || *token == REDIRECT_IN_OP
				|| *token == PIPE_OP || *token == BG_OP) {
			cmd->symbols[i] = token;
			cmd->args[i] = NULL;
		} else {
			cmd->args[i] = token;
		}
		token = strtok(NULL, " ");
		i++;
	}
	cmd->args[i] = NULL;
}

/* Finds the index of the first occurance of symbol in cmd->symbols.
 * * Returns -1 if not found. */
int findSymbol(Cmd* cmd, char symbol) {
	for (int i = 0; i < MAX_ARGS; i++) {
		if (cmd->symbols[i] && *cmd->symbols[i] == symbol) {
			return i;
		}
	}
	return -1;
}

/* Signal handler for SIGTSTP (SIGnal - Terminal SToP),
 * which is caused by the user pressing control+z. */
void sigtstpHandler(int sig_num) {
	/* Reset handler to catch next SIGTSTP. */
	signal(SIGTSTP, sigtstpHandler);
	if (foregroundPid > 0) {
		/* Foward SIGTSTP to the currently running foreground process. */
		kill(foregroundPid, SIGTSTP);
		/* TODO: Add foreground command to the list of jobs. */
	}
}
/**
 * This is essential for me to find the .txt file name for output.
 * Might be a better way to do it, but this was my solution.
 * @param cmd
 * @param textfile
 * @return the index in args where .txt filename is
 */
int findTxt(Cmd* cmd, char* textfile){
    for (int i = 0; i < MAX_ARGS; i++){
        if (cmd->args[i] == NULL) continue;
        if (strstr(cmd->args[i], textfile) != NULL) {
            return i;
        }
    }
    return -1;
}

/**
 * Helper method to print out what is stored in the cmd struct
 * @param cmd
 */
void printCMD(Cmd* cmd){
    printf("Line %d %d %d \n", cmd->line[0],cmd->line[1], cmd->line[2]);
    printf("Token line %d %d %d \n", cmd->tokenLine[0],cmd->tokenLine[1], cmd->tokenLine[2]);
    printf("Args %s %s %s \n", cmd->args[0],cmd->args[1], cmd->args[2]);
    printf("Symbols %s %s %s \n", cmd->symbols[0],cmd->symbols[1], cmd->symbols[2]);
}

/**
 *  * Helper method to feed in the two args value during a pipe
 * ex: ls | grep txt; where each contains argv1: ls & argv2: grep txt
 * @param argv1
 * @param argv2
 */
void execPipe(char** argv1, char** argv2){
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid == -1){ printf("Error in fork!!\n");}
    if (pid == 0) {// this is the child
        close(fd[1]);
        dup2(fd[0], 0);
        execvp(argv2[0], argv2); //run command after pipe character
        perror("failed to exec argv2\n");
    } else { //parent
        close(fd[0]);
        dup2(fd[1], 1);
        execvp(argv1[0], argv1);
        perror("failed to exec argv1\n");
    }
}

/**
 * Helper method to break a part the cmd->args to know where pipe splits commands
 * @param argv1 first half of args before pipe
 * @param argv2 second half of args after pipe
 * @param cmd
 * @param pSymbol pipe Symbol
 */
void parseArgs(char** argv1, char** argv2, Cmd* cmd, int pSymbol){
    int j = 0;
    for(int i = 0; i < MAX_ARGS; i++){
        if (i < pSymbol) argv1[i] = cmd->args[i];
    }
    argv1[pSymbol] = NULL;
    for (int i = pSymbol; i < MAX_ARGS; i++){
        if (cmd->args[i] != NULL) {
            argv2[j] = cmd->args[i];
            j++;
        }
    }
    argv2[j] = NULL;
}

int main(void) {
	/* Listen for control+z (suspend process). */
	signal(SIGTSTP, sigtstpHandler);
	while (1) {
		printf("352> ");
		fflush(stdout);
		Cmd *cmd = (Cmd*) calloc(1, sizeof(Cmd));
		fgets(cmd->line, MAX_LINE, stdin);
		parseCmd(cmd);
		if (!cmd->args[0]) {
			free(cmd);
		} else if (strcmp(cmd->args[0], "exit") == 0) {
			free(cmd);
			exit(0);

			/* TODO: Add built-in commands: jobs and bg. */

		} else {
			if (findSymbol(cmd, BG_OP) != -1) {
			    int status;
				/* TODO: Run command in background. */
                //signal(SIGCHLD, SIG_IGN);
                cmd->pid = fork();
                if (cmd->pid < 0) exit(-1); //failed
                if (cmd->pid == 0) {//Child process
                    execvp(*cmd->args, cmd->args);
                } else{
                    cmd->jobNumber++;
                    printf("[%d] %d\n", cmd->jobNumber, cmd->pid);
                    waitpid(cmd->pid, &status, WNOHANG);
                }


			} else {
                /* TODO: Run command in foreground. */
                int status = 0;
                //printCMD(cmd);
                //The following three ints are to find special occasions of redirection and pipes for future if
                int output = findSymbol(cmd, '>');
                int input = findSymbol(cmd, '<');
                int pSymbol = findSymbol(cmd, '|');
                cmd->pid = fork();
                if (cmd->pid < 0) exit(-1); //failed
                foregroundPid = cmd->pid;
                if (cmd->pid == 0) //child process
                {
                    char* filename = ".txt";
                    if (output > 0) { //This section handles the case when outputting to a text file with redirection
                        if (strcmp(cmd->symbols[output], ">") == 0) {
                            int findtxt = findTxt(cmd, filename);
                            if (findtxt < 0) fprintf(stderr, "missing output .txt file argument\n");
                            char *filename = cmd->args[findtxt];
                            freopen(filename, "w", stdout);
                            execvp(*cmd->args, cmd->args);
                            exit(-1);
                        }
                    }
                    if (input > 0) { //This section handles the case when the shell is receiving input with redirection
                        if (strcmp(cmd->symbols[input], "<") == 0) {
                            int findtxt = findTxt(cmd, filename);
                            if (findtxt < 0) fprintf(stderr, "missing output .txt file argument\n");
                            char *filename = cmd->args[findtxt];
                            freopen(filename, "r", stdin);
                            execvp(*cmd->args, cmd->args);
                            exit(-1);
                        }
                    }
                    if (pSymbol > 0) { //This section handles the pipe. Breaking the cmd->args into seperate args to be fed into execPipe() helper method
                        char* argv1[MAX_ARGS];
                        char* argv2[MAX_ARGS];
                        parseArgs(argv1, argv2, cmd, pSymbol);
                        execPipe(argv1, argv2);
                        exit(-1);
                    }
                    else { //Execute any commands that doesn't have a special request needed
                        execvp(*cmd->args, cmd->args);
                        fprintf(stderr, "unknown command: %s\n", cmd->args[0]);
                        exit(-1);
                    }
                } else { //parent process needs to wait
                    foregroundPid = cmd->pid;
                    waitpid(cmd->pid, &status, 0);
                    if (status != 0) fprintf (stderr, "error with status code\n");
                }
			}
		}
		/* TODO: Check on status of background processes. */
        //signal(SIGCHLD, SIG_IGN);
        //Waits for the child processes to finish and parent before printing done with execution.
		while (wait(NULL)>0){
            printf("[%d] Done ", cmd->jobNumber);
            for (int i = 0; i < MAX_ARGS; i++){
                if (cmd->args[i] != NULL){
                    printf("%s ", cmd->args[i]);
                }
            }
            printf("\n");
            cmd->jobNumber--;
        }
	}
	return 0;
}
