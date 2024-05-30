#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_SIZE 475

char username[32];

void login(int);
void signup(int);
void add_contact(int);
void msg_contact(int);
void view_messages(int);

int main() {

	int sock = 0;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}

	while (1) {
		int choice;
		printf("1. Login\n2. Signup\n3. Exit\n");
		scanf("%d", &choice);
		getchar();

		if (choice == 1)  login(sock); 

		else if (choice == 2) signup(sock); 

		else if (choice == 3) {
			close(sock);
			exit(0);
		}

		else printf("Invalid choice\n");
	}

	return 0;
}

void login(int sock) {

	char password[32], buffer[BUFFER_SIZE];
	printf("Enter username: ");
	scanf("%s", username);
	printf("Enter password: ");
	scanf("%s", password);

	snprintf(buffer, BUFFER_SIZE, "login:%s:%s", username, password);
	send(sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(sock, buffer, BUFFER_SIZE);
	printf("%s", buffer);

	if (strstr(buffer, "successful")) {

		while (1) {
			int choice;
			printf("1. Add contacts\n2. Msg contacts\n3. View messages\n4. Exit\n");
			scanf("%d", &choice);
			getchar(); 

			if (choice == 1) add_contact(sock);

			else if (choice == 2) msg_contact(sock);

			else if (choice == 3) view_messages(sock);

			else if (choice == 4) break;

			else printf("Invalid choice\n");
		}
	}
}

void signup(int sock) {

	char password[32], confirm_password[32], buffer[BUFFER_SIZE];
	printf("Enter username: ");
	scanf("%s", username);
	printf("Enter password: ");
	scanf("%s", password);
	printf("Confirm password: ");
	scanf("%s", confirm_password);

	if (strcmp(password, confirm_password) != 0) {
		printf("Passwords do not match\n");
		return;
	}

	snprintf(buffer, BUFFER_SIZE, "signup:%s:%s", username, password);
	send(sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(sock, buffer, BUFFER_SIZE);
	printf("%s", buffer);
}

void add_contact(int sock) {

	char contact[32], buffer[BUFFER_SIZE];
	printf("Enter contact username: ");
	scanf("%s", contact);

	snprintf(buffer, BUFFER_SIZE, "add_contact:%s:%s", username, contact);
	send(sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(sock, buffer, BUFFER_SIZE);
	printf("%s\n", buffer);
}

void msg_contact(int sock) {

	char buffer[BUFFER_SIZE];
	snprintf(buffer, BUFFER_SIZE, "get_contacts:%s", username);
	send(sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(sock, buffer, BUFFER_SIZE);
	printf("Your contacts:\n%s", buffer);

	char *contact_list = strdup(buffer);
	int choice;
	printf("Select contact to message: ");
	scanf("%d", &choice);
	getchar();

	char *line = strtok(contact_list, "\n");
	for (int i = 1; i < choice; ++i) {
		line = strtok(NULL, "\n");
		if (line == NULL) {
			printf("Invalid choice\n");
			free(contact_list);
			return;
		}
	}

	char to[32];
	sscanf(line, "%*d. %s", to);

	char msg[MAX_MESSAGE_SIZE];
	printf("Message: ");
	fgets(msg, MAX_MESSAGE_SIZE, stdin);
	msg[strcspn(msg, "\n")] = '\0'; 
	snprintf(buffer, BUFFER_SIZE, "send:%s:%s:%s", username, to, msg);
	send(sock, buffer, strlen(buffer), 0);
	free(contact_list);

	memset(buffer, 0, BUFFER_SIZE);
	read(sock, buffer, BUFFER_SIZE);
	printf("%s\n", buffer);
}

void view_messages(int sock) {

	char buffer[BUFFER_SIZE];
	snprintf(buffer, BUFFER_SIZE, "receive:%s", username);
	send(sock, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	int valread = read(sock, buffer, BUFFER_SIZE);
	if (valread > 0) {
		printf("Received messages:\n%s", buffer);
	} else {
		printf("No new messages.\n");
	}
}
