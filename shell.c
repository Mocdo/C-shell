// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
unsigned int hist_index;			//p3

char hist_sign[1];







int cd(char *tokens[]);
int help(char * params[]);
int history_func(char * params[]);
int history_sign(char * params[],_Bool *in_background);

char prev_dir[COMMAND_LENGTH];



































/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	int i = 0;
	if(buff[0] == '!'){

		tokens[0]=hist_sign;
		i++;
		token_count++;
		
	}
	for (; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	tokens[token_count+1] = NULL;
	return token_count;
}















/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */




void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;
	char cwd[COMMAND_LENGTH-1];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("error: cannot get current dir");
		exit(0);
	}
	
	int length = -99;
	
	// Read input
	do{
		
		
		//write(STDOUT_FILENO, "$ ", strlen("$ "));
		write(STDOUT_FILENO, cwd, strlen(cwd));
		write(STDOUT_FILENO, "> ", strlen("> "));
		
	
		length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
		
		if (length < 0 && (errno !=EINTR)) {
			perror("Unable to read command from keyboard. Terminating.\n");
			exit(-1);
		}
	}while((length < 0 && errno ==EINTR));


	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}



	// Adding in history			



	history[(hist_index % HISTORY_DEPTH)][0] = '\0';
	strcat(history[(hist_index % HISTORY_DEPTH)],buff);
	hist_index++;
					

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}







/* Signal handler function */
void handle_SIGINT(){
	write(STDOUT_FILENO, "\n", strlen("\n"));
	char *temp[3];
	temp[2]=NULL;
	temp[1]=NULL;
	help(temp);
	return;
	//exit(0);

}

int exec_cmd(char*tokens[],_Bool *in_background){
	
	if(tokens[0]==NULL){return 0;}


	if(strcmp(tokens[0], "exit") == 0) {

			
		exit(1);
		return 0;
	}else if(strcmp(tokens[0], "pwd") == 0) {
		char cwd[COMMAND_LENGTH];
		if(getcwd(cwd, sizeof(cwd)) != NULL) {
			write(STDOUT_FILENO, " current directory is: ", strlen(" current directory is: "));
			//write(STDOUT_FILENO, cwd, strlen(cwd));
			//write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		else {
			perror("getcwd() error\n");
		}
		return 0;
	}else if(strcmp(tokens[0], "cd") == 0) {	
		cd(tokens);
		return 0;
	}else if(strcmp(tokens[0], "help") == 0) {
		help(tokens);
		
		return 0;
	}else if(strcmp(tokens[0], "!") == 0) {
		history_sign(tokens,in_background);
		return 0;
	}else if(strcmp(tokens[0], "history") == 0) {
		history_func(tokens);
		
		return 0;
	}else if(strcmp(tokens[0], "ls")==0 || strcmp(tokens[0], "w")==0){
		return 0;
	}else if(tokens[0][0] == '!') {
		write(STDOUT_FILENO, "if you want to run commands from the history list, do not use space before \'!\' \n", strlen("if you want to run commands from the history list, do not use space before \'!\' \n"));
		
		return 0;
	}else{
		write(STDOUT_FILENO, "\'", strlen("\'"));
		write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
		write(STDOUT_FILENO, "\' is not a vaild command\n", strlen("\' is not a vaild command\n"));
		return 1;
	}

	return 1;
}
















/**
 * Main and Execute Commands
 */




int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	hist_index = 0;				//p3
	hist_sign[0] = '!';

	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);


	while (true) {

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().


		

		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);

		// DEBUG: Dump out arguments:
//		for (int i = 0; tokens[i] != NULL; i++) {
//			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
//			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
//			write(STDOUT_FILENO, "\n", strlen("\n"));
//		}
//		if (in_background) {
//			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
//		}
		



		


	

		exec_cmd(tokens, &in_background);

		
		int status;
		pid_t  var_pid;
		var_pid = fork();
		if (var_pid < 0) {
			fprintf (stderr, "fork Failed");
			exit(-1);
		}else if (var_pid == 0) { /* child process */
			if(execvp(tokens[0], tokens)==-1){exit(-1);}
			exit(0);
		}else { /* parent process */
			waitpid(var_pid, &status, 0);
			//printf ("Child %d Completed\n", var_pid);
		}
		

		


		


		



		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

		
	}
	return 0;
}





int help(char * params[]){

	if(params[2]!=NULL){
		fprintf (stderr, " Failed, more than one arguement\n");
		return 0;
	}

	if(params[1]==NULL){	
			write(STDOUT_FILENO, "\'cd\' : Change the current working directory. \n", strlen("\'cd\' : Change the current working directory. \n"));
			write(STDOUT_FILENO, "\'exit\' : Exit the shell program. \n", strlen("\'exit\' : Exit the shell program. \n"));
			write(STDOUT_FILENO, "\'pwd\' : Display the current working directory. \n", strlen("\'pwd\' : Display the current working directory. \n"));
			write(STDOUT_FILENO, "\'help\' : Display help information on internal commands. \n", strlen("\'help\' : Display help information on internal commands. \n"));
			write(STDOUT_FILENO, "\'history\' : Display the 10 most recent commands executed in the shell.\n", strlen("\'history\' : Display the 10 most recent commands executed in the shell.\n"));
			write(STDOUT_FILENO, "\'!\' : Allow users to run commands directly from the history list.\n", strlen("\'!\' : Allow users to run commands directly from the history list.\n"));		
	}else if(strcmp(params[1],"cd")==0){
		write(STDOUT_FILENO, "\'cd\' is a builtin command for changing the current working directory.\n", strlen("\'cd\' is a builtin command for changing the current working directory.\n"));
	}else if(strcmp(params[1],"pwd")==0){
		write(STDOUT_FILENO, "\'pwd\' is a builtin command for displaying the current working directory.\n", strlen("\'cd\' is a builtin command for displaying the current working directory.\n"));
	}else if(strcmp(params[1],"exit")==0){
		write(STDOUT_FILENO, "\'exit\' is a builtin command for exiting the shell program.\n", strlen("\'exit\' is a builtin command for exiting the shell program.\n"));
	}else if(strcmp(params[1],"history")==0){
		write(STDOUT_FILENO, "\'history\' is a builtin command for displaying the 10 most recent commands executed in the shell.\n", strlen("\'history\' is a builtin command for displaying the 10 most recent commands executed in the shell.\n"));		
	}else if(strcmp(params[1],"!")==0){
		write(STDOUT_FILENO, "\'!\' is a builtin command for allowing users to run commands directly from the history list.\n", strlen("\'!\' is a builtin command for allowing users to run commands directly from the history list.\n"));		
	}else{
			write(STDOUT_FILENO, "\'", strlen("\'"));
			write(STDOUT_FILENO, params[1], strlen(params[1]));
			write(STDOUT_FILENO, "\' is an external command or application\n", strlen("\' is an external command or application\n"));
	}

	return 0;
}




int history_func(char * params[]){
	if(params[1]!=NULL){
		fprintf (stderr, " Failed, more than one arguement");
		return 0;
	}

	if(hist_index <= 9){
		for(int i = hist_index-1; i>=0;i--){

			char temp_int_to_string[10];
			sprintf(temp_int_to_string, "%d \t", i);
			write(STDOUT_FILENO, temp_int_to_string, strlen(temp_int_to_string));
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		return 0;
	}	
	
	int temp = hist_index % HISTORY_DEPTH-1;
	for(int i = 10;i > 0;i-- ){
		char temp_int_to_string[10];
		sprintf(temp_int_to_string, "%d \t", hist_index-11+i);
		write(STDOUT_FILENO, temp_int_to_string, strlen(temp_int_to_string));
		write(STDOUT_FILENO, history[(temp+i)% HISTORY_DEPTH], strlen(history[(temp+i)% HISTORY_DEPTH]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}

	return 0;


}


int history_sign(char * params[],_Bool *in_background){

	if(params[2]!=NULL){
		fprintf (stderr, " Failed, more than one arguement\n");
		return 0;
	}
	if(params[1]==NULL){
		fprintf (stderr, " Failed, need one arguement\n");
		return 0;
	}

	int hist_stack_index = (hist_index-1) % HISTORY_DEPTH;

	if(strcmp(params[1],"!")==0){
		if(hist_index==1){
			fprintf (stderr, " Failed, no previous command\n");
			return 0;
		}
		
		write(STDOUT_FILENO, history[(hist_stack_index-1)%HISTORY_DEPTH], strlen(history[(hist_stack_index-1)%HISTORY_DEPTH]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
		char *tokens[NUM_TOKENS];
		// Tokenize (saving original command string)
		int token_count = tokenize_command(history[(hist_stack_index-1)%HISTORY_DEPTH], tokens);
		if (token_count == 0) {
			return -1;
		}
		//execute
		
		exec_cmd(tokens, in_background);

		int status;
		pid_t  var_pid;
		var_pid = fork();
		if (var_pid < 0) {
			fprintf (stderr, "fork Failed");
			exit(-1);
		}else if (var_pid == 0) { /* child process */
			if(execvp(tokens[0], tokens)==-1){exit(-1);}
			exit(0);
		}else { /* parent process */
			waitpid(var_pid, &status, 0);
			//printf ("Child %d Completed\n", var_pid);
		}

		return 0;	
	}
	int number = atoi(params[1]);
	if (number == 0 && params[1][0] != '0'){
		fprintf (stderr, " Failed, arguement is not a number\n");
		return 0;
	}
	
	number = hist_index-1 - number;
	if(number>9||number<=0){
		fprintf (stderr, " Failed, arguement out of range\n");
		return 0;
	}
	write(STDOUT_FILENO, history[(hist_stack_index-number)%HISTORY_DEPTH], strlen(history[(hist_stack_index-number)%HISTORY_DEPTH]));
	write(STDOUT_FILENO, "\n", strlen("\n"));
	char *tokens[NUM_TOKENS];
	// Tokenize (saving original command string)
	int token_count = tokenize_command(history[(hist_stack_index-number)%HISTORY_DEPTH], tokens);
	if (token_count == 0) {
		return 0;
	}
	if(strcmp(tokens[1],params[1])==0){
		write(STDOUT_FILENO, "command is calling itself, cannot re-execute the command...\n", strlen("command is calling itself, cannot re-execute the command...\n"));
		return 0;
	}
	//execute
	
	exec_cmd(tokens, in_background);
	
	int status;
	pid_t  var_pid;
	var_pid = fork();
	if (var_pid < 0) {
		fprintf (stderr, "fork Failed");
		exit(-1);
	}else if (var_pid == 0) { /* child process */
		if(execvp(tokens[0], tokens)==-1){exit(-1);}
		exit(0);
	}else { /* parent process */
		waitpid(var_pid, &status, 0);
		//printf ("Child %d Completed\n", var_pid);
	}


	

	return 0;
}




int cd(char *tokens[]){

	uid_t uid = getuid(); 
	struct passwd *temp_pw = getpwuid(uid);
	char *home_dir = temp_pw->pw_dir;

	
	char cwd[COMMAND_LENGTH];	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("error: cannot get current dir");
		return 1;
	}



	if(tokens[1]==NULL){
		if(chdir(home_dir) != 0) {
			write(STDOUT_FILENO, "error: Invalid directory.\n", strlen("error: Invalid directory.\n"));
			return 0;
		}
		strcpy(prev_dir, cwd);
		return 0;
	}	
	if(tokens[1][0] == '~'){
		strcat(home_dir,tokens[1] + sizeof(char));
		if(chdir(home_dir) != 0) {
			write(STDOUT_FILENO, "error: Invalid directory.\n", strlen("error: Invalid directory.\n"));
			return 0;
		}
		strcpy(prev_dir, cwd);
		return 0;
	}

	if(strcmp(tokens[1],"-")==0){
		if(chdir(prev_dir) != 0) {
			write(STDOUT_FILENO, "error: Invalid directory.\n", strlen("error: Invalid directory.\n"));
			return 0;
		}
		strcpy(prev_dir, cwd);
		return 0;
	}

	
	if(chdir(tokens[1]) != 0) {
		write(STDOUT_FILENO, "SHELL: Invalid directory.\n", strlen("SHELL: Invalid directory.\n"));
		return 0;
	}
	strcpy(prev_dir, cwd);
	return 0;
}




