#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define ARRAY_SIZE 30  /* size of array to be sent */
#define PORT_NO 12345 /* PORT Number */
	
static int getLine (int *buff, size_t sz);

void getMessage(int sock_fd, char buffer[]);
void sendMessage(int sock_fd, char * message);
void getUserInput();
int userMenu();

int main(int argc, char *argv[]) {
	int sock_fd, numbytes, readline;  
	int input;
	char buf[MAXDATASIZE];
	char sendbuffer[ARRAY_SIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */

	for (int i = 0; i < MAXDATASIZE; i++) {
		buf[i] = 0;
	}

	if (argc != 3) {
		fprintf(stderr,"usage: client_hostname port \n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;      /* host byte order */
	their_addr.sin_port = htons(atoi(argv[2]));    /* short, network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */
	
	if (connect(sock_fd, (struct sockaddr *)&their_addr, \
	sizeof(struct sockaddr)) == -1) {
		perror("Failed to Connect");
		exit(1);
	}
	
	//check if server is full
	getMessage(sock_fd,buf);
	if (strcmp(buf,"accept")!=0){
		printf("Max users connected, Try again later");
		close(sock_fd);
		exit(0);
	}
	
	//----------------------------------------------------------------------------------
	// Authentication
	
	printf("Enter Username: ");
	getUserInput(sendbuffer, sizeof(sendbuffer));
	sendMessage(sock_fd, sendbuffer);
		
	printf("Enter Password: ");
	getUserInput(sendbuffer, sizeof(sendbuffer));
	sendMessage(sock_fd, sendbuffer);
	
	//result
	getMessage(sock_fd, buf);
	
	if (strcmp(buf,"authPass")!=0){ // not equal
		printf("Authentication Failed \n");
		close(sock_fd);
		exit(0);
	} else {
		printf("Authentication Accepted\n");
	}
	
	//----------------------------------------------------------------------------------
	// Main Connection Loop
	while (1){		
		//***hangman title
		
		int selection = userMenu(sock_fd);
		
		// User selects to play game
		if (selection == 1) {
			char guessed_letters[30] = "";
			char temp[50] = "";
			int guesses_left;
			int word_size;
			int won_game = 0;
			int counter = 0;
			
			// Gets the size of the word to be guessed
			getMessage(sock_fd, buf);
			strcpy(temp, buf);
			word_size = strlen(temp);
			
			// Calculates the number of guesses the user gets
			if ((word_size - 1 + 10) < 26) {
				guesses_left = (word_size -1 + 10);
			} else {
				guesses_left = 26;
			}
		
			// Game is run until guesses run out or user guesses word
			while (guesses_left > 0) {
				printf("\n\nGuessed letters: ");
				
				// Displays the letters already guessed
				for (int i = 0; i < sizeof(guessed_letters)/sizeof(char); i++){
					printf("%c", guessed_letters[i]);
				}
				
				// Displays how many guesses remaining
				printf("\nNumber of guesses left: %d\n", guesses_left);
				printf("Word: ");
				
				// Displays the word to be guessed with any correct letters guessed if any
				for (int i = 0; i < word_size; i++){
					printf("%c", temp[i]);
				}
				
				// Gets the guessed letter from user and sends to server and receives result 
				printf("\nEnter your guess: ");
				getUserInput(sendbuffer, sizeof(sendbuffer));
				sendMessage(sock_fd, sendbuffer);
				guesses_left--;
				guessed_letters[counter] = *sendbuffer;
				counter++;
				getMessage(sock_fd, buf);
				strcpy(temp, buf);
				
				// Checks to see if user has guessed the word 
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
			
			// Displays message if user won game
			if (won_game == 1 && guesses_left >= 0) {
				printf("\n\nGuessed letters: ");
				
				for (int i = 0; i < sizeof(guessed_letters)/sizeof(char); i++){
					printf("%c", guessed_letters[i]);
				}
				
				printf("\nNumber of guesses left: %d\n", guesses_left);
				printf("Word: ");
				
				for (int i = 0; i < word_size; i++){
					printf("%c", temp[i]);
				}
				
				printf("\n\nGame over\n\n");
				printf("Well done! You won this round of Hangman!\n\n");
			// Displays message if user lost game
			} else {
				printf("\n\nGuessed letters: ");
				
				for (int i = 0; i < sizeof(guessed_letters)/sizeof(char); i++){
					printf("%c", guessed_letters[i]);
				}
				
				printf("\nNumber of guesses left: %d\n", guesses_left);
				printf("Word: ");
				
				for (int i = 0; i < word_size; i++){
					printf("%c", temp[i]);
				}
				
				printf("\n\nGame over\n\n");
				printf("Bad luck! You have run out of guesses. The Hangman got you!\n\n");
			}	
		// User selects to display leader board
		} else if (selection == 2) {
			char name[10];
			int won = 0;
			int played = 0;
			int players = 0;
			
			// Gets how many players have played
			getMessage(sock_fd, buf);
			sscanf(buf, "%d", &players);
			
			// Display leaderboard if there has been at least 1 player
			if (players > 0) {
				for (int i = 0; i < players; i++) {
					getMessage(sock_fd, buf);
					sscanf(buf, "%s %d %d", name, &won, &played);
					printf("\nPlayer - %s\n", name);
					printf("Number of games won - %d\n", won);
					printf("Number of games played - %d\n", played);
					sendMessage(sock_fd,"received");
				}
			// Display message if there have been no players
			} else {
				printf("\n\nThere is no information currently stored in the Leader Board. Try again later\n\n");
			}
		// User selects to quit game
		} else {
				printf("\n\nGoodbye\n");
				exit(0);
		}
	}
	
	close(sock_fd);
	exit(0);
	
	return 0;
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


void sendMessage(int sock_fd, char *message){
	if (send(sock_fd,message, strlen(message),0)== -1) {
		fprintf(stderr, "Failure Sending Message\n");
		close(sock_fd);
		exit(1);
	}
}

int userMenu(int sock_fd) {
	char buffer[1024];
	int input;
	
	// Displays user menu selection
	do {
		printf("\n----- Enter your choice: -----\n\n");
		printf("1. Play Hangman\n");
		printf("2. Show Leaderboard\n");
		printf("3. Quit\n\n");
		printf("Selection option 1-3: ");
		
		// Gets input from user and sends to server
		getUserInput(buffer, sizeof(buffer));
		sendMessage(sock_fd, buffer);
		input = atoi(buffer);
		
		// Displays if user has chosen a selection not within range
		if ((input > 3) || (input < 1)){
			printf("Please enter a number within the range supplied \n");
		}

	} while ((input > 3) || (input < 1));
	
	return input;
}

void getUserInput(char * buffer, int size){
	fgets(buffer, size, stdin);
	buffer[strlen(buffer)-1] = '\0';
}