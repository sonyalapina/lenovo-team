//описание класса Client

#include "client.hpp"
#include "encryption.hpp"
#include <iostream>
#include <thread>
#include <ws2tcpip.h>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

//конструктор
Client::Client(const std::string& name, int key, bool isAdmin) 
    : name(name), key(key), isAdmin(isAdmin) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); 
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
}

//деструктор
Client::~Client() {
    closesocket(socket);
    WSACleanup();
}

void Client::connectToServer(const std::string& ip, int port) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; 
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr); 
    serverAddr.sin_port = htons(port); 

    if (connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server" << std::endl;
        exit(1);
    }
}

void Client::start() {
    send(socket, name.c_str(), static_cast<int>(name.size()), 0);

    //создание потока для приема сообщений от сервера
    std::thread receiver([this]() {
        char buffer[2048];
        while (true) {
            int bytes = recv(this->socket, buffer, sizeof(buffer), 0);
            if (bytes <= 0) {
                std::cout << "Disconnected from server" << std::endl;
                closesocket(this->socket);
                exit(0);
            }

            std::string received(buffer, bytes);

            //обработка зашифрованных сообщений
            if (received.rfind("ENC:", 0) == 0) {
                std::string payload = received.substr(4);
                size_t delimiterPos = payload.find(": ");
                if (delimiterPos != std::string::npos) {
                    std::string sender = payload.substr(0, delimiterPos);
                    std::string encryptedText = payload.substr(delimiterPos + 2);
                    std::string decryptedText = Encryption::decrypt(encryptedText, key);
                    std::cout << sender << ": " << decryptedText << std::endl;
                } 
            } 
            //обработка обычных сообщений
            else if (received.rfind("PLAIN:", 0) == 0) {
                std::string plainMsg = received.substr(6);
                std::cout << plainMsg;
                
                if (plainMsg.find("You were kicked by an administrator!") != std::string::npos) {
                    std::cout << "Press any key to exit..." << std::endl;
                    closesocket(this->socket);
                    exit(0);
                }
            } 
        }
    });

    while (true) {
        std::string message;
        std::getline(std::cin, message);
        
        if (message == "/exit") {
            closesocket(socket);
            break;
        }
        
        //обработка команды кика
        if (isAdmin && message.rfind("/kick ", 0) == 0) {
            std::string toSend = message + "\n";
            send(socket, toSend.c_str(), static_cast<int>(toSend.size()), 0);
        } else {
            std::string encryptedText = Encryption::encrypt(message, key);
            std::string toSend = "ENC:" + name + ": " + encryptedText + "\n";
            send(socket, toSend.c_str(), static_cast<int>(toSend.size()), 0);
        }
    }

    //ожидание завершения потока приемника
    receiver.join();
}

