#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/wait.h>

#define TokenDelimiter " "
const char* ShellBuiltIns[] = {
  "bg",
  "cd"
};

/*Execute non builtin command process in the background*/
int executeNonBuiltInBackground(char** args) {
	pid_t id = fork();
	int status, i;
	for (i=0; args[i] != NULL; i++) {
		args[i] = args[i+1];
	}
  	if (id == 0) {
  		//child
  		if (execvp(args[0], args) == -1) { 
  			//error
  			return 0;
  		}
  	} else if (id < 0) {
  		//error
  		return 0;
  	}
  	return 1;	
}

/*Execute non builtin command process */
int executeNonBuiltIn(char** args) {
	pid_t id = fork();
	int status;
  	if (id == 0) {
  		//child
  		if (execvp(args[0], args) == -1) { 
  			//error
  			return 0;
  		}
  	} else if (id < 0) {
  		//error
  		return 0;
  	} else {
  		//parent
  		wait(&status);
  	}
  	return 1;	
}


/*Change directory*/
int cd(char** args) {
	if (args[1] == NULL || (strcmp(args[1], "~") == 0)) {
		//home
		chdir(getenv("HOME"));
		return 1;
	}
	if (chdir(args[1])==0){
		//success
		return 1;
	}
	return 0;
}

/*Set up for executing a background process*/
int bg(char** args) {
	if (args[1] == NULL) {
		//error. no arg
		return 0;
	}
	return executeNonBuiltInBackground(args);
}

/*Resolve the built in commands,*/
int resolveShellBuiltIn(char** args, int position) {
	if (strcmp(ShellBuiltIns[position], "bg") == 0) {
		return bg(args);
	}
	else if (strcmp(ShellBuiltIns[position], "cd") == 0) {
		return cd(args);
	}
	return 0;
}

/*Compare the passed in arguments with the built in shell commands*/
int callShellBuiltIns(char** args) {
	int i;
	for (i = 0; i < 2; i++) {
		if (strcmp(args[0], ShellBuiltIns[i]) == 0) {
			return resolveShellBuiltIn(args, i);
		}
	}
	return 0;
}

/*Execute the command. First check if we have a built in available. If we don't, call exec()*/
int execute(char** args) {
	if(args[0] == NULL) {
		return 0;
	}
	if (callShellBuiltIns(args)) {
		return 1;
	}
	else {
		return executeNonBuiltIn(args);
	}
}


/*
Each element of this double pointer array will point to a string, namely the command line arguments.
We are leveraging strtok() which returns a pointer to one of the argument strings we're after.
*/
char** splitOnWhiteSpace(char* command) {
	int position = 0;
	char *token;
	char **args = malloc(PATH_MAX * sizeof(char*));
	token = strtok(command, TokenDelimiter);
	if (token != NULL) {
		do {
			args[position] = token;
			position++;
			token = strtok(NULL, TokenDelimiter);			
		}
		while(token != NULL);
	}
	args[position] = NULL;
	return args;
}

/*Set up and loop to display our welcome prompt. This will continue until we encounter an error.*/
int main(int argc, char*argv[])
{
	char cwdBuffer[PATH_MAX];
	char *cwd, *prompt, *command;
	int status = 0;
	do {
		cwd = getcwd(cwdBuffer,PATH_MAX);
		prompt = malloc(3 + strlen(cwd));
		strcpy(prompt, cwd);
		strcat(prompt, " > ");
		printf("\n");
	
		command = readline(prompt);
		char** args = splitOnWhiteSpace(command);
		status = execute(args);
        free(prompt);
	} while(status);
}