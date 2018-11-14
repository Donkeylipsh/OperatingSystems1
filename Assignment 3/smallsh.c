/**********************************************************************************************
 * Author: Colin Van Overschelde
 * Date: 5/ 28/ 2018
 * Assignment: Assignment 3
 * Description: smallsh.c is a Unix shell OS that demonstrates using fork and exec to execute
 * 	subprocesses, and also uses signal handling to catch SIGINT and SIGTSTP.
 * 	Built in commands: cd, status, exit
 *********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "dynArray.h"
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Define structs to hold complex data
struct exitStatus{
    int type;
    int status;
};

struct command{
    char *args[512];
    int argCount;
    char *inputFile;
    char *outputFile;
    int background;
};

// Define function prototypes
void readInput(struct command *);
void freeArgs(char **, int);
void exitShell(struct DynArr *, struct command *, struct exitStatus *);
void cdCommand(char **, int);
void statusCommand();
void otherCommand(struct exitStatus *, struct command *, struct DynArr *);

// Glabal flags for use in signals
int Bg_Status = 1;

// Signal handlers
/***********************************************************************************
 * Description: catchSIGINT catches and handles the SIGINT signal. The child process is
 * 	terminated and returns to the shell
 ***********************************************************************************/
void catchSIGINT(int signo){
    // Display the terminating signal message
    char message[30];
    memset(message, '\0', 30);
    sprintf(message, "terminated by signal %d\n", signo);
    write(STDOUT_FILENO, message, 29);

    // Return to shell
}



/***********************************************************************************
 * Description: catchSIGTSTP catches and handles the SIGTSTP signal. The child process is
 * 	terminated and the SIGUSR2 is raised
 ***********************************************************************************/
void catchSIGTSTP(int signo){
    //puts("SIGTSTP caught");
    // When the process terminates, raise SIGUSR2
    raise(SIGUSR2);
}

/***********************************************************************************
 * Description: catchSIGUSR2 catches and handles the SIGUSR2 signal. The Bg_Status flag
 * 	is updated and a notification is written to stdout
 ***********************************************************************************/
void catchSIGUSR2(int signo){
    //puts("SIGUSR2 caught");
    if(Bg_Status){
	// Print entering foreground only mode
	char *message = "Entering foreground-only mode (& is now ignored)\n";
	write(STDOUT_FILENO, message, 49);
	Bg_Status = 0;
    }else{
	char *message = "Exiting foreground-only mode\n";
	write(STDOUT_FILENO, message, 29);
	Bg_Status = 1;
    }
    //char *prompt = ": ";
    //write(STDOUT_FILENO, prompt, 2);
}


/***********************************************************************************
 * Description: main() starts the shell, reads input and delegates commands
 ***********************************************************************************/
int main(){
    

    // Register signal handlers
    // Initialize SIGINT_action struct
    struct sigaction SIGINT_action = {}, SIGTSTP_action = {0}, SIGUSR2_action = {0}, ignore_action = {0};
    // Block other actions while this mask is in place
    SIGINT_action.sa_handler = catchSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;

    SIGTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;

    SIGUSR2_action.sa_handler = catchSIGUSR2;
    sigfillset(&SIGUSR2_action.sa_mask);
    SIGUSR2_action.sa_flags = 0;

    ignore_action.sa_handler = SIG_IGN;

    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    sigaction(SIGUSR2, &SIGUSR2_action, NULL);

    // Initialize command struct to hold the input command    
    struct command *myCommand = (struct command *)malloc(sizeof(struct command));
    myCommand->argCount = 0;
    myCommand->inputFile = malloc(sizeof(char) * 255);
    memset(myCommand->inputFile, '\0', 255);
    myCommand->outputFile = malloc(sizeof(char) * 255);
    memset(myCommand->outputFile, '\0', 255);
    myCommand->background = 0;


    /*char command[2049];
    char *args[512];
    char prompt = ':';
    int argCount = 0;
*/
    struct DynArr *myProcesses = malloc(sizeof(struct DynArr));;
    createArray(myProcesses, 10);

    struct exitStatus *lastExitStatus = malloc(sizeof(struct exitStatus));


    // Start shell loop
    while(1){
	// Read input from user
	// Reset argCount
	myCommand->argCount = 0;
	readInput(myCommand);
	//argCount--;

/*	printf("Executing: %s\n", myCommand->args[0]);
	if(myCommand->argCount > 0){
	    printf("With arguments:\n");
	    int i;
	    for(i = 0; i < myCommand->argCount; i++){
	        printf("%s\n", myCommand->args[i]);
	    }
	}
*/	
	// Check if comment
	if(myCommand->args[0][0] != '#'){
            // Check for built in commands
            char *exitCmd = "exit";
	    char *cdCmd = "cd";
	    char *statusCmd = "status";
            if(strcmp(exitCmd, myCommand->args[0]) == 0){
                //puts("Exiting shell");
	        exitShell(myProcesses, myCommand, lastExitStatus);
            }else if(strcmp(cdCmd, myCommand->args[0]) == 0){
	        cdCommand(myCommand->args, myCommand->argCount);
	    }else if(strcmp(statusCmd, myCommand->args[0]) == 0){
	        statusCommand(lastExitStatus);
	    }else{
	        otherCommand(lastExitStatus, myCommand, myProcesses);
	    }
	}else{
	    //puts("Skipping comment");
	}

	//puts("Execution complete");
	// Free the args array
	freeArgs(myCommand->args, myCommand->argCount);
	memset(myCommand->inputFile, '\0', 255);
	memset(myCommand->outputFile, '\0', 255);
    }
    // Cleanup all allocated memory
    exitShell(myProcesses, myCommand, lastExitStatus);
    return 0;
}

/*******************************************************************************
* Description: readInput accepts input from the terminal and parses it into a command
* Pre-Conditions: n/a
* Post-Conditions: Input is read and parsed into a command
* Parameters: myCommand is a command struct to hold the data input by the user
********************************************************************************/
void readInput(struct command *myCommand){
    //puts("I'm in readInput()");
   // int exitShell = 0;
    // Display prompt
    printf(": ");
    // Get the line input by the user
    char *inputLine = readline(NULL);

    // Check if command contains arguments
    char *cmdChar = strchr(inputLine, ' ');
    char *lastIndex = inputLine;
    if(cmdChar){
//	printf("Initializing argCount\n");
	myCommand->argCount = 0;
	int addLast = 1;
        while(cmdChar){
	    //printf("argCount = %d\n", *argCount);

	    // parse command from string
            int cmdIndex = (int)(cmdChar - lastIndex);
	    
	    // Check if this is the first command in the line
	    if(myCommand->argCount == 0){
		//printf("Assigning command value\n");

                // Copy inputLine to to first element of myCommand->args
		myCommand->args[0] = malloc(cmdIndex + 1);
		strncpy(myCommand->args[0], inputLine, cmdIndex);
		myCommand->args[0][cmdIndex] = '\0';
	    }else{
		// If not first command in line, add argument to myCommand->args
		//printf("Saving argument: %d\n", myCommand->argCount);
		
		// Get the array index for the next argument
		int someArg = myCommand->argCount;

		// Check if the string read is a redirection
  	        char someParam = inputLine[cmdIndex - 1];
		//printf("someParam = %c\n", someParam);
		if(someParam == '<' || someParam == '>'){
		    //puts("Do input redirect");
		    
		    // Copy the remaining string to inputLine
		    strcpy(inputLine, cmdChar + 1);
		
		    // Update lastIndex to point to the front of inputLine
		    lastIndex = inputLine;

		    // Get the index of the next space
		    cmdChar = strchr(inputLine, ' ');
		    // Check if index exists
		    if(cmdChar){
			// If yes, copy the string to the appropriate myCommand value
			cmdIndex = (int)(cmdChar - lastIndex);
			if(someParam == '<'){
			    // Copy to inputFile
			    strncpy(myCommand->inputFile, inputLine, cmdIndex);
			    myCommand->inputFile[cmdIndex] = '\0';
			}else{
			    // Copy to output file
			    strncpy(myCommand->outputFile, inputLine, cmdIndex);
			    myCommand->outputFile[cmdIndex] = '\0';
			}
		    }else{
//			puts("assigning rest of line");
			// If not, copy the rest of the line to the appropriate myCommand value
			if(someParam == '<'){
			    // Copy remaining line to inputFile
			    strcpy(myCommand->inputFile, inputLine);
			    addLast = 0;
			}else{
//			    puts("Assigning outputFile");
			    // Copy remaining line to outputFile
			    strcpy(myCommand->outputFile, inputLine);
			    addLast = 0;
			}
		    }
		    // Redirection Assigned
		    //printf("inputFile = %s\n", myCommand->inputFile);
//		    printf("outputFile = %s\n", myCommand->outputFile);
		    // Decrememnt argCount to offset incrementation at end
		    myCommand->argCount--;
		}else{
		    //puts("Do argument add");
		    //printf(" to index: %d\n", someArg);
		    myCommand->args[someArg] = malloc(cmdIndex + 1);
		    memset(myCommand->args[someArg], '\0', cmdIndex + 1);
		    strncpy(myCommand->args[someArg], inputLine, cmdIndex);
		}
	    }
	    //printf("Updating inputLine: %s\n", inputLine);
	    //printf("cmdChar: %s\n", cmdChar + 1);

	    if(cmdChar){
	        // Update inputLine string
	        strcpy(inputLine, cmdChar + 1);
//	        puts("Copy complete");
	        lastIndex = inputLine;
	    
	        // Get the index of the next space
	        cmdChar = strchr(inputLine, ' ');
	    }
	    //Increment argCount
	    myCommand->argCount++;

//	    printf("Getting next argument\n");
	} 
	//printf("Check read last argument\n");
	
	// Read last argument if more than 1 argument
	if(myCommand->argCount >= 1 && addLast == 1){
	    int someArg = myCommand->argCount;
	    myCommand->args[someArg] = malloc(sizeof(char) * strlen(inputLine));

	    if(strcmp("&", inputLine) == 0){
		myCommand->background = 1;
	    }else{
	        strcpy(myCommand->args[someArg], inputLine);
	
 	        // Increment argCount
	        myCommand->argCount++;
	    }
	    // Add the NULL argument
	    //myCommand->args[myCommand->argCount + 1] = NULL;
	}
	// Add the NULL argument
	myCommand->args[myCommand->argCount] = NULL;
   }else{
	// Copy input string to first arg
	myCommand->args[0] = malloc(strlen(inputLine) + 1);
	strcpy(myCommand->args[0], inputLine);
	myCommand->args[1] = NULL;
    }
    // Free memory allocated by readline()
    free(inputLine);
}


/*******************************************************************************
* Description: freeArgs() is a cleanup function that frees all the arugments that were
* 	allocated as dynamic memory
* Pre-conditions: someArgs is an array of dynamically allocated strings
* Post-Conditions: All strings in someArgs have been released from memory
* Paramaters: someArgs is an array of dynamically allocated strings
* 	size is the number of strings in the array
********************************************************************************/
void freeArgs(char **someArgs, int size){
    int i;
    for(i = 0; i < size - 1; i++){
	free(someArgs[i]);
    }
}

/*******************************************************************************
* Description: exitShell is a cleanup function that kills all children processes and releases
* 	all allocated dynamic memory
* Pre-conditions: n/a
* Post-Conditions: All child processes are ended and all memory has been freed
* Parameters: processess is a array of structs containing the child processes to terminate
* 	myCommand is the struct that stored all commands
* 	lastExitStatus is a struct to be released from memory
********************************************************************************/
void exitShell(struct DynArr *processes, struct command *myCommand, struct exitStatus *lastExitStatus){
    //puts("Exiting shell...");
    // Kill all existing processes
    kill(-processes->myArray[0], SIGINT);

    // Free all memoery that was allocated
    arrayDelete(processes);
    free(processes);
    free(lastExitStatus);
    free(myCommand->inputFile);
    free(myCommand->outputFile);
    free(myCommand);

    exit(0);
}


/*******************************************************************************
* Descriptions: cdCommand executes teh built-in cd command to navigate directories
* Pre-Conditions: args contains 1 or 2 arguments
* Post-Conditions: the cd command is executed
* Parameters: args is an array of strings containing arguments for the cd command
* 	count is the number of arguments in args
********************************************************************************/
void cdCommand(char **args, int count){
    //puts("cdCommand started...");
    //printf("count = %d\n", count);
    char *newPath;

    // Check argument count
    if(count <= 1){
        newPath = getenv("HOME");
    }else if(count == 2){
	char tempPath[255] = "";
	//printf("Changing to: %s\n", args[0]);
	strcat(tempPath, args[1]);
	newPath = tempPath;
    }else{
	puts("Inavalid argument count. cd command requires 0 or 1 arguments");
    }

    int result = chdir(newPath);
    if(result == -1){
	puts("ERROR - Unable to change directory");
    }
}

/*******************************************************************************
* Description: statusCommand executes the built in status command, which displays the
* 	exit status of the most recently exit'ed process
* Pre-Conditions: n/a
* Post-Conditions: The most recent exit status is displayed
* Parameters: someStatus contains the status type and status value to be displayed
********************************************************************************/
void statusCommand(struct exitStatus *someStatus){
    //puts("statusCommand started...");
    if(someStatus->type == 0){
        printf("exit value %d\n", someStatus->status);
    }else if(someStatus->type == 1){
	printf("terminated by signal %d\n", someStatus->status);
    }
}

/*******************************************************************************
* Description: otherCommand executes any command not built into smallsh by forking a new process
* 	and attempting to exec the Unix command
* Pre-Conditions: n/a
* Post-Conditions: The command executes and returns to the shell
* Parameters: lastExitStatus is the struct to store the exit status of the command
* 	myCommand is the struct containing the command to execute
* 	myProcess is the array containing all the pids of child processes
********************************************************************************/
void otherCommand(struct exitStatus *lastExitStatus, struct command *myCommand, struct DynArr *myProcesses){
    //puts("otherCommand started...");
    int status;
    pid_t result;
    pid_t cmd_pid = -5;
    pid_t endId;

    int inputFile = -1;
    int outputFile = -1;
    //puts("In child")
    //fflush(stdout)
    //
    //printf("Input File: %s\n", myCommand->inputFile);
    //printf("Output File: %s\n", myCommand->outputFile);
    // Check for input redirection
    if(myCommand->inputFile[0] != '\0'){	
	// Create new string to hold file name
	char filename[64];
	memset(filename, '\0', 64);
	sprintf(filename, "%s", myCommand->inputFile);
	//printf("Opeing file: %s\n", filename);
	// Open the file to redirect output to
	inputFile = open(filename, O_RDONLY);
    }


    // Check for output redirection
//    printf("Redirection: %s\n", myCommand->outputFile);
    if(myCommand->outputFile[0] != '\0'){
	//puts("Opening output file");

	// Create new string to hold file name
	char filename[64];
	memset(filename, '\0', 64);
	sprintf(filename, "%s", myCommand->outputFile);

	// Open the file to redirect output to
	outputFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    // Fork new process fo command
    cmd_pid = fork();
    switch( cmd_pid ){
	case -1:
	    puts("Hull breach!");
	    break;
	case 0:
	    // Check if input file was opened
	    if( inputFile >= 0 ){
		//puts("Executing with redirected input");
		// If yes, redirect input to read from the open file
		if(dup2(inputFile, 0) < 0){
		    puts("Hull Breach: Unable to redirect input!");
		}
	    }

	    // Check if output file was opened
	    if( outputFile >= 0 ){
		//puts("Executing with redirected output");
		    // If yes, redirect output to the open file
		if(dup2(outputFile, 1) < 0){
		    puts("Hull Breach: Unable to redirect output!");
  		}
	    }
	    // Execute the command
	    execvp(myCommand->args[0], myCommand->args);
	    break;
	default:
 	    //puts("In parent");
	    //fflush(stdout);
	    //kill(cmd_pid, SIGINT);
	    arrayAdd(myProcesses, cmd_pid);
	    if(Bg_Status && myCommand->background){
		result = waitpid(cmd_pid, &status, WNOHANG);
	    }else{
	        result = waitpid(cmd_pid, &status, 0);
	    }
	    if(WIFEXITED(status) != 0){
		lastExitStatus->type = 0;
		lastExitStatus->status = WEXITSTATUS(status);
	    }else{
		puts("Setting signal type");
		lastExitStatus->type = 1;
		puts("Setting signal status");
		lastExitStatus->status = WTERMSIG(status);
	    }
	    // Close the input and output files
	    if(inputFile >= 0){
		close(inputFile);
	    }
	    if(outputFile >= 0){
		close(outputFile);
	    }
	    //puts("Child has completed");
	    fflush(stdout);
    }

}
