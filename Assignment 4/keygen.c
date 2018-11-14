/****************************************************************************************************
 * Author: Colin Van Overschelde
 * Date: 6/9/2018
 * Assignment: Program 4 - OTP
 * Description: keygen.c generates a random sequence of upper case letters and the space character
 * 	of length specified by a command line argument, an dprints the string to stdout
 ***************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Function definitions
int convertInput(char *);
void generateKey(int, char*); 

/*************************************************************************************************
 * Description: main executes the program
 * Parameters: argv[1] is the number of characters to generate in the key
 *************************************************************************************************/
int main(int argc, char *argv[]){
    //printf("Command line argument: %s\n", argv[1]);
    int keyLength = convertInput(argv[1]);
    //printf("Key length = %d\n", keyLength);

    char someKey[keyLength + 2];
    memset(someKey, '\0', keyLength + 2);
    generateKey(keyLength, someKey);
    //printf("%d\t%c", keyLength, someKey[keyLength]);
    //printf("%d\t%c", keyLength + 1, someKey[keyLength + 1]);
    printf("%s", someKey);
    //fflush(stdout);

    return 0;
}

/********************************************************************************
 * Description: convertInput converts the command line argument string to an integer
 * Pre-conditions: stringNum is a string of numeric characters
 * Post-conditions: returns the integer value of stringNum
 * Parameters: stringNum is a string of numeric characers
 ********************************************************************************/
int convertInput(char *stringNum){
    // Int to hold the converted string value
    int num;
    // Read from input string and store in num
    sscanf(stringNum, "%d", &num);
    //printf("Input number: %d\n", num);
    return num;
}

/*********************************************************************************
 * Description: generateKey generates a string of random upper case characters including the
 * 	space character and stores the value in the string passed as an argument
 * Pre-conditions: length is less than or equal to the length of someKey
 * Post-conditions: someKey contains length random upper case characters
 * Parameters: length is the number of random characters to generate
 * 	someKey is the string to store the random upper case characters
 *********************************************************************************/
void generateKey(int length, char *someKey){
    // Seed random generator
    srand(time(0));

    //printf("Generating %d random characters\n", length);
    //printf("Index\tValue\n");
    // Generate length random uppercase letters and add to someKey
    int i;
    for(i = 0; i < length; i++){
	// Generate a new random number between 0 and 26 inclusive
	int randomNum = rand();
	randomNum = randomNum % 26;
	char nextChar;
	// Convert number to upper case character or space
	if(randomNum == 0){
	    nextChar = ' ';
	}else{
	    nextChar = randomNum + 64;
	}
	// Assign the value to the output string
	someKey[i] = nextChar;
	//printf("%d\t%c\n", i, someKey[i]);
    }

    // Add new line character to end of key
    someKey[length] = '\n';
}
