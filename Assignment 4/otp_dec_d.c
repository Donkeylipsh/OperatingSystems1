/**********************************************************************************************
 * Author: Colin Van Overschelde
 * Date: 6/10/2018
 * Assignment: OTP
 * Description: otp_dec_d.c is a server program that decrypts an encrypted string and returns it
 * 	to a client. otp_dec_d demonstrates forking a child process to perform the decryption
 * 	while the partent returns to listening for new connections.
 ********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
    socklen_t sizeOfClientInfo;
    char buffer[1001];
    struct sockaddr_in serverAddress, clientAddress;

    if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

    // Set up the address struct for this process (the server)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0){
        error("ERROR opening socket");
    }

    // Enable the socket to begin listening
    // Connect socket to port
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
	error("ERROR on binding");
    }
    
    // Flip the socket on - it can now receive up to 5 connections
    listen(listenSocketFD, 5);

    while (1){
        // Accept a connection, blocking if one is not available until one connects
        // Get the size of the address for the client that will connect
        sizeOfClientInfo = sizeof(clientAddress);
        // Accept connection
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        pid_t spawnpid = -5;
	int childExitMethod = -5;
	spawnpid = fork();
        if(spawnpid == 0){   
            // Check if connected
            if (establishedConnectionFD < 0){
	        // If not, write error
	        error("ERROR on accept");
            }
	        	
            /*******************************************************************************
             * Get the size of the cypher text input string from the client
             * *****************************************************************************/
            memset(buffer, '\0', 1001);
            // Read the client's message from the socket
            charsRead = recv(establishedConnectionFD, buffer, 1000, 0);
    
            // Check if at least 1 character was sent
            if (charsRead < 0){
	        // If not, write error
	        error("ERROR reading from socket");
            }
            // Display the message received
            //printf("SERVER: I received this from the client: \"%s\"\n", buffer);

            // Send a Success message back to the client
            charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0);
            // Check if success message sent successfully
            if (charsRead < 0){
	        // If not, write error
	        error("ERROR writing to socket");
            }

	    int msgSize = atoi(buffer);
	    char totalMsg[75000];
	    int curIndex = 0;
	    // Initialize totalMsg
	    memset(totalMsg, '\0', 75000);
	    int charsLeft = msgSize; 
	    /*******************************************************************************
 	     * Get the plain text input string from the client
 	     *******************************************************************************/
	    //printf("Listening for %d characters of cypher text...\n", msgSize);
	    while(charsLeft > 0){
		// Clear the buffer
		memset(buffer, '\0', 1000);
		// Receive message from client
		charsRead = recv(establishedConnectionFD, buffer, 1000, 0);
	
                // Check if at least 1 character was sent
                if (charsRead < 0){
	            // If not, write error
	            error("ERROR reading from socket");
                }
                // Display the message received
		//printf("SERVER: I received this from the client: \"%s\"\n", buffer);
		//printf("SERVER: I received %d characters\n", charsRead);
		// Check if client terminates input
		/*if (strcmp(buffer, "quit") == 0){
		    break;
		}*/
		int buffSize = strlen(buffer);
		// Copy the received string to totalMsg
		memcpy(&totalMsg[curIndex], &buffer, 1000);
		// Decrement charsLeft
		charsLeft -= 1000;
		// Increment curIndex
		curIndex += 1000;
		char receivedSize[20];
		memset(receivedSize, '\0', 20);
		//itoa(msgSize, receivedSize, 10);
		sprintf(receivedSize, "%d", charsRead);

		// Send characters received to client
		charsRead = send(establishedConnectionFD, receivedSize, strlen(receivedSize), 0);
	    }
	    // Display the total message received
	    //printf("SERVER: Total message received: \"%s\"\n", totalMsg);
	    
	    /*****************************************************************************************
 	     * Get the size of the key string from the client
 	     *****************************************************************************************/
	    // Clean out the buffer
	    memset(buffer, '\0', 1000);
	    // Receive next message from client
	    charsRead = recv(establishedConnectionFD, buffer, 1000, 0);

            // Check if at least 1 character was sent
            if (charsRead < 0){
	        // If not, write error
	        error("ERROR reading from socket");
            }
            // Display the message received
            //printf("SERVER: I received this key size from the client: \"%s\"\n", buffer);

            // Send a Success message back to the client
            charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0);
            // Check if success message sent successfully
            if (charsRead < 0){
	        // If not, write error
	        error("ERROR writing to socket");
            }

	    int keySize = atoi(buffer);
	    curIndex = 0;
	    // Initialize totalKey
	    char totalKey[75000];
	    memset(totalKey, '\0', 75000);
	    
	    //printf("SERVER: Listening for %d characters of key text...\n", keySize);
	    while(keySize > 0){
		// Clear the buffer
		memset(buffer, '\0', 1000);
		// Receive message from client
		charsRead = recv(establishedConnectionFD, buffer, 1000, 0);
	
                // Check if at least 1 character was sent
                if (charsRead < 0){
	            // If not, write error
	            error("ERROR reading from socket");
                }
                // Display the message received
		//printf("SERVER: I received this from the client: \"%s\"\n", buffer);

		// Check if client terminates input
		if (strcmp(buffer, "quit") == 0){
		    break;
		}

		// Copy the received string to totalMsg
		memcpy(&totalKey[curIndex], &buffer, charsRead);
		// Decrement msgSize
		keySize = keySize - charsRead;
		curIndex += charsRead;
		char receivedSize[20];
		memset(receivedSize, '\0', 20);
		sprintf(receivedSize, "%d", charsRead);

		charsRead = send(establishedConnectionFD, receivedSize, strlen(receivedSize), 0);
	    }
	    // Display the total message received
	    //printf("SERVER: Total key received: \"%s\"\n", totalKey);	    

	    //printf("SERVER: Total key processed, moving to dencryption\n");

	    /***********************************************************************************
 	     * Encrypt the input text
 	     ***********************************************************************************/ 
	    //printf("msgSize = %d\n", msgSize);
	    char cypherText[msgSize + 2];
	    memset(cypherText, '\0', msgSize + 2);
	    //printf("SERVER: Starting dencryption...\n");
	    // Loop through each character in totalMsg
	    int cypherIndex;
	    for(cypherIndex = 0; cypherIndex < msgSize; cypherIndex++){
		// Perform modular addition on current character
		char nextChar = totalMsg[cypherIndex];
		char nextKey = totalKey[cypherIndex];
		// Check if the character is a space character
		if(nextChar == ' '){
		    // If yes, set value to zero
		    nextChar = 0;
		}else{
		    // If not, subtract 64 form value to get a number between 1 and 26 inclusive
		    nextChar -= 64;
		}
		// Check if the key is a space
		if(nextKey == ' '){
		    // If yes, set value to zero
		    nextKey = 0;
		}else{
		    // If not, suptract 64 from value to get a number between 1 and 26 inclusive
		    nextKey -= 64;
		}
		char cypherChar = nextChar - nextKey;
		//printf("Cypher sum: %d\n", cypherChar);
		// Check if cypherChar is out of range
		if( cypherChar < 0 ){
		    // If yes, subtract 27
		    cypherChar += 27;
		}

		// Add normalizat cypher char
		if( cypherChar == 0 ){
		    cypherChar = ' ';
		}else{
		    cypherChar += 64;
		}
		// Assign value of next character in cypher text
		cypherText[cypherIndex] = cypherChar;
	    }
	    // Append new line to end of string
	    cypherText[msgSize] = '\n';

	    //printf("SERVER: Cypher generated: %s\n", cypherText);


  	    /*****************************************************************************
 	     *  Send characters received to client
 	     *****************************************************************************/
	    // Send size of  plain Text
	    char cypherSize[20];
	    memset(cypherSize, '\0', 20);
	    sprintf(cypherSize, "%d", msgSize + 2);
	    //printf("SERVER: Sending cypher size = %s\n", cypherSize);
	    charsRead = send(establishedConnectionFD, cypherSize, strlen(cypherSize), 0);

	    // Check if write was successful
	    if (charsRead < 0){
		// If not, display error
		error("SERVER: ERROR writing to socker");
	    }

	    // Check if all characters were sent
	    if (charsRead < strlen(cypherSize)){
		// If yes, display warning
		//printf("SERVER: WARNING: Not all data written to socket!\n");
	    }

	    // Display response from client
	    // Clear out buffer for reuse
	    memset(buffer, '\0', strlen(buffer));
	    // Read message from client
	    charsRead = recv(establishedConnectionFD, buffer, strlen(buffer) - 1, 0);

	    // Check if read was successful
	    if(charsRead < 0){
		// If not, display error
		error("SERVER: ERROR reading from socket");
	    }
	    // Display response from server
	    //printf("SERVER: I recieved this from the client: \"%s\"\n", buffer);

	    /*******************************************************************************
 	    * Send cypher text to client
 	    *******************************************************************************/
	    charsLeft = msgSize + 1;
	    //printf("Server: Starting cypher transmission: %d\n", charsLeft);
	    cypherIndex = 0;
	    int charsWritten;
	    while(charsLeft > 0){
		char subbuff[1001];
		memset(subbuff, '\0', 1001);
		memcpy(subbuff, &cypherText[cypherIndex], 1000);
		int subbuffSize = strlen(subbuff);

		// Send the subbuff to the client
		charsWritten = send(establishedConnectionFD, subbuff, 1000, 0);
		
		// Check if write was successful
		if (charsWritten < subbuffSize){
		    // If not, display error
		    error("SERVER: ERROR writing to socket");
		}

		// Check if all characters were sent
		if (charsWritten < subbuffSize){
		    // If not, display error
		    //printf("SERVER: WARNING: Not all data written to socket!\n");
		}
		// Wait for data to leave system
		int checkSend = -5;	// Bytes remaining in send buffer
		// Loop until send buffer is empty
		do{
		    ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);
		}while (checkSend < 0);

	        // Check if we stopped loop because of error
	        if (checkSend < 0){
		    error("ioctl error");
	        }

	        // Get return message from server
	        // Clear out buffer for reuse
	        memset(buffer, '\0', strlen(buffer));
	        // Read data from the socket, leaving \0 at the end
	        charsRead = recv(establishedConnectionFD, buffer, 1000, 0);

	        //charsWritten = atoi(buffer);
		cypherIndex += charsWritten;
		charsLeft -= charsWritten;

		// Check if read was successful
		if (charsRead < 0){
		    // If not, display error
		    error("SERVER: ERROR reading from socket");
		}

		// Display response from the client
		//printf("SERVER: I received this from the client: \"%s\"\n", buffer);

	    }
	    //printf("All data sent to client\n");

            // Close the existing socket which is connected to the client
            close(establishedConnectionFD);
            
	    exit(0);
	}
	waitpid(-1, &childExitMethod, WNOHANG);

	//printf("Child processes complete\n");
    }

    // Close the listening socket
    close(listenSocketFD);

    return 0; 
}
