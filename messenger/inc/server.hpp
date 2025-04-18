//определение класса Server

#pragma once
#include <vector>
#include <map>
#include <string>
#include <mutex> //синхронизаци€ потоков
#include <winsock2.h>

class Server {
private:
    SOCKET serverSocket;
    std::vector<SOCKET> clients;
    std::map<SOCKET, std::string> clientNames;
    std::map<std::string, std::string> adminCredentials;
    std::mutex clientMutex;

public:
    Server();
    ~Server();
    void start(int port);
    void addAdmin(const std::string& name, const std::string& password);
};
