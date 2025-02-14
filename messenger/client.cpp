#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "encryption.cpp"
#pragma comment(lib, "ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define PORT 3000
#define BUFFER_SIZE 1024

void handleError(const char* message) {
    std::cerr << "Error: " << message << std::endl;
    exit(EXIT_FAILURE);
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        handleError("WSAStartup failed");
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        handleError("Socket creation failed");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        handleError("Connection failed");
    }

    int key;
    std::cout << "Enter encryption key(number): ";
    std::cin >> key;
    std::cin.ignore();

    send(clientSocket, std::to_string(key).c_str(), std::to_string(key).length(), 0);

    char keyBuffer[BUFFER_SIZE];
    int keyBytesReceived = recv(clientSocket, keyBuffer, BUFFER_SIZE, 0);
    if (keyBytesReceived > 0) {
        int otherKey = atoi(keyBuffer);

        if (key != otherKey) {
            std::cout << "Error: Encryption keys do not match! Disconnecting..." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        } else {
            std::cout << "Encryption keys match! Starting chat..." << std::endl;
        }
    }

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cout << "Disconnected from server!" << std::endl;
            break;
        }

        if (strstr(buffer, "Your turn to send a message.") != nullptr) {
            std::cout << "You: ";
            std::string message;
            std::getline(std::cin, message);

            if (message == "/exit") {
                std::cout << "Exiting chat...\n";
                break;
            }

            std::string encryptedMessage = encrypt(message, key);
            send(clientSocket, encryptedMessage.c_str(), encryptedMessage.length(), 0);
        } else {
            std::string receivedMessage = decrypt(std::string(buffer, bytesReceived), key);
            std::cout << "Received: " << receivedMessage << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}