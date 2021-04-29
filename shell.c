#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#define BUFFER_SIZE 80
#define TIME_LIMIT 50 

//KNOWN ERRORS
//Guessing Game prints mini-shell twice after completion.

//Forward method declaration
void interactive_mode();
void parse_arguments(char* line);
void shell_error();
int check_for_builtin(char** args);
void execute_shell_command(char** argv, char** argv_pipe);
void guessingGame();
void free_arg_arrays();
void alarm_handler(int);
void child_handler(int);
void fork_function(char** argv, int in, int out);
void fork_error_handling(pid_t pid);
//Globals for handlers
int timeout = 0;
int child_done = 0;

//Global Variables
char error_message[30] = "An error has occurred\n";	//error message for shell_error()
int arg_position;                                    	//position + 1 of last element in argv array;
int pipe_position;					//position + 1 of last element in argv_pipe array
int arg_bufsize = BUFFER_SIZE;                          //Buffer for getline function in interactive_mode()
char** argv;						//argv array
char** argv_pipe;					//argv array when pipe present
char* builtin[4];  					//built in array for help function
bool pipeCheck;						//true when pipe present, false when not

//Main function
//Intialize a few global variables
//Call method for infinite while loop
int main(){
	
	builtin[0] = "exit";
	builtin[1] = "cd";
	builtin[2] = "help";
	builtin[3] = "GuessingGame";
	argv = NULL;
	argv_pipe = NULL;
	interactive_mode();
  	return 0;
}

//Signal Handler for ctrl + C
void signal_handler(int sig){
	write(1,"\nmini-shell terminated\n",23);
	free_arg_arrays();
	exit(0);
}

//Infinite while loop
//checks for ctrl + c
//Read user input and decide whether to execute shell or builtin function
//free arrays after loop ends
void interactive_mode(){

	signal(SIGINT, signal_handler);
	while(1)
	{
		pipeCheck = false;
		char* line = NULL;
		size_t bufsize = 0;
		argv = malloc(arg_bufsize*sizeof(char*));
		if(!argv) {
			printf("argv error\n");
			shell_error();
		}
		printf("mini-shell>> ");
		//get line allocs a buffer if a buffer size of 0 is provided.
		if(getline(&line, &bufsize, stdin) == -1) {
			printf("getline error\n");
			shell_error();
		}
		//parsing arguments
		parse_arguments(line);
		if(argv[0] != NULL) {
			//continue if string is empty
			if(strcmp(argv[0], "") == 0 ){
				printf("\n");
				continue;
			}
			
			if(check_for_builtin(argv) != 1){
				execute_shell_command(argv, argv_pipe);
			}
		}
		//free arrays and mallocd line
		free_arg_arrays();
		free(line);		
	}
}

//Method to parse arguments
void parse_arguments(char* line){
	char *token;
	pipe_position = 0;
	arg_position = 0;
	
	//return if newline or emptystring character
	if(strcmp(line, "\n") == 0 || strcmp(line, "") == 0)
	{
		return;
	}
	
	token = strtok(line, " \t\n");
	while(token != NULL) {
		//check for pipe symbol
		if(strcmp(token, "|") == 0) {
			argv_pipe = malloc(arg_bufsize*sizeof(char*)); 
			pipeCheck = true;
			token = strtok(NULL, " \t\n");
			continue;
		}

		//add to pipe array if pipe symbol found
		if(pipeCheck == true)
		{
			argv_pipe[pipe_position] = strdup(token);
			pipe_position++;
			if(pipe_position >= arg_bufsize) 
			{
                        	arg_bufsize += BUFFER_SIZE;
                        	argv_pipe = realloc(argv_pipe, arg_bufsize * sizeof(char));
                        	if(!argv_pipe)
				{
                                	shell_error();
                        	}	
				continue;
                	}
		}
		//add to argv array if pipe symbol not found or before its found
		else
		{	
			argv[arg_position] = strdup(token);
			arg_position++;
			if(arg_position >= arg_bufsize) {
				arg_bufsize += BUFFER_SIZE;
				argv = realloc(argv, arg_bufsize * sizeof(char));
				if(!argv){
					shell_error();
				}
			}
		}
		token = strtok(NULL, " \t\n");
	}
	//set position after last element in both arrays to NULL
	argv[arg_position] = NULL;
	if(pipeCheck == true){
		argv_pipe[pipe_position] = NULL;
	}
	//free mallocd strdup token
	free(token);

}

//shell error method
//prints error message and frees arrays
void shell_error() {
	write(STDERR_FILENO, error_message, strlen(error_message));
	free_arg_arrays();	
	exit(1);
}

//Method to free argv and agrv_pipe array
void free_arg_arrays() {
	if(argv != NULL)
	{
		free(argv);
		argv = NULL;
	}
	if(argv_pipe != NULL)
	{
		free(argv_pipe);
		argv_pipe = NULL;
	}
}

//Checks for builtin functions
//Checks if user input matches any builtin function
//returns 1 if it matches, 0 if it doesnt.
int check_for_builtin(char** argv){
	if(strcmp(argv[0], "exit") == 0) {
		
		if(arg_position != 1) {
			shell_error();
		}
		printf("Mini-shell exiting\n");
		free_arg_arrays();
		exit(0);
	
	}else if(strcmp(argv[0], "cd") == 0) {
		
		if(arg_position != 2) {
			printf("path not specified\n");
			return 1;
		}
		
		char cwd[500];
        			
		if(chdir(strdup(argv[1])) == -1){
			printf("changing directories failed\n");
			printf("Directory is still %s\n", cwd);
		
		}
		return 1;
		
	}else if(strcmp(argv[0], "help") == 0) {
			
		if(arg_position > 1) {
			printf("Too many Arguments for help\n");
		}
		int i;
		for(i = 0; i < 4; i++){
			printf("%d. %s\n", i+1, builtin[i]);
		}
		return 1;	
	
	}else if(strcmp(argv[0], "GuessingGame") == 0) {
		
		guessingGame();
		while ((getchar()) != '\n');
		return 1;
	
	}
	return 0;

}

//Executes shell commands
//If mini-shell is run from here, it will only run for 50 seconds
//as defined by TIME_LIMIT below header file declarations.
//Duration can be adjusted by changing the time there.
void execute_shell_command(char** argv, char** argv_pipe){

	//If no pipe present
	if(pipeCheck == false){
		
		pid_t pid = fork();
		if(pid == -1) 
		{
			perror("Fork Failed");
			exit(1);
		}
       		if(pid == 0){
			execvp(argv[0], argv);
			printf("Command not found. Did you mean something else?\n");
			exit(1);
		}else {
			fork_error_handling(pid);	
		}
	}
	//if pipe present
	else if(pipeCheck == true) 
	{
		pid_t pid;
		int fd[2];
		int child_status;
		pipe(fd);
		//Left side of pipe
		fork_function(argv, fd[1], fd[0]);
		//Right side of pipe
		if((pid = fork()) == 0)
		{	
			close(STDIN_FILENO);
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
			close(fd[1]);			
			execvp(argv_pipe[0], argv_pipe);
			printf("Command not found. Did you mean something else?\n");
			exit(1);	
		}
		else if(pid != 0) {
			close(fd[0]);
			close(fd[1]);
			fork_error_handling(pid);	
			return;	
		}
	}
}

//Seperated fork function to help with error handling in case
//fork runs too long.
//For Left side of pipe
void fork_function(char** argv, int in, int out) {
	pid_t pid = fork();
	if(pid == -1)
	{
        	perror("Fork Failed");
               	exit(1);
        }
        if(pid == 0){
        	close(STDOUT_FILENO);
                dup2(in, STDOUT_FILENO);
                close(in);
                close(out);
		execvp(argv[0], argv);
        	exit(1);
        }else {
		fork_error_handling(pid);
		return;		
	}	
}


void fork_error_handling(pid_t pid) {
	signal(SIGALRM, alarm_handler);
        signal(SIGCHLD, child_handler);
        alarm(TIME_LIMIT);
        pause();
        if(timeout) {
	        int result = waitpid(pid, NULL, WNOHANG);
                if(result == 0) {
        	        kill(pid, 9);
                        wait(NULL);
                }
        }
        else if (child_done) {
                wait(NULL);
        }
}

//handle for SIGCHLD
void child_handler(int sig) {
	child_done = 1;
}
//handler for SIGALRM
void alarm_handler(int sig){
	timeout = 1;
}

//Guessing game where user guesses a number bw 1 and 10
void guessingGame(){
	int counter = 0;
	srand(time(NULL));
	int correctAnswer = rand() % 10 + 1;
	printf("Game %d:\n", counter + 1);
	int userGuess;
	printf("Guess a number between 1 and 10\n");
	bool flag = false;
	while(flag == false){
		scanf("%d", &userGuess);
		if(userGuess == correctAnswer){
			printf("You guessed it right.\n");
			flag = true;
		}
		else if(userGuess > correctAnswer){
			printf("Your guess is higher than correct answer\n");
		}
		else{
			printf("Your guess is lower than correct answer\n");
		}
	}
	return;
}