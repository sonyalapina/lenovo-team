// описание класса Server

#include "server.hpp"
#include "encryption.hpp"
#include <iostream>
#include <thread>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

Server::Server() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}

Server::~Server() {
    closesocket(serverSocket); //серверный
    for (SOCKET sock : clients) closesocket(sock); //все клиентские
    WSACleanup();
}

void Server::addAdmin(const std::string& name, const std::string& password) {
    adminCredentials[name] = password; //пара имя-пароль
}

void Server::start(int port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server listening on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &len); //принятие входящего подключения

        //обработка клиента в отдельном потоке
        std::thread([this, clientSocket]() {
            char buffer[1024];
            int bytes = recv(clientSocket, buffer, sizeof(buffer), 0); //прием данных
            if (bytes <= 0) {
                closesocket(clientSocket);
                return;
            }

            std::string name(buffer, bytes); //данные - строка
            bool isAdmin = false;

            if (name.rfind("admin ", 0) == 0) { //подстрока в начале строки
                std::string credentials = name.substr(6); //len(admin ) == 6
                size_t sep = credentials.find('|');
                if (sep == std::string::npos) { //нет элемента
                    std::string msg = "PLAIN:Invalid admin format\n";
                    send(clientSocket, msg.c_str(), static_cast<int>(msg.size()), 0);
                    closesocket(clientSocket);
                    return;
                }
                std::string adminName = credentials.substr(0, sep);
                std::string password = credentials.substr(sep + 1);

                if (adminCredentials.count(adminName) && adminCredentials[adminName] == password) { //кол-во эл-тов с ключом adminName && совпадение пароля
                    name = "admin " + adminName;
                    isAdmin = true;
                } else {
                    std::string msg = "PLAIN:Invalid admin credentials\n";
                    send(clientSocket, msg.c_str(), static_cast<int>(msg.size()), 0);
                    closesocket(clientSocket);
                    return;
                }
            }

            {
                std::lock_guard<std::mutex> lock(clientMutex);
                clients.push_back(clientSocket); //клиента в общий список
                clientNames[clientSocket] = name;
            }

            std::string connectMsg = "PLAIN:" + name + " has joined the chat\n";
            std::cout << name << " connected.\n";

            {
                std::lock_guard<std::mutex> lock(clientMutex);
                for (SOCKET sock : clients) {
                    if (sock != clientSocket) { //всем кроме себя
                        send(sock, connectMsg.c_str(), static_cast<int>(connectMsg.size()), 0);
                    }
                }
            }

            while (true) {
                bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytes <= 0) break;

                std::string received(buffer, bytes);

                if (isAdmin && received.rfind("/kick ", 0) == 0) {
                    std::string kickTarget = received.substr(6); //кого кикать
                    
                    kickTarget.erase(std::remove(kickTarget.begin(), kickTarget.end(), '\n'), kickTarget.end()); //удаляем /n /r
                    kickTarget.erase(std::remove(kickTarget.begin(), kickTarget.end(), '\r'), kickTarget.end());
                    
                    std::lock_guard<std::mutex> lock(clientMutex);
                    
                    //поиск жертвы
                    auto it = std::find_if(clients.begin(), clients.end(),
                        [&](const SOCKET& s) {
                            return clientNames[s] == kickTarget; //первого, чей сокет ассоциирован с именем
                        });

                    if (it != clients.end()) {
                        SOCKET targetSocket = *it;
                        
                        std::string disconnectMessage = "PLAIN:" + kickTarget + " was kicked by an administrator!\n";
                        for (auto sock : clients) {
                            if (sock != targetSocket) {
                                send(sock, disconnectMessage.c_str(), static_cast<int>(disconnectMessage.size()), 0);
                            }
                        }

                        std::string kickMsg = "PLAIN:You were kicked by an administrator!\n";
                        send(targetSocket, kickMsg.c_str(), static_cast<int>(kickMsg.size()), 0);
                        
                        shutdown(targetSocket, SD_BOTH); // остановка обмена данными
                        closesocket(targetSocket);
                        //закрытие и удаление из списков
                        clients.erase(it);
                        clientNames.erase(targetSocket);
                        
                        std::cout << "Kicked user: " << kickTarget << std::endl;
                    } else {
                        std::string errorMsg = "PLAIN:User '" + kickTarget + "' not found\n";
                        send(clientSocket, errorMsg.c_str(), static_cast<int>(errorMsg.size()), 0);
                    }
                } else {
                    std::lock_guard<std::mutex> lock(clientMutex);
                    for (SOCKET sock : clients) {
                        if (sock != clientSocket) {
                            send(sock, received.c_str(), static_cast<int>(received.size()), 0);
                        }
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(clientMutex);
                auto it = std::find(clients.begin(), clients.end(), clientSocket);
                if (it != clients.end()) {
                    std::string disconnectMsg = "PLAIN:" + clientNames[clientSocket] + " has left the chat\n";
                    clients.erase(it);
                    
                    for (SOCKET sock : clients) {
                        send(sock, disconnectMsg.c_str(), static_cast<int>(disconnectMsg.size()), 0);
                    }
                    
                    clientNames.erase(clientSocket);
                }
            }
            
            closesocket(clientSocket);
        }).detach(); //поток отделяется от главного
    }
}