#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 512

typedef struct {
	char username[32];
	char password[32];
	char contacts[10][32];
	int contact_count;
} User;

typedef struct {
	char from[32];
	char to[32];
	char message[BUFFER_SIZE];
} Message;

User users[10];
int user_count = 0;
Message messages[100];
int message_count = 0;
pthread_mutex_t lock;

void *client_handler(void *);
bool authenticate_user(const char *, const char *);
User* get_user(const char *);
void add_message(const char *, const char *, const char *);
void handle_signup(int, char *, char *);
void handle_login(int, char *, char *);
void handle_add_contact(int, const char *, const char *);
void handle_get_contacts(int, const char *);
void handle_send_message(int, const char *, const char *, const char *);
void handle_receive_messages(int, const char *);

int main() {

	int server_fd, client_sock, c;
	struct sockaddr_in server, client;

	pthread_mutex_init(&lock, NULL);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %d\n", PORT);
	c = sizeof(struct sockaddr_in);

	while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&c))) {
		printf("Connection accepted\n");

		pthread_t sniffer_thread;
		int *new_sock = malloc(1);
		*new_sock = client_sock;

		if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock) < 0) {
			perror("Could not create thread");
			free(new_sock);
			return 1;
		}

		printf("Handler assigned\n");
	}

	if (client_sock < 0) {
		perror("Accept failed");
		return 1;
	}

	pthread_mutex_destroy(&lock);
	return 0;
}

void *client_handler(void *socket_desc) {

	int sock = *(int *)socket_desc;
	int read_size;
	char client_message[BUFFER_SIZE];

	while ((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
		client_message[read_size] = '\0';
		printf("Received: %s\n", client_message);

		char *command = strtok(client_message, ":");
		if (strcmp(command, "signup") == 0) {

			char *username = strtok(NULL, ":");
			char *password = strtok(NULL, ":");
			handle_signup(sock, username, password);
		} 

		else if (strcmp(command, "login") == 0) {

			char *username = strtok(NULL, ":");
			char *password = strtok(NULL, ":");
			handle_login(sock, username, password);
		} 

		else if (strcmp(command, "add_contact") == 0) {
			char *username = strtok(NULL, ":");
			char *contact = strtok(NULL, ":");
			handle_add_contact(sock, username, contact);
		}

		else if (strcmp(command, "get_contacts") == 0) {
			char *username = strtok(NULL, ":");
			handle_get_contacts(sock, username);
		}

		else if (strcmp(command, "send") == 0) {

			char *from = strtok(NULL, ":");
			char *to = strtok(NULL, ":");
			char *msg = strtok(NULL, ":");
			handle_send_message(sock, from, to, msg);
		} 

		else if (strcmp(command, "receive") == 0) {
			char *username = strtok(NULL, ":");
			handle_receive_messages(sock, username);
		}

		memset(client_message, 0, BUFFER_SIZE);
	}

	if (read_size == 0) printf("Client disconnected\n");

	else if (read_size == -1) perror("Recv failed");

	free(socket_desc);
	return 0;
}

bool authenticate_user(const char *username, const char *password) {

	for (int i = 0; i < user_count; ++i) {

		if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
			return true;
		}
	}
	return false;
}

User* get_user(const char *username) {

	for (int i = 0; i < user_count; ++i) {

		if (strcmp(users[i].username, username) == 0) return &users[i];
	}
	return NULL;
}

void add_message(const char *from, const char *to, const char *msg) {

	pthread_mutex_lock(&lock);
	strncpy(messages[message_count].from, from, sizeof(messages[message_count].from) - 1);
	strncpy(messages[message_count].to, to, sizeof(messages[message_count].to) - 1);
	strncpy(messages[message_count].message, msg, sizeof(messages[message_count].message) - 1);
	message_count++;
	pthread_mutex_unlock(&lock);
}

void handle_signup(int client_sock, char *username, char *password) {

	for (int i = 0; i < user_count; ++i) {

		if (strcmp(users[i].username, username) == 0) {

			char response[BUFFER_SIZE] = "Username already exists\n";
			send(client_sock, response, strlen(response), 0);
			return;
		}
	}

	pthread_mutex_lock(&lock);
	strncpy(users[user_count].username, username, sizeof(users[user_count].username) - 1);
	strncpy(users[user_count].password, password, sizeof(users[user_count].password) - 1);
	users[user_count].contact_count = 0;
	user_count++;
	pthread_mutex_unlock(&lock);

	char response[BUFFER_SIZE] = "Signup successful\n";
	send(client_sock, response, strlen(response), 0);
}

void handle_login(int client_sock, char *username, char *password) {

	if (authenticate_user(username, password)) {
		char response[BUFFER_SIZE] = "Login successful\n";
		send(client_sock, response, strlen(response), 0);
	}

	else {
		char response[BUFFER_SIZE] = "Invalid username or password\n";
		send(client_sock, response, strlen(response), 0);
	}
}

void handle_add_contact(int client_sock, const char *username, const char *contact) {

	User *user = get_user(username);

	if (user) {

		pthread_mutex_lock(&lock);
		strncpy(user->contacts[user->contact_count], contact, sizeof(user->contacts[user->contact_count]) - 1);
		user->contact_count++;
		pthread_mutex_unlock(&lock);
		char response[BUFFER_SIZE] = "Contact added successfully\n";
		send(client_sock, response, strlen(response), 0);
	}

	else {
		char response[BUFFER_SIZE] = "User not found\n";
		send(client_sock, response, strlen(response), 0);
	}
}

void handle_get_contacts(int client_sock, const char *username) {

	User *user = get_user(username);

	if (user) {

		char response[BUFFER_SIZE] = "";

		for (int i = 0; i < user->contact_count; ++i) {

			char contact[64];
			snprintf(contact, sizeof(contact), "%d. %s\n", i + 1, user->contacts[i]);

			if (strlen(response) + strlen(contact) < BUFFER_SIZE) {
				strncat(response, contact, BUFFER_SIZE - strlen(response) - 1);
			}

			else break;
		}
		send(client_sock, response, strlen(response), 0);
	} 

	else {
		char response[BUFFER_SIZE] = "User not found\n";
		send(client_sock, response, strlen(response), 0);
	}
}

void handle_send_message(int client_sock, const char *from, const char *to, const char *msg) {

	add_message(from, to, msg);
	char response[BUFFER_SIZE] = "Message sent\n";
	send(client_sock, response, strlen(response), 0);
}

void handle_receive_messages(int client_sock, const char *username) {

	char response[BUFFER_SIZE] = "";

	for (int i = 0; i < message_count; ++i) {

		if (strcmp(messages[i].to, username) == 0) {

			char message[BUFFER_SIZE];
			snprintf(message, sizeof(message), "%s: %s\n", messages[i].from, messages[i].message);

			if (strlen(response) + strlen(message) < BUFFER_SIZE) {

				strncat(response, message, BUFFER_SIZE - strlen(response) - 1);
			} 

			else break;
		}
	}

	send(client_sock, response, strlen(response) > 0 ? strlen(response) : sizeof("No new messages\n"), 0);
}

