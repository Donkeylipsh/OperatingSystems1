/*************************************************************************************
 * Author: Colin Van Overschelde
 * Date: 5/10/2018
 * Title: Assignment 2 - Adventure
 * Description: Adventure is map navigation game that demonstrates reading files from
 * 	the most recently modified sub-directory, multi-threaded processing of the
 * 	Adventure map navigation game
 ***********************************************************************************/
 
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

struct Connection{
    struct Connection *prev;	// Points to the previous Connection
    struct Connection *next;	// Points to the next Connection
    int roomIndex;		// Index of room in gameMap
};

struct RoomList{
    struct Connection *front;	// Points to the first Connection
    struct Connection *back;	// Points to the last Connection
};

struct Room{
    char name[100];		// Name of the Room
    char roomType[100];		// Type of the Room
    char connectedRooms[6][100];	// Array of connected Room names
    int connectionCount;	// Count of connected Rooms
};

int isValidDir(const char*);
void getMostRecentFolder(char *);
void readRooms(char *, struct Room[]);
int getStartIndex(struct Room[]);
int getEndIndex(struct Room[]);
void displayRoom(struct Room *);
void *getTime();

int main(){
    // Get the name of the most recently modified directory
    // Create string to hold most recent directory
    char recent[100];
    getMostRecentFolder(recent);

   // printf("Most recent directory: %s\n", recent);

    // Load all rooms from most recent directory into Room list
    struct Room gameMap[7];
    int i;
    for(i = 0; i < 7; i++){
	memset(gameMap[i].name, '\0', 100);
	memset(gameMap[i].roomType, '\0', 100);
	int j;
	for(j = 0; j < 6; j++){
	    memset(gameMap[i].connectedRooms[j], '\0', 100);
	}
    }
    //printf("Rooms created\n");
    readRooms(recent, gameMap);   
    //printf("Rooms read\n");
    // Initialize move list
    struct RoomList *visitedRooms = malloc(sizeof(struct RoomList));
    struct Connection *frontSent = malloc(sizeof(struct Connection));
    struct Connection *backSent = malloc(sizeof(struct Connection));
    visitedRooms->front = frontSent;
    visitedRooms->back = backSent;
    frontSent->next = backSent;
    frontSent->prev = NULL;
    frontSent->roomIndex = -1;
    backSent->prev = frontSent;
    backSent->next = NULL;
    backSent->roomIndex = -1;
  
    // Initialize game variables
    int startIndex = getStartIndex(gameMap);
    int currentRoom = startIndex;
    int endIndex = getEndIndex(gameMap);
    int gameOver = 0; 
    int steps = 0;

    // Start game loop
    while(!gameOver){
	//printf("Current Room: %d\n", currentRoom);
    	// Display currentRoom
    	displayRoom(&gameMap[currentRoom]);
    	// Get input
        char *inputString;
	size_t len = 0;
	ssize_t nread;
	nread = getline(&inputString, &len, stdin);
	//printf("Input read: %s\n", inputString);
	// Remove new line character from inputString
	char goodInput[strlen(inputString)];
	int looper;
	for(looper = 0; looper < strlen(inputString) - 1; looper++){
	    goodInput[looper] = inputString[looper];
	}
    	// Validate input
    	int validInput = 0;
    	int k = 0;
	while( (!validInput) && k < gameMap[currentRoom].connectionCount){
	    //printf("Comparing to: %s\n", gameMap[currentRoom].connectedRooms[k]);
	    if(strcmp(goodInput, gameMap[currentRoom].connectedRooms[k]) == 0 ){
	//	printf("Match found!\n");
		validInput = 1;
		k--;
	    }
	    else if(strcmp(goodInput, "time") == 0 ){
		validInput = -1;
	    }
	    k++;
	}
	if(validInput == 1){
    	    // If yes, udpate currentRoom to input
    	    k = 0;
	    int roomFound = 0;
	    while( (!roomFound) && k < 7 ){
		if(strcmp(goodInput, gameMap[k].name) == 0){
		   roomFound = 1;
		   k--;
		}
		k++;
	    }
	    // Update currentRoom
	    currentRoom = k;
	    // Add Room as new link in visitedRooms
	    struct Connection *newRoom = malloc(sizeof(struct Connection));
	    newRoom->roomIndex = currentRoom;
	    newRoom->next = visitedRooms->back;
	    newRoom->prev = visitedRooms->back->prev;
	    visitedRooms->back->prev->next = newRoom;
	    visitedRooms->back->prev = newRoom;
	    // Increment steps
	    steps++;
	}else if(validInput == -1){
	    int result_code;
	    pthread_t myThreadID;
	    result_code = pthread_create(&myThreadID, NULL, getTime, NULL);
	    assert(0 == result_code);
	    result_code = pthread_join(myThreadID, NULL);
	    printf("Thread complete\n");
	
	}else{
    	    // If not, display error
	    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}
    	// Check if game over
    	if( currentRoom == endIndex){
    	    gameOver = 1;
	}
    }
    // Display victory messages
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    if(steps > 1){
        printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
    }else{
	printf("YOU TOOK %d STEP. YOUR PATH TO VICTORY WAS:\n", steps);
    }
    struct Connection *current = visitedRooms->front;
    // Print each link in vidistedRooms
    while(current->next != visitedRooms->back){
	current = current->next;
	printf("%s\n", gameMap[current->roomIndex].name);
	// Free the previous link from memory
	free(current->prev);
    }
   
    // Free allocated memory
    free(current->next);
    free(current);
    free(visitedRooms);

    return 0;
}

/************************************************************************
 * Descriptions: isValidDir() returns true if name parameter is an invalid directory
 * 	to read from. And returns false if the directory is valid
 * Pre-conditions: n/a
 * Post-conditions: n/a
 * Parameters: name is the name of the directory to test
 ***********************************************************************/
int isValidDir(const char* name){
    // Check if name is an invalid name
    if(name==NULL || name[0] == '\0'){
	// If yes, then the directory can't be read and is invalid
	return 1;
    }else if(name[0] == '.'){
	if(name[1] == '\0'){
	    // If yes, name is the current directory, which is invalid
	    return 1;
	}else if (name[1] == '.' && name[2] == '\0'){
	    // If yes, name is the parent directory, which is invalid
	    return 1;
	}
    }
    // If name is a valid directory, return 0
    return 0;
}

/*********************************************************************
 * Description: getMostRecentFolder() reads each sub-directory from the current
 * 	directory, and returns the name of the most recently modifed sub-directory
 * Pre-conditions: n/a
 * Post-conditions: recent contains the name of the most recently modified
 * 	sub-directory
 * Parameters: recent is the string to save the most recently modified directory name
 *********************************************************************/
void getMostRecentFolder(char *recent){
    // Struct to hold directory data
    struct stat sb;
    // Identifies a file as a directory
    unsigned char isFolder = 0x4;
    // Current directory
    const char *path = ".";
    // String to hold the filename read
    char buffer[100];
    // Open current directory
    DIR *dir=opendir(path); 
    // Struct to hold the current file being read
    struct dirent* entry;
    time_t recenttime = 0;
    // Read each file from teh current directory
    while( NULL != (entry = readdir(dir))){
	// Check if the file is a directory with a valid name
	if( entry->d_type == isFolder && !isValidDir(entry->d_name) ){
	    // If yes, get the file stat
	    // Print full directory name to buffer
	    sprintf(buffer, "%s/%s", path, entry->d_name);
	    // Get directory info
	    stat(buffer, &sb);
	    // Check if the directory is most recently modified
	    if (sb.st_mtime > recenttime){
		// If yes, set recent to directory name
		strncpy(recent, entry->d_name, 100);
		// Update most recently modified time
		recenttime = sb.st_mtime;
	    }
	}
    }
    // Close the directory
    closedir(dir);

    //printf("Most recent directory: %s\n", recent);
}

/**************************************************************************
 * Description: readRooms() finds the most recently modified directory within the 
 * 	current directory, opens that folder and reads 7 files with hard coded
 * 	names and formatted text and parses each file into a Room struct
 * Pre-conditions: Most recently modified directory within current directory contains
 * 	7 Room files
 * Post-conditions: Each Room file is read, parsed into a Room struct and added to
 * 	the someRooms array
 * Parameters: folderName is a string containing the name of the most recently modifed
 * 	directory in the current directory
 **************************************************************************/
void readRooms(char *folderName, struct Room someRooms[]){
   // printf("Starting readRooms()...\n");

    // Create string to hold the name of the current file to be read
    char currentFile[100];
    memset(currentFile, '\0', 100);
    // Create input file
    FILE *inFile;
    // Read each file within folderName directory
    int i;
    for(i = 0; i < 7; i++){
	// Append the hardcoded file name to the directory
        sprintf(currentFile, "./%s/%s%d", folderName, "Room", i);
      //  printf("currentFile = %s\n", currentFile);

	// Initialize room counter
	int roomIndex = 0;
	// Open file to be read
        inFile = fopen(currentFile, "r");
	
	// Initialize variables to read each line from file
	char *currentLine = NULL;
	size_t len = 0;
	ssize_t nread;
	// Read each line from file
	while( (nread = getline(&currentLine, &len, inFile)) != -1){
	   // printf("Line read from file: %s", currentLine);
	    // Read first token
	    char *token = strtok(currentLine, ":\n");
	    //printf("Token 1: %s\n", token);
	    // Check if the token == ROOM NAME
	    if(strcmp(token, "ROOM NAME") == 0){
	        // If yes, get next token
	    	token = strtok(NULL, ":\n");
		//printf("Token length %d\n", strlen(token));
		char roomName[strlen(token)];
		// Copy token to roomName
		int j;
		for(j = 0; j < strlen(token) - 1; j++){
		    roomName[j] = token[j + 1];
		}
		roomName[strlen(token) - 1] = '\0';
		
		// Save roomName to someRooms
		strcpy(someRooms[i].name, roomName);
		//printf("Token 2: %s\n", token);
		//printf("Token 3: %s\n", someRooms[i].name);
	    }else if(strcmp(token, "ROOM TYPE") == 0 ){
		// If yes, get next token
		token = strtok(NULL, ":\n");
		//printf("ROOM TYPE: %s\n", token);
		// Copy token to roomType
		char roomType[strlen(token)];
		int j;
		for(j = 0; j < strlen(token) - 1; j++){
		    roomType[j] = token[j + 1];
		}
		roomType[strlen(token) - 1] = '\0';
		// Copy roomType to someRooms[i].roomType
		strcpy(someRooms[i].roomType, roomType);
	    }else{
		// Get next token
		token = strtok(NULL, ":\n");
		// Copy token to nextRoom
		char nextRoom[strlen(token)];
		int j;
		for( j = 0; j < strlen(token) - 1; j++){
		    nextRoom[j] = token[j + 1];
		}
		nextRoom[strlen(token) - 1] = '\0';
		// Copy nextRoom to someRooms[i].connectedRooms
		strcpy(someRooms[i].connectedRooms[roomIndex], nextRoom);
		roomIndex++;
	    }
	}
	// Update connectionCount
	someRooms[i].connectionCount = roomIndex;
	// Close input file
	fclose(inFile);
    }

}

/****************************************************************************
 * Description: getStartIndex() accepts an array of Room structs and returns the
 * 	index of the room that has roomType = "START_ROOM"
 * Pre-conditions: someRooms is a valid game map
 * Post-Condistions: Index of starting room is returned
 * Parameters: someRooms is an array of 7 Room structs
 ***************************************************************************/
int getStartIndex(struct Room someRooms[]){
    // Find the index of the room with roomType = "START_ROOM"
    int startIndex = -1;
    int currentIndex = 0;
    while( startIndex < 0 && currentIndex < 7 ){
	// Check if the roomType = "START_ROOM"
	if( strcmp(someRooms[currentIndex].roomType, "START_ROOM") == 0 ){
	    // Assign startIndex value
	    startIndex = currentIndex;
	}
	currentIndex++;
    }

    //printf("Start Index = %d\n", startIndex);
    return startIndex;
}


/****************************************************************************
 * Description: getEndIndex() accepts an array of Room structs and returns the
 * 	index of the room that has roomType = "END_ROOM"
 * Pre-conditions: someRooms is a valid game map
 * Post-Condistions: Index of ending room is returned
 * Parameters: someRooms is an array of 7 Room structs
 ***************************************************************************/
int getEndIndex(struct Room someRooms[]){
    // Find the index of the room with roomType = "END_ROOM"
    int endIndex = -1;
    int currentIndex = 0;
    while( endIndex < 0 && currentIndex < 7 ){
	// Check if the roomType = "END_ROOM"
	if( strcmp(someRooms[currentIndex].roomType, "END_ROOM") == 0 ){
	    // Assign endIndex value
	    endIndex = currentIndex;
	}
	currentIndex++;
    }

    //printf("End Index = %d\n", endIndex);
    return endIndex;
}

/***************************************************************************
 * Description: displayRoom() prints a Room struct
 * Pre-Conditions: n/a
 * Post-Conditions: someRoom has been printed to standard output
 * Parameters: someRoom is the Room struct to be printed
 ***************************************************************************/
void displayRoom(struct Room *someRoom){
    // Print the contents of the room
    printf("CURRENT LOCATION: %s\n", someRoom->name);
    printf("POSSIBLE CONNECTIONS: ");
    int i;
    for(i = 0; i < someRoom->connectionCount; i++){
	printf("%s", someRoom->connectedRooms[i]);
	if( i < ( someRoom->connectionCount - 1 ) ){
	    printf(", ");
	}
	else{
	    printf(".\n");
	}
    }
    printf("WHERE TO? >");
}

/**********************************************************************
 * Description:
 * Pre-Conditions:
 * Post-Conditions:
 * Description:
 **********************************************************************/
void* getTime(){
    char timeString[200];
    time_t t;
    struct tm *tmp;
    size_t result;

    t = time(NULL);
    tmp = localtime(&t);
    result = strftime(timeString, sizeof(timeString), "%I:%M%P, %A, %B %d, %Y", tmp);
    printf("\n%s\n\n", timeString);

    return NULL;
}
