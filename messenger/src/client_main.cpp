//main-функция клиента

#include "client.hpp"
#include <iostream>

int main() {
    std::cout << "1. Regular user\n2. Admin\nChoose mode: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(); //игнор '\n' после ввода числа

    std::string name;
    bool isAdmin = false;     
    int key = 0;

    if (choice == 2) {
        std::cout << "Enter admin name: ";
        std::getline(std::cin, name);
        std::cout << "Enter admin password: ";
        std::string password;
        std::getline(std::cin, password);
        name = "admin " + name + "|" + password;
        isAdmin = true;
    } else {
        std::cout << "Enter your name: ";
        std::getline(std::cin, name);
        std::cout << "Enter your encryption key: ";
        std::cin >> key;
        std::cin.ignore(); // очищаем буфер после ввода числа
    }

    Client client(name, isAdmin ? 0 : key, isAdmin);
    client.connectToServer("127.0.0.1", 3000);
    client.start();
    return 0;
}
