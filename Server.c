#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define MAXDATASIZE 100
#define ARRAY_SIZE 30  /* Size of array to receive */
#define BACKLOG 10     /* how many pending connections queue will hold */
#define RETURNED_ERROR -1
#define DEFAULT_PORT_NO 12345
#define MAX_USERS 10
#define MAX_THREADS 10

//define structs

typedef struct User {
   char *username;
   char *password;
   int games_played;
   int games_won;
} User;

typedef struct Word_pair{
   char *type;
   char *object;
} Word_pair;

/* Definitions */
int sock_fd;  /* listen on sock_fd, new connection on new_fd */
struct sockaddr_in my_addr;    /* my address information */
struct sockaddr_in their_addr; /* connector's address information */
socklen_t sin_size;

User userlist[MAX_USERS];
int userCount = 0;
int connectedUsers = 0;

User sortedUsers[MAX_USERS];

Word_pair words[500];
int numWords = 0;

char* authFilename = "Authentication.txt";
char* wordsFilename = "hangman_text.txt";

int threadID[MAX_THREADS];
pthread_t threads[MAX_THREADS];
	
//Define Functions
void debug_printuserlist(); // e.g. debug_printuserlist(&userlist, userCount);
void getMessage(int sock_fd, char buffer[]); // e.g. (sock_fd, buffer);
void sendMessage(int sock_fd, char * message);
int userCompare(); // 1 is up, -1 is down, 0 is same
void sortUsers();
void* gamePlay(void *some_fd);
void importWords(char * filename, Word_pair * output, int * wordCount);
void importUsers(char * filename, User * output, int * userCount);


//Begin Program
int main(int argc, char *argv[]){
	



	
//-------------------------------------------------------------------	
// ***Import word and user arrays

	 importWords(wordsFilename, words , &numWords);
	 importUsers(authFilename, userlist, &userCount);
	
//------------------------------------------------------------------
// generate socket & listen

	/* generate the socket */
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* generate the end point */
	my_addr.sin_family = AF_INET;         /* host byte order */
	my_addr.sin_port = htons(DEFAULT_PORT_NO); //set port to default
	if (argc > 2){my_addr.sin_port = htons(atoi(argv[1]));}// set port to user specified if available
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
	bzero(&(my_addr.sin_zero), 8);       /* zero the rest of the struct */

	/* bind the socket to the end point */
	if (bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	


	/* start listnening */
	if (listen(sock_fd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	printf("server starts listnening ...\n");
	



//-----------------------------------------------------------------------------
// toplevel client progress loop

	while (1) {  
		
			int new_fd;
			sin_size = sizeof(struct sockaddr_in);
			if ((new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
				perror("accept");
				//continue;
			}
			
			//-------------------------------------------------------------------		
			// Thread creation
			/* create the request-handling threads */
			//for (int i=0; i<MAX_THREADS; i++) {
				threadID[connectedUsers];
				
				
				if (pthread_create(&threads[connectedUsers], NULL, gamePlay, (void *)new_fd)){
					perror("couldn't create thread");
					return 1;
				} 
				//connectedUsers++;
				printf("Connected %d", connectedUsers);
				connectedUsers++;
				
			//}
			
			printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));

			//gamePlay(new_fd, &userlist, userCount, &words, numWords);
		 
	}
}


void debug_printuserlist(User *userlist, int userCount){
	int currentUser = 0;
	while(currentUser < userCount){
		if (userlist[currentUser].username != NULL){
			printf("[%s]  [%s]	[%d][%d] \n", userlist[currentUser].username, userlist[currentUser].password,
				userlist[currentUser].games_won, userlist[currentUser].games_played);
		}
		currentUser++;
	}
}

void getMessage(int sock_fd, char buffer[]){
	int numbytes;
	if ((numbytes=recv(sock_fd, buffer, MAXDATASIZE, 0)) == -1) {
		perror("recv");
		close(sock_fd);
		exit(1);
	} 
	buffer[numbytes] = '\0';
}

void sendMessage(int sock_fd, char * message){
	if (send(sock_fd,message, strlen(message),0)== -1) {
		fprintf(stderr, "Failure Sending Message\n");
		close(sock_fd);
		exit(1);
	}
}

int userCompare(User user1, User user2){
	//one is up, -1 is down, 0 is same
	// less games won is up
	// less win percent is up
	// earlier alphabetical is up
	
	//games won
	int gamesWonDiff = user1.games_won - user2.games_won;
	if (gamesWonDiff < 0){
		return 1;
	} else if (gamesWonDiff > 0){
		return -1;
	} else{
		// percentage of games won
		double winPercentDiff = ((double)user1.games_won/(double)user1.games_played) - 
			((double)user2.games_won/(double)user2.games_played);
		if (winPercentDiff < 0){
			return 1;
		} else if (winPercentDiff > 0){
			return -1;
		} else{
			// alphabetical order
				//copy user string
				//set to lowercase
				
			char * tempString1 = malloc(20* sizeof(char));
			char * tempString2 = malloc(20* sizeof(char));
			strcpy(	tempString1, user1.username);
			strcpy( tempString2, user2.username);
			
			for(int i = 0; tempString1[i]; i++){
				tempString1[i] = tolower(tempString1[i]);
			}
			for(int i = 0; tempString2[i]; i++){
			  tempString2[i] = tolower(tempString2[i]);
			}
					
			int usercompare = strcmp(tempString1,tempString2);
			if (usercompare > 0){
				return -1;
			} else if (usercompare < 0){
				return 1;
			} else{
				return 0;
			}
		}
	}
}

void sortUsers(User *userlist, User *SortedUserList, int numUsers){
	int sortableUsers = 0;
	int hasSwapped;
	User tempUser;
	
	//clear old sorted list
	memset(SortedUserList, '\0', sizeof(SortedUserList));
	
	//copy sortable(games_played > 0) users from userlist into sortableUsers
	for(int i=0; i < numUsers;i++){
		if (userlist[i].games_played > 0){
			memcpy(&SortedUserList[sortableUsers], &userlist[i], sizeof(userlist[i]));
			sortableUsers += 1;
		}
	}
	
	//sort users into ssortedUsers array
	int tempInt = 0;
	
	if(sortableUsers > 1){ //enough users to sort
		do{
		hasSwapped = 0;
			for (int i = 1; i < numUsers; i++){ // compare whole array
				tempInt = userCompare(SortedUserList[i-1],SortedUserList[i]);
				if (tempInt < 1){
					//swap here
					tempUser = SortedUserList[i-1];
					SortedUserList[i-1] = SortedUserList[i];
					SortedUserList[i] = tempUser;
					hasSwapped = 1;					
				}
			}
		} while (hasSwapped == 1); // continue until no swaps aka sorted
	}	
}

void* gamePlay(void *some_fd){
	char buffer[1024] = "0";
	User sortedUsers[MAX_USERS];
	int currentUser = 0;
	int new_fd;
	new_fd = (int)some_fd;
	
	//userlist = userlist;
	//words = words;

	char *username;
	char *password;
	
	username = malloc(15 * sizeof(char));
	password = malloc(15 * sizeof(char));
	
	printf("Connected to thread %lu\n", pthread_self());
	//recv username
	getMessage(new_fd, username);
	printf("%s\n", username);
	
	getMessage(new_fd, password);
	printf("%s\n", password);
	
	//check if user registered
	int authFailed = 1;
	for (int i = 0; i < userCount; i++ ){		
		if (strcmp(username, userlist[i].username) == 0 && strcmp(password, userlist[i].password) == 0){
			//printf("user found & pass correct \n");
			authFailed = 0;
			currentUser = i;
			break;
		}
	}
	free(username);
	free(password);
	
	//send response to client
	char * message = malloc(20);
	if (authFailed == 1){
		//printf("failed auth, quitting \n");
		message = "authFail";
		sendMessage(new_fd, message);
		close(new_fd);
		exit(0);
	} else {
		message = "authPass";
		sendMessage(new_fd, message);
	}
	//free(message);



//***hangman title
	srand(time(NULL));
	while(1) {
		getMessage(new_fd, buffer);
		printf("User Selected: %s\n", buffer);
		
		if (strcmp(buffer, "1") == 0) {
			char word[50] = "";
			char temp[50] = "";
			int guesses_left;
			int word_size;
			int won_game = 0;
			int r = rand() % numWords;
			
			strcat(word, words[r].type);
			strcat(word, " ");
			strcat(word, words[r].object);
			word_size = strlen(word);
			
			if ((word_size - 1 + 10) < 26) {
				guesses_left = (word_size -1 + 10);
			} else {
				guesses_left = 26;
			}
			
			printf("Game Start\n");
			printf("%s\n", word);
			
			for (int i = 0; i < word_size; i++) {
				if (word[i] == *" ") {
					temp[i] = word[i];
				} else {
					temp[i] = *"_";
				}
			}
			
			strcpy(buffer, temp);
			sendMessage(new_fd, buffer);
			
			while (guesses_left > 0) {
				getMessage(new_fd, buffer);
				printf("User Guessed: %s\n", buffer);
				guesses_left--;
				
				for (int i = 0; i < sizeof(word)/sizeof(char); i++){
					if (*buffer == word[i]){
					temp[i]= word[i];
					}
				}
				
				strcpy(buffer, temp);
				sendMessage(new_fd, buffer);
				
				int letters_left = 0;
				
				for (int i = 0; i < word_size; i++) {
					if (temp[i] == *"_") {
						letters_left++;
					}
				}
				
				if (letters_left == 0) {
					won_game = 1;
					break;
				}
			}
			if (won_game == 1) {
				printf("Player Won\n");
				userlist[currentUser].games_played++;
				userlist[currentUser].games_won ++;
			} else {
				printf("Player Lost\n");
				userlist[currentUser].games_played++;
			}
		} else if (strcmp(buffer, "2")== 0) {
			int players = 0;
			
			memset(&buffer[0], 0, sizeof(buffer));
			sortUsers(userlist, sortedUsers, userCount);
			printf("Show Leaderboard\n");
			
			for (int i = 0; i < userCount; i++){
				if (userlist[i].games_played > 0) {
					players++;
				}
			}
			
			sprintf(buffer, "%d", players);
			//printf("Players %s\n", buffer);
			sendMessage(new_fd, buffer);
			usleep(500);
			
			if (players > 0) {
				for (int i = 0; i < players; i++) {
					sprintf(buffer, "%s %d %d", sortedUsers[i].username, sortedUsers[i].games_won, sortedUsers[i].games_played);
					sendMessage(new_fd, buffer);
					//printf("%s\n", buffer);
					do{
						getMessage(new_fd, buffer);
						usleep(500);
					}while(strcmp(buffer,"received") != 0);
				}
			}
		} else if (strcmp(buffer, "3")== 0){
			
			// modification to users (test sorting)
			userlist[3].games_played += 1;
			userlist[3].games_won += 1;
			userlist[5].games_won += 5;
			userlist[1].games_played += 1;
			for(int i=0; i < userCount; i++){
				userlist[i].games_played += 1;
			}
			//end test user modification section
			//sort user function
			sortUsers( userlist, sortedUsers, userCount);
			//print both user arrays with line between
			debug_printuserlist(userlist, userCount);
			printf("-----------------------------------------\n");
			debug_printuserlist(sortedUsers, userCount);
			
			
			printf("User Quit\n");
			//close(new_fd);
			break;
			//exit(0);
		}				
	}
	//close(new_fd); // connection close
	//exit(0);
}

void importWords(char * filename, Word_pair * output, int * wordCount){
	char buffer[1024] = "0";
	FILE *file;
	
	file = fopen(filename, "r");
	if (file == NULL){
        printf("Could not open file %s",filename);
    }
	int wordPairCounter = 0;
	while (fgets(buffer, sizeof(buffer), file) != NULL){
		output[wordPairCounter].type = malloc(20*sizeof(char));
		output[wordPairCounter].object = malloc(20*sizeof(char));
		sscanf( buffer, "%[^,],%s ", output[wordPairCounter].object, output[wordPairCounter].type);
		wordPairCounter++;
	}
	 fclose(file);
	 *wordCount = wordPairCounter;
}

void importUsers(char * filename, User * output, int * userCount){
	char buffer[1024] = "0";
	FILE *file;
	
	file = fopen(filename, "r");
    if (file == NULL){
        printf("Could not open file %s",filename);
    }
	int currentUser = -1; //-1 to prevent header
	while (fgets(buffer, sizeof(buffer), file) != NULL){
		if (currentUser < 0){ //skip header line
			currentUser++;
			continue;
		}
		output[currentUser].username = malloc(20*sizeof(char));
		output[currentUser].password = malloc(20*sizeof(char));
		sscanf( buffer, "%s %s", output[currentUser].username, output[currentUser].password);
		output[currentUser].games_played = 0;
		output[currentUser].games_won = 0;
		currentUser++;
	}
	fclose(file);
	*userCount = currentUser;
}