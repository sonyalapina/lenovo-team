//main-функия сервера

#include "server.hpp"
#include <iostream>

int main() {
    Server server;

    std::cout << "Setting up admin accounts (enter empty name to finish):\n";
    while (true) {
        std::string name, password;
        std::cout << "Admin name: ";
        std::getline(std::cin, name);
        if (name.empty()) break;

        std::cout << "Admin password: ";
        std::getline(std::cin, password);

        server.addAdmin(name, password);
    }

    server.start(3000);
    return 0;
}
