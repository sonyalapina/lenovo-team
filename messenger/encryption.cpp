#include <string>
#include <cctype> 

std::string encrypt(const std::string& message, int key) {
    std::string encrypted = message;
    for (char& c : encrypted) {
        if (isalpha(c)) { 
            char offset = islower(c) ? 'à' : 'À';
            c = (c - offset + key) % 33 + offset;
        }
    }
    return encrypted;
}

std::string decrypt(const std::string& message, int key) {
    std::string decrypted = message;
    for (char& c : decrypted) {
        if (isalpha(c)) { 
            char offset = islower(c) ? 'à' : 'À';
            c = (c - offset - key + 33) % 33 + offset; 
        }
    }
    return decrypted;
}