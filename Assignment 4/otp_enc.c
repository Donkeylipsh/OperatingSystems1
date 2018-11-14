/*********************************************************************************************
 * Author: Colin Van Overschelde
 * Date: 6/10/2018
 * Assignment: OTP
 * Description: otp_enc.c is a client application that demonstrates encrypting a plain text string
 * 	using a server to perform the encryption. otp_enc accepts as arugments the filename for
 * 	for the file containing th eplain text to be encrypted, and the filename for the file
 * 	that contains the key to be used for the encryption.
 ********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <sys/ioctl.h>

// Function definitions
int validateInput(char *, int *, char *);
void readFile(char *, char *);

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(0); 
}


int main(int argc, char *argv[])
{
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[1001];
    
    // Check usage & args
    if (argc < 4) { 
	fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
	exit(0); 
    }

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number 
    // Convert the machine name into a special form of address
    serverHostInfo = gethostbyname("localhost");   

    // Validate input contents
    // Check if plaintext file contains invalid characters(i.e. Not A-Z or ' ')'
    int minLength;
    char *inputString = (char *)malloc(sizeof(char) * 75000);
    memset(inputString, '\0', sizeof(char) * 75000);
    int isInputValid = validateInput(argv[1], &minLength, inputString);

    if( isInputValid == 0 ){
	fprintf(stderr, "otp_enc error: input contains bad characters\n");
	exit(1);
    }else if(isInputValid == -1){
	fprintf(stderr, "otp_enc error: unable to open input file\n");
	exit(1);
    }   
 
    // Validate contents of the key file
    // Set minLength to the number of characters in the input file
    int keyLength;
    char *keyString = (char *)malloc(sizeof(char) * 75000);
    memset(keyString, '\0', sizeof(char) * 75000);
    int isKeyValid = validateInput(argv[2], &keyLength, keyString);

    // Check if key file was validated
    if( isKeyValid == 0){
	fprintf(stderr, "otp_enc error: key contains bad characters\n");
	exit(1);
    }
    if( keyLength < minLength ){
	fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
	exit(1);
    }

    // Check if host exists
    if (serverHostInfo == NULL) { 
	// If not, print erro and exit
	fprintf(stderr, "CLIENT: ERROR, no such host\n");
	exit(0); 
    }

    // Copy in the address
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) {
	error("CLIENT: ERROR opening socket");
    }
	
    // Connect to server
    // Connect to socket
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
	// If not connected, display error
	error("CLIENT: ERROR connecting");
    }

    // Send request to server
    // Send size of inputString
    char inputSize[20];
    memset(inputSize, '\0', 20);
    sprintf(inputSize, "%d", minLength);
    charsWritten = send(socketFD, inputSize, 20, 0);


    // Check if write was successful
    if (charsWritten < 0){
	// If not, display error
	error("CLIENT: ERROR writing to socket");
    }

    // Check if all characters were sent
    if (charsWritten < strlen(inputSize)){
	// If not, display error
//	printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    // Clear out buffer for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at the end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    // Check if read was successful
    if (charsRead < 0){
	// If not, display error
	error("CLIENT: ERROR reading from socket");
    }
    // Display response from server
//    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Send the inputString
    int charsLeft = minLength;
    int charIndex = 0;
    while(charsLeft > 0){
//	printf("chars left: %d\n", charsLeft);
        
	char subbuff[1001];
	memset(subbuff, '\0', 1001);
	memcpy( subbuff, &inputString[charIndex], 1000);
	int subbuffSize = strlen(subbuff);
	//printf("\nSub-buff: %s\n\n", subbuff);
	
	charsWritten = send(socketFD, subbuff, 1000, 0);
	charIndex += 1000;
        
        // Check if write was successful
        if (charsWritten < 0){
	    // If not, display error
	    error("CLIENT: ERROR writing to socket");
        }

        // Check if all characters were sent
        if (charsWritten < subbuffSize){
	    // If not, display error
//	    printf("CLIENT: WARNING: Not all data written to socket!\n");
        }

	charIndex += charsWritten;
	charsLeft -= charsWritten;
	
	// Wait for data to leave system
	int checkSend = -5;	// Bytes remaining in send buffer
	// Loop until send buffer is empty
	do{
	    ioctl(socketFD, TIOCOUTQ, &checkSend);
	}while (checkSend > 0);

	// Check if we stopped loop becaus eof error
	if (checkSend < 0){
	    error("ioctl error");
	}

        // Get return message from server
        // Clear out buffer for reuse
        memset(buffer, '\0', sizeof(buffer));
        // Read data from the socket, leaving \0 at the end
        charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

        // Check if read was successful
        if (charsRead < 0){
	    // If not, display error
	    error("CLIENT: ERROR reading from socket");
        }
        // Display response from server
       // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	
	//charsLeft = charsLeft - charsWritten;
//	printf("CLIENT: Chars left: %d\n", charsLeft);
    }
/*
    printf("************************************************************\n");
    printf("Input Strint transmission complete\n");
    printf("************************************************************\n");
*/
    /*******************************************************************************************
     * Send the keyString to the server
     *******************************************************************************************/
    // Send size of keyString
    //char inputSize[20];
    memset(inputSize, '\0', 20);
  //  printf("CLIENT: Key size = %d\n", keyLength);
    sprintf(inputSize, "%d", keyLength);
    charsWritten = send(socketFD, inputSize, 20, 0);


    // Check if write was successful
    if (charsWritten < 0){
	// If not, display error
	error("CLIENT: ERROR writing to socket");
    }

    // Check if all characters were sent
    if (charsWritten < strlen(inputSize)){
	// If yes, display warning
//	printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    // Clear out buffer for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at the end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    // Check if read was successful
    if (charsRead < 0){
	// If not, display error
	error("CLIENT: ERROR reading from socket");
    }
    // Display response from server
  //  printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Send the keyString
    charsLeft = keyLength;
    charIndex = 0;
    while(charsLeft > 0){
	 
	char subbuff[1001];
	memset(subbuff, '\0', 1001);
	memcpy( subbuff, &keyString[charIndex], 1000);
	int subbuffSize = strlen(subbuff);
	//printf("\nSub-buff: %s\n\n", subbuff);
	
	charsWritten = send(socketFD, subbuff, 1000, 0);
	//charIndex += 1000;
        
        // Check if write was successful
        if (charsWritten < 0){
	    // If not, display error
	    error("CLIENT: ERROR writing to socket");
        }

        // Check if all characters were sent
        if (charsWritten < subbuffSize){
	    // If not, display error
//	    printf("CLIENT: WARNING: Not all data written to socket!\n");
        }
	
	// Wait for data to leave system
	int checkSend = -5;	// Bytes remaining in send buffer
	// Loop until send buffer is empty
	do{
	    ioctl(socketFD, TIOCOUTQ, &checkSend);
	}while (checkSend > 0);

	// Check if we stopped loop becaus eof error
	if (checkSend < 0){
	    error("ioctl error");
	}

        // Get return message from server
        // Clear out buffer for reuse
        memset(buffer, '\0', sizeof(buffer));
        // Read data from the socket, leaving \0 at the end
        charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	charsWritten = atoi(buffer);

        // Check if read was successful
        if (charsRead < 0){
	    // If not, display error
	    error("CLIENT: ERROR reading from socket");
        }
        // Display response from server
  //      printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	
	charIndex += charsWritten;
	charsLeft -= charsWritten;

	//charsLeft = charsLeft - charsWritten;
//	printf("CLIENT: Chars left: %d\n", charsLeft);
    }

  //  printf("CLIENT: All data sent to server\n");

    /*******************************************************************************
     * Get cypher text from server
     *******************************************************************************/
   // printf("CLIENT: Listening for cypher text...\n");

    // Get cypher size
    // Clear out the buffer
    memset(buffer, '\0', 1001);
    charsRead = recv(socketFD, buffer, 1000, 0);


    // Check if at least 1 character was sent
    if (charsRead < 0){
	// If not, write error
	error("ERROR reading from socket");
    }

    // Send a Success message back to the server
    charsWritten = send(socketFD, "I am the client, and I got your message", 39, 0);

    //Check if success message sent successfully
    if (charsRead < 0){
	// If not, write error
	error("ERROR writing to socket");
    }

    // Displat the message received
   // printf("CLIENT: I received this cypher size from the server: \"%s\"\n", buffer);

    int cypherSize = atoi(buffer);
    char cypherText[cypherSize + 2];
    memset(cypherText, '\0', cypherSize + 2);
    int cypherIndex = 0;
    //printf("CLIENT: Starting cypher receive loop...\n");
    while(cypherSize > 0){
	// Clear the buffer
	memset(buffer, '\0', 1001);

/**************************** this is error *******************************/
	// Receive the message form the server
	charsRead = recv(socketFD, buffer, 1000, 0);

	// Check if at least 1 character was sent
	if (charsRead < 0){
	    // If not, write error
	    error("ERROR reading from socket");
	}

	// Displat the message received
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	// Copy the received string to cypherText
	memcpy(&cypherText[cypherIndex], &buffer, 1000);
	// Decrement cypherSize
	cypherSize -= 1000;
	cypherIndex += 1000;

	// Return the number of characters received
	char receivedSize[20];
	memset(receivedSize, '\0', 20);
	sprintf(receivedSize, "%d", charsRead);
	charsRead = send(socketFD, receivedSize, strlen(receivedSize), 0);
    }

    /*******************************************************************************
     * Print cypher text to stdout
     ******************************************************************************/
    //printf("CLIENT: cypher text received: \n");
    printf("%s", cypherText);

    // Close the socket
    close(socketFD);
 
    // Free memory allocated with readline()
    free(inputString);
    free(keyString);

    return 0;
}

/*****************************************************************************************************
 * Description: validateInput checks the contents of the plaintext file for valid contents.
 * 	Returns 1 if valid, 0 if file contains invalid characters and -1 if unable to open file
 * Pre-conditions:
 * Post-conditions:
 *****************************************************************************************************/
int validateInput(char *inputFile, int *size, char *someText){

    //printf("Validating input\n");
    //fflush(stdout);

    // Create the file descriptor
    int input_fd = open(inputFile, O_RDONLY);
    if(input_fd == -1){
	// Close the file
	close(input_fd);
	// Return -1 for unable to open
	return -1;
    }

    // Read the entire contents for the file
    *size = 0;
    ssize_t bytesread = 0;
    unsigned char myBuffer[PIPE_BUF];
    char *moreChars = NULL;
    int limit = 1000;
    while((bytesread = read(input_fd, myBuffer, PIPE_BUF)) > 0){
	*size += bytesread;
	// Concat the characters read to the input string
	strcat(someText, myBuffer);
	someText[*size - 1] = '\0';
    }
    //printf("Size read: %d\n", *size);

    // Save the size of the line read
    *size = strlen(someText);
    //printf("Size saved: %d\n", *size);

    int isValid = 1;
    int index = 0;
    // Loop through each character in the string read from file
    while( isValid && index < sizeof(someText) ){
	if(someText[index] < 65 || someText[index] > 90){
	    // Check if space character
	    if(someText[index] != ' ' && someText[index] != '\n'){
	        // Set notValid = 1 to exit loop
	        isValid = 0;
	    }
	}
	// Increment index to move to next character
	index++;
    } 


    // Return 1 for success
    return isValid;
}
