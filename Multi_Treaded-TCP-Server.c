#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10

// Structure to pass client socket and address to the thread
typedef struct {
    int clientSocket;
    struct sockaddr_in clientAddr;
} ClientInfo;

// Function to handle client requests
void* handleClient(void* arg) {
    ClientInfo* clientInfo = (ClientInfo*)arg;
    int clientSocket = clientInfo->clientSocket;

    char buffer[1024];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        printf("Received from %s:%d: %s", inet_ntoa(clientInfo->clientAddr.sin_addr),
               ntohs(clientInfo->clientAddr.sin_port), buffer);

        // Echo the received data back to the client
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    // Client disconnected or an error occurred
    printf("Client %s:%d disconnected.\n", inet_ntoa(clientInfo->clientAddr.sin_addr),
           ntohs(clientInfo->clientAddr.sin_port));

    close(clientSocket);
    free(clientInfo);

    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    pthread_t threads[MAX_CLIENTS];
    int threadCount = 0;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1) {
        perror("Socket listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            perror("Connection accept failed");
            continue;
        }

        printf("Client %s:%d connected.\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Create a thread to handle the client
        ClientInfo* clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->clientSocket = clientSocket;
        clientInfo->clientAddr = clientAddr;

        if (pthread_create(&threads[threadCount], NULL, handleClient, (void*)clientInfo) != 0) {
            perror("Thread creation failed");
            free(clientInfo);
        } else {
            threadCount++;
        }
    }

    // Close the server socket (this part is never reached in this example)

    return 0;
}
