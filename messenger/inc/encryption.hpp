#pragma once
#include <string>

class Encryption {
public:
    static std::string encrypt(const std::string& message, int key);
    static std::string decrypt(const std::string& message, int key);
};