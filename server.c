#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>

#include "keyValStore.h"

#define PORT 5678
#define BUFFER_SIZE 1024

void processCommand(int clientSocket, char *cmd);
void handleChildProcesses(int sig);

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    initializeStore();
    signal(SIGCHLD, handleChildProcesses);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (clientSocket < 0) {
            perror("Accept");
            continue;
        }

        if (fork() == 0) { // Child process
            close(serverSocket);
            char buffer[BUFFER_SIZE];
            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                int readBytes = read(clientSocket, buffer, BUFFER_SIZE - 1);
                if (readBytes < 0) {
                    perror("Read error");
                    break;
                }
                if (readBytes == 0) {
                    break; // Connection closed by client
                }

                buffer[readBytes] = '\0';
                processCommand(clientSocket, buffer);
            }

            close(clientSocket);
            exit(0);
        } else {
            close(clientSocket); // Parent closes the child's socket
        }
    }

    close(serverSocket);
    return 0;
}

void processCommand(int clientSocket, char *cmd) {
    char key[256], value[256], response[512];
    // Check if the command is QUIT
    if (strncmp(cmd, "QUIT", 4) == 0) {
        sprintf(response, "Connection closing...\n");
        send(clientSocket, response, strlen(response), 0);
        close(clientSocket);  // Schließt die Verbindung zum Client
        exit(0); // Beendet den Child-Prozess sauber
    }
    if (sscanf(cmd, "GET %255s", key) == 1) {
        if (get(key, value) == 0) {
            sprintf(response, "GET:%s:%s", key, value);
        } else {
            sprintf(response, "GET:%s:key_nonexistent", key);
        }
    } else if (sscanf(cmd, "PUT %255s %255s", key, value) == 2) {
        put(key, value);
        sprintf(response, "PUT:%s:%s", key, value);
    } else if (sscanf(cmd, "DEL %255s", key) == 1) {
        if (del(key) == 0) {
            sprintf(response, "DEL:%s:key_deleted", key);
        } else {
            sprintf(response, "DEL:%s:key_nonexistent", key);
        }
    } else {
        sprintf(response, "Invalid command");
    }

    send(clientSocket, response, strlen(response), 0);
}

void handleChildProcesses(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0); // Clean up zombie processes
}