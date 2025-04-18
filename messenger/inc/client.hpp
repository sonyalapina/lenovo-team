//определение класса Client

#pragma once
#include <winsock2.h>
#include <string>

class Client {
private:
    SOCKET socket;          
    std::string name;      
    int key;               
    bool isAdmin;           

public:
    Client(const std::string& name, int key, bool isAdmin);
    ~Client();

    void connectToServer(const std::string& ip, int port);
    
    void start();
};
