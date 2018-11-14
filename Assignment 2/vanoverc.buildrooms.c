/*********************************************************************************
 * Author: Colin Van Overschelde
 * Date: 5/10/2018
 * Title: Assignment 2 Build Rooms
 * Description: Build Rooms demonstrates randomly generating the contents of Room
 * 	files and writing them to a new folder with a name contianing the unique
 * 	process id of the Build Rooms program that generated the files.
 *********************************************************************************/

#include <stdio.h>
#include <unistd.h>	// Used for getpid()
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>

//Define Connection struct
struct Connection{
    struct Connection *prev;
    struct Connection *next;
    struct Room *someRoom;
};

// Define RoomList struct
struct RoomList{
    struct Connection *front;
    struct Connection *back;
};

// Define Room struct
struct Room{
    char name[100];
    struct RoomList *connectedRooms;
    char roomType[100];
    struct Room *next;
    struct Room *prev;
};

struct GameMap{
    struct Room *front;
    struct Room *back;
};

// Define functions
void CreateRoom(struct Room *someRoom, char *someName);
void CreateRoomArray(struct GameMap *gameMap, char allRooms[][9]);
void PopulateAllRooms(char allRooms[][9]);
void CreateGraph();
int IsGraphFull(struct GameMap *gameMap);
void AddRandomConnection();
struct Room* GetRandomRoom(struct GameMap *myMap);
int CanAddConnectionFrom(struct Room *someRoom);
int ConnectionAlreadyExists(struct Room *leftRoom, struct Room *rightRoom);
void ConnectRoom(struct Room *leftRoom, struct Room *rightRoom);
int IsSameRoom(struct Room *leftRoom, struct Room *rightRoom);
void WriteRoomFiles(struct GameMap *myMap);


int main(){

    // Create array of all rooms
    char allRooms[10][9];
    PopulateAllRooms(allRooms);
/*
    int i;
    for(i = 0; i < 10; i++){
	printf("Room[%d]: %s\n", i, allRooms[i]);
    }
*/
    // Select Rooms for Map
    struct GameMap *myMap = malloc(sizeof(struct GameMap));
    struct Room *frontMap = malloc(sizeof(struct Room));
    struct Room *backMap = malloc(sizeof(struct Room));
    frontMap->prev = NULL;
    frontMap->next = backMap;
    backMap->prev = frontMap;
    backMap->next = NULL;
    myMap->front = frontMap;
    myMap->back = backMap;

//    printf("Creating RoomArray\n");
    CreateRoomArray(myMap, allRooms);

    // Test CreateGraph
    CreateGraph(myMap);
    WriteRoomFiles(myMap);

//    printf("Freeing memory\n");

    // Free Rooms from myMap
    struct Room *curRoom = frontMap;
    while(curRoom->next != backMap){
	// Move to the next link
	curRoom = curRoom->next;
	// Free the previous link
//	printf("Freeing previous link of curRoom\n");
	free(curRoom->prev);
//	printf("Creating handle\n");
	// Get a handle to the front sentinel of curRoom->connectedRooms
	struct Connection *curConnect = curRoom->connectedRooms->front;
	// Free each link in curConnect
	while(curConnect->next != curRoom->connectedRooms->back){
	    // Move to the nextLink
	    curConnect = curConnect->next;
	    // Free the previous link
//	    printf("Freeing previous link of curConnect\n");
	    free(curConnect->prev);
	}
	// Free the back sentinel of curConnect
//	printf("Freeing back sentinel of curConnect\n");
	free(curConnect->next);
	// Free the last link of curConnect
//	printf("Freeing last link of curConnect\n");
	free(curConnect);
	// Free the Room in curRoom
//	printf("Freeing the Room in curRoom\n");
	free(curRoom->connectedRooms);
    }
    // Free the back sentinel of curRoom
  //  printf("Freeing back sentinel of curRoom\n");
    free(curRoom->next);
    // Free the last link of curRoom
//    printf("Freeing the last link of curRoom\n");
    free(curRoom);
    // Free myMap
//    printf("Freeing myMap\n");
    free(myMap);

    return 0;
}

/****************************************************************************
 * Description: CreateRoom() initializes the values of a new Room struct
 * Pre-Conditions: n/a
 * Post-Conditions: n/a
 * Parameters: someRoom is the new Room struct to be populated with data
 * 	someName is the name of the Room
 ****************************************************************************/
void CreateRoom(struct Room *someRoom, char *someName){
    //printf("CreateRoom() started...\n");
    assert(someRoom != NULL);

    // Allocate memory for Room contents
    struct RoomList *connectedRooms = malloc(sizeof(struct RoomList));
    struct Connection *frontSentinel = malloc(sizeof(struct Connection));
    struct Connection *backSentinel = malloc(sizeof(struct Connection));

    // Initialize RoomList Sentinels
    frontSentinel->next = backSentinel;
    frontSentinel->prev = NULL;
    backSentinel->prev = frontSentinel;
    backSentinel->next = NULL;
    connectedRooms->front = frontSentinel;
    connectedRooms->back = backSentinel;

    // Initialize Room values
    strcpy(someRoom->name, someName);
    someRoom->connectedRooms = connectedRooms;

    //printf("Create room complete...\n");
}

/***************************************************************************
 * Description: CreateRoomArray randomly selects the Room Names to include
 * 	in the Game Map
 * Pre-Conditions: allRooms contains the names of each Room
 * Post-Conditions: gameMap is populated with 7 randomly selected Rooms
 * Parameters: gameMap is teh GameMap to populate
 * 	allRooms is an array of strings containing names for rooms
 ***************************************************************************/
void CreateRoomArray(struct GameMap *gameMap, char allRooms[][9]){
    //printf("CreateRoomArray() started...\n");
    assert(gameMap != NULL);
    // Seed randomizer
    srand(time(NULL));

    // Initialize selected rooms array
    int selectedRooms[7];
    int i;
    for(i = 0; i < 7; i++){
	selectedRooms[i] = -1;
    }

    // Boolean flags for room type assignment
    int startFilled = 0;
    int endFilled = 0;
    int midCount = 0;

    //printf("* Selecting rooms...\n");
    // Select rooms
    for(i = 0; i < 7; i++){
	int newRoom = 0;
	int selection = 0;
	while(!newRoom){
	    // Set newRoom = 1;
	    newRoom = 1;
            // Generate random index of selected Room
	    selection = rand() % 10;
	    //printf("* Random selection: %d\n", selection);
            // Check if index has been selected
            int j;
	    for(j = 0; j < i; j++){
		// Check if the value of the current index matches the random selection
	        if( selectedRooms[j] == selection ){
		    // If yes, set newRoom to false
		    newRoom = 0;
	        }
	    }
	}
	// Add index to array of selected indicies
	selectedRooms[i] = selection;
	//printf("* Index selected: %d\n", selection);
	
	// Allocate memory for new Room
	struct Room *someRoom = malloc(sizeof(struct Room));
	// Initialize new Room
	CreateRoom(someRoom, allRooms[selection]);

	// Add roomType
	// Prepare string to hold selected type
	char roomType[20];
        // Initialize string to null terminators
        memset(roomType, '\0', 20);
	int rtRoll;
	//printf("startFilled = %d\nendFilled = %d\nmidCount= %d\n", startFilled, endFilled, midCount);
	// Select room type
	if(!startFilled){
	    if(!endFilled){
		if(midCount < 5){
	//	    printf("Choosing type: All 3\n");
	    	    // Roll random value for all three types
	    	    rtRoll = rand() % 3;
	    	    // Assign roomType based on value of rtRoll	
	    	    if( rtRoll == 0 ){ 
	//		printf("* Selected START for %s\n", someRoom->name);
	        	// Set roomType = START_ROOM
	        	strcpy(someRoom->roomType, "START_ROOM");
	        	//strcpy(someRoom->roomType, roomType);
	        	//someRoom->roomType = roomType;
	        	startFilled = 1;
			//printf("* Type assigned\n");
	    	    }else if( rtRoll == 1 ){
	//		printf("* Selected MID for %s\n", someRoom->name);
			// Set roomType = MID_ROOM
			strcpy(someRoom->roomType, "MID_ROOM");
			//strcpy(someRoom->roomType, roomType) ;
			midCount++;
	//		printf("* Type assigned to someRoom\n");
	    	    }else{
	//		printf("Selected END for %s\n", someRoom->name);
			// Set roomType = END_ROOM
			strcpy(someRoom->roomType, "END_ROOM");
			//strcpy(someRoom->roomType, roomType);
			endFilled = 1;
	//		printf("* Type assigned\n");
	    	    }
		}else{
	//	    printf("Choosing type: START, END\n");
	    	    // Roll random value for START and END types
	    	    rtRoll = rand() % 2;
	    	    // Assign roomType based on value of rtRoll
	    	    if( rtRoll == 0 ){
	//		printf("Selected START for %s\n", someRoom->name);
	        	// Set roomType = START_ROOM
			strcpy(someRoom->roomType, "START_ROOM");
			//strcpy(someRoom->roomType, roomType);
			startFilled = 1;
	    	    }else{
	//		printf("Selected END for %s\n", someRoom->name);
			// Set roomType = END_ROOM
			strcpy(someRoom->roomType, "END_ROOM");
			endFilled = 1;
	    	    }
		}
	    }else if (midCount < 5){
	//	printf("Choosing type: START, MID\n");
	        // Roll random value for START and MID types
	        rtRoll = rand() % 2;
	        // Assign roomType based on value of rtRoll
	        if( rtRoll == 0 ){
	//	    printf("Selected START for %s\n", someRoom->name);
		    // Set roomType = START_ROOM
		    strcpy(someRoom->roomType, "START_ROOM");
		    startFilled = 1;
	        }else{
	//		printf("Selected MID for %s\n", someRoom->name);
		    // Set roomType = MID_ROOM
		    strcpy(someRoom->roomType, "MID_ROOM");
		    midCount++;
		}
	    }else{
	//	printf("Choosing type: START for %s\n", someRoom->name);
	        // Set roomType = START_ROOM
	        strcpy(someRoom->roomType, "START_ROOM");
	        startFilled = 1;
	    }
	}else if (!endFilled){
	    if(midCount < 5){
	//	printf("Choosing type: MID, END\n");
	        // Roll random value for MID and END types
	        rtRoll = rand() % 2;
	        // Assign roomType based on value of rtRoll
	        if( rtRoll == 0 ){
	//	    printf("* MID selected for %s\n", someRoom->name);
	            // Set roomType = MID_ROOM
	            strcpy(someRoom->roomType, "MID_ROOM");
	            midCount++;
	        }else{
	//	    printf("* END selected for %s\n", someRoom->name);
	            // Set roomType = END_ROOM
	            strcpy(someRoom->roomType, "END_ROOM");
	            endFilled = 1;
	        }
	    }else{
	//	printf("Choosing type: END for %s\n", someRoom->name);
	        // Set roomType to END_ROOM
	        strcpy(someRoom->roomType, "END_ROOM");
	        endFilled = 1;
	    }
	}else{
	  //  printf("Choosing type: MID for %s\n", someRoom->name);
	    // Set roomType = MID_ROOM
	    strcpy(someRoom->roomType, "MID_ROOM");
	    midCount++;
	}
	//printf("assigning gameMap[%d]\n", i);
	// Add new Room to gameMap
	someRoom->next = gameMap->back;
	someRoom->prev = gameMap->back->prev;
	gameMap->back->prev->next = someRoom;
	gameMap->back->prev = someRoom;
    }
	//printf("CreateRoomArray complete...\n");
}

/*******************************************************************************
 * Descriptions: PopulateAllRooms assigns string values to an array of strings
 * Pre-Conditions: n/a
 * Post-Conditions: allRooms is populated with the names of the 10 rooms
 * Parameters: allRooms is the array to store the Room names
 *******************************************************************************/
void PopulateAllRooms(char allRooms[][9]){
    //printf("PopulateAllRooms() started...\n");

    // Populate names of each room
    // Create string to hold each room name
    char someRoom[9];

    // Initialize string to null terminators
    memset(someRoom, '\0', 9);
    // Copy hardcoded name to someRoom
    strcpy(someRoom, "Market");
    // Copy someRoom to the next index in allRooms
    strcpy(allRooms[0], someRoom);

    // Repeat for each room
    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Armor");
    strcpy(allRooms[1], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Weapons");
    strcpy(allRooms[2], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Magic");
    strcpy(allRooms[3], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Inn");
    strcpy(allRooms[4], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Castle");
    strcpy(allRooms[5], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Dungeon");
    strcpy(allRooms[6], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Gates");
    strcpy(allRooms[7], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Healer");
    strcpy(allRooms[8], someRoom);

    memset(someRoom, '\0', 9);
    strcpy(someRoom, "Trainer");
    strcpy(allRooms[9], someRoom);
}

/**************************************************************************
 * Description: CreateGraph() creates all connections for the Room in teh Game Map
 * Pre-Conditions: n/a
 * Post-Conditions: The Rooms in gameMap are all populated with valid connections
 * Parameters: gameMap is the linked list of Rooms to connected
 *************************************************************************/
void CreateGraph(struct GameMap *gameMap){
    //printf("CreateGraph() started...\n");
    assert(gameMap != NULL);
    // Add Connections to rooms until the graph is full
    while( IsGraphFull(gameMap) == 1){
	// Add a random Connection to the graph
	AddRandomConnection(gameMap);
    }

}

/**************************************************************************
 * Description: IsGraphFull() Returns true if all rooms have 3 to 6 outbound 
 * 	connections, false otherwise
 * Pre-Conditions: n/a
 * Post-Conditions: Boolean result is returned
 * Parameters: gameMap is the GameMap to test
 *************************************************************************/
int IsGraphFull(struct GameMap *gameMap){
    //printf("IsGraphFull() started...\n");
    assert(gameMap != NULL);
    // Initialize Loop Control Variables
    int hasConnections = 0;
    int currentIndex = 0;
    struct Room *current = gameMap->front;   
 
    // Loop through each room in gameMap
    while(!hasConnections && current->next != gameMap->back){
	// Check if current Room has more than 3 and less than 6 connections
	int connectionCount = 0;
	current = current->next;
	struct Connection *currentConnection = current->connectedRooms->front;

	// Add up all the connected rooms
	while(currentConnection->next != current->connectedRooms->back){
	    connectionCount++;
	    currentConnection = currentConnection->next;
	}
	// Check if connectionCount is valid
	if(connectionCount < 3 || connectionCount > 6){
	    hasConnections = 1;
	}
	// Increment currentIndex to check next room
	currentIndex++;
    }
    // Return True if all Room have between 3 and 6 connections, False otherwise
    return hasConnections;
}

/**************************************************************************
 * Description: AddRandomConnection adds a random, valid outbound connection from 
 * 	a Room to another Room
 * Pre-Conditions: n/a
 * Post-Conditions: A new random Connection has been added to two Room in myMap
 * Parameters: myMap is the linked list of Rooms to select from to create a
 * 	connection
 *************************************************************************/
void AddRandomConnection(struct GameMap *myMap){
    //printf("AddRandomConnection() started...\n");
    assert(myMap != NULL);
    // Define Room's to connect
    struct Room *leftRoom;
    struct Room *rightRoom;

    // Get a valid leftRoom
    while(1){
	//printf("Selecting leftRoom\n");
	// Get a random Room
	leftRoom = GetRandomRoom(myMap);

	// Check if the Room can have more connections
	if (CanAddConnectionFrom(leftRoom)){
	    break;
	}
    }

    // Get a valid rightRoom
    do{
	//printf("Selecting rightRoom\n");
	// Get a random Room
	rightRoom = GetRandomRoom(myMap);

	// Repeat until valid Room found
    }while(CanAddConnectionFrom(rightRoom) == 0 || IsSameRoom(rightRoom, leftRoom) == 1 ||
	   ConnectionAlreadyExists(leftRoom, rightRoom) == 1 );
    
    // Connect leftRoom and rightRoom
    ConnectRoom(leftRoom, rightRoom);
    ConnectRoom(rightRoom, leftRoom);
}

/**************************************************************************
 * Description: Returns a random Room from myMap, does NOT validate if connection 
 * 	can be added
 * Pre-Conditions: n/a
 * Post-Conditions: A random Room is returned
 * Parameters: myMap is the linked list of Rooms to select from
 *************************************************************************/
struct Room* GetRandomRoom(struct GameMap *myMap){
    //printf("GetRandomRoom() started...\n");
    assert(myMap != NULL);

    // Roll a random index
    int index = rand() % 7;

    //printf("Room selected: %d\n", index);
    // Find the Room at the selected index
    int roomIndex = 0;
    struct Room *current = myMap->front;
    while(current->next != myMap->back && roomIndex <= index){
	current = current->next;
	roomIndex++;
    }
    // Return the selected Room
    return current;
}

/**************************************************************************
 * Description: CanAddConnectionFrom() returns true if a connection can be added 
 * 	from Room someRoom, false otherwise
 * Pre-Conditions: n/a
 * Post-Conditions: Result of test is returned
 * Parameters: someRoom is the Room to test
 *************************************************************************/
int CanAddConnectionFrom(struct Room *someRoom){
    //printf("CanAddConnectionFrom() started...\n");
    assert(someRoom != NULL);
    // If connections < 6, return true
    int connectionCount = 0;
    struct Connection *current = someRoom->connectedRooms->front;

    // Add up all connected Rooms
    while(current->next != someRoom->connectedRooms->back){
	connectionCount++;
	current = current->next;
    }

    if( connectionCount > 5 ){
	//printf("CanAddConnectionFrom() = 0\n");
        return 0;
    }
    else{
	//printf("CanAddConnectionFrom() = 1\n");
	return 1;
    }
}

/**************************************************************************
 * Description: ConnectionAlreadyExists() returns true is a connection from leftRoom
 * 	to rightRoom already exists, false otherwise
 * Pre-Conditions: n/a
 * Post-Conditions: Result of test is returned
 * Parameters: leftRoom is the source Room to test
 * 	rightRoom is the destination Room to test
 *************************************************************************/
int ConnectionAlreadyExists(struct Room *leftRoom, struct Room *rightRoom){
    //printf("ConnectionAlreadyExists() started...\n");
    assert(leftRoom != NULL);
    assert(rightRoom != NULL);
    struct Connection *current = leftRoom->connectedRooms->front;
    int found = 0;

    // Loop through each Connection until the Connection is found or end of list reached
    while( !found && (current->next != leftRoom->connectedRooms->back) ){
	//printf("* Testing next link\n");
	// Move to next link
	current = current->next;
	// Check if the link contains rightRoom
	if( rightRoom == current->someRoom ){
	    found = 1;
	}
    }

    //printf("ConnectionAlreadyExists() = %d\n", found);
    return found;
}

/**************************************************************************
 * Description: ConnectRoom() connects Rooms leftRoom and rightRoom, doesn't check 
 * 	if connection is valid
 * Pre-Conditions: n/a
 * Post-Conditions: A new connection from leftRoom to rightRoom is created
 * Parameters: leftRoom is the source Room to connect
 * 	rightRoom is the destination Room to connect
 *************************************************************************/
void ConnectRoom(struct Room *leftRoom, struct Room *rightRoom){
    //printf("ConnectRoom() started...\n");
    assert(leftRoom != NULL);
    assert(rightRoom != NULL);

    // Initialize new Connection
    struct Connection *newRoom = malloc(sizeof(struct Connection));
    newRoom->next = leftRoom->connectedRooms->back;
    newRoom->prev = leftRoom->connectedRooms->back->prev;
    newRoom->someRoom = rightRoom;

    // Insert new Connection at the end of leftRoom->connectedRoom
    leftRoom->connectedRooms->back->prev->next = newRoom;
    leftRoom->connectedRooms->back->prev = newRoom;

}

/**************************************************************************
 * Description: IsSameRoom() returns true if leftRoom and rightRoom are the same 
 * 	room, false otherwise
 * Pre-Conditions: n/a
 * Post-Conditions: The result of the test is returned
 * Parameters: leftRoom and rightRoom are the Rooms to be tested
 *************************************************************************/
int IsSameRoom(struct Room *leftRoom, struct Room *rightRoom){
    //printf("IsSameRoom() started...\n");
    assert(leftRoom != NULL);
    assert(rightRoom != NULL);
    if(leftRoom == rightRoom){
	//printf("IsSameRoom = 1\n");
        return 1;
    }
    else{
	//printf("IsSameRoom = 0\n");
	return 0;
    }
}

/**************************************************************************
 * Description: WriteRoomFiles writes each room to a file in a directory uniquely
 * 	named with the process id of the program
 * Pre-Conditions: n/a
 * Post-Conditions: All Rooms have been saved to individual files
 * Parameters: gameMap is the linked list containing the Rooms to be saved
 *************************************************************************/
void WriteRoomFiles(struct GameMap *gameMap){
    //printf("WriteRoomFiles() started...\n");
    assert(gameMap != NULL);
    int pid = getpid();

    //printf("Proccess ID: %d\n", pid);

    char fileName[100];
    char pidString[100];
    memset(fileName, '\0', 100);
    memset(pidString, '\0', 100);
    strcpy(fileName,"./vanoverc.rooms.");
    //printf("fileName=%s\n", fileName);    
    sprintf(pidString, "%d", pid);
    strcat(fileName, pidString);
    //printf("Directory: %s\n", fileName);

    mkdir(fileName, 0755);

    strcat(fileName, "/Room");

    //printf("With Room: %s\n", fileName);

    int endIndex = strlen(fileName);
    struct Room *someRoom = gameMap->front;
    FILE * oFile;
    int i = 0;
    // Traverse each link the gameMap
    while( someRoom->next != gameMap->back ){
	// Move to next link
	someRoom = someRoom->next;
	// Get character value of file index
	fileName[endIndex] = (char)(i + 48);
	// Print the current room to file
	oFile = fopen(fileName, "w");	
	fprintf(oFile, "ROOM NAME: %s\n", someRoom->name);
	// Print each Connection
	int j = 1;
	struct Connection *current = someRoom->connectedRooms->front;
	while( current->next != someRoom->connectedRooms->back ){
	    current = current->next;
	    fprintf(oFile, "CONNECTION %d: %s\n", j, current->someRoom->name);
	    j++; 
	}
	fprintf(oFile, "ROOM TYPE: %s\n", someRoom->roomType);
	fclose(oFile);
	i++;
    }
}
