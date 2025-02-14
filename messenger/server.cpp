#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#define PORT 3000
#define BUFFER_SIZE 1024

//обработка ошибок
void maybe_error(const char* message) {
    std::cerr << "Error: " << message << std::endl;
    exit(EXIT_FAILURE);
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, client_1, client_2;
    sockaddr_in serverADDR, clientADDR;
    int addrLen = sizeof(clientADDR);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        maybe_error("WSAStartup is not initialized");

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //адрес сервера
    serverADDR.sin_family = AF_INET;
    serverADDR.sin_addr.s_addr = INADDR_ANY;
    serverADDR.sin_port = htons(PORT);

    //привязка сокета к адресу и порту
    if (bind(serverSocket, (struct sockaddr*)&serverADDR, sizeof(serverADDR)) == SOCKET_ERROR)
        maybe_error("Binding failed");

    //ожидание подключений
    if (listen(serverSocket, 2) == SOCKET_ERROR)
        maybe_error("Listening failed");

    std::cout << "Server is running on port " << PORT << "...\nWaiting for 2 clients..." << std::endl;

    //первый клиент
    client_1 = accept(serverSocket, (struct sockaddr*)&clientADDR, &addrLen);
    if (client_1 == INVALID_SOCKET)
        maybe_error("First client is failed to connect");
    std::cout << "First client connected!" << std::endl;

    //второй клиент
    client_2 = accept(serverSocket, (struct sockaddr*)&clientADDR, &addrLen);
    if (client_2 == INVALID_SOCKET)
        maybe_error("Second client is failed to connect");
    std::cout << "Second client connected!" << std::endl;

    //получение ключей(добрые шифровальщики)
    char keyBuffer1[BUFFER_SIZE], keyBuffer2[BUFFER_SIZE];
    int key_1 = recv(client_1, keyBuffer1, BUFFER_SIZE, 0);
    int key_2 = recv(client_2, keyBuffer2, BUFFER_SIZE, 0);

    send(client_2, keyBuffer1, key_1, 0);
    send(client_1, keyBuffer2, key_2, 0);

    const char* turnMessage = "Your turn to send a message.";
    send(client_1, turnMessage, strlen(turnMessage), 0);

    //основной цикл чата
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int received = recv(client_1, buffer, BUFFER_SIZE, 0);
        if (received <= 0) {
            std::cout << "First client disconnected" << std::endl;
            break;
        }
        send(client_2, buffer, received, 0);
        send(client_2, turnMessage, strlen(turnMessage), 0);

        memset(buffer, 0, BUFFER_SIZE);
        received = recv(client_2, buffer, BUFFER_SIZE, 0);
        if (received <= 0) {
            std::cout << "Second client disconnected" << std::endl;
            break;
        }
        send(client_1, buffer, received, 0);
        send(client_1, turnMessage, strlen(turnMessage), 0);
    }

    closesocket(client_1);
    closesocket(client_2);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}