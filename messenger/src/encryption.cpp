#include "encryption.hpp"

std::string Encryption::encrypt(const std::string& message, int key) {
    std::string result = message;
    for (char& c : result) c ^= key;
    return result;
}

std::string Encryption::decrypt(const std::string& message, int key) {
    return encrypt(message, key);
}