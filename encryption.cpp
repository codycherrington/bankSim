#include "encryption.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// Generates a random salt string of the specified length
std::string generateSalt(size_t length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string salt;
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dist(gen)];
    }
    return salt;
}

// Derives a key from the password and salt (basic implementation for now)
std::string deriveKey(const std::string& password, const std::string& salt) {
    return password + salt; // placeholder — we'll strengthen this
}

// Encrypts data using XOR stream cipher
std::string encrypt(const std::string& data, const std::string& password) {
    std::string salt = generateSalt();
    std::string key = deriveKey(password, salt);

    std::string encrypted;
    for (size_t i = 0; i < data.size(); ++i) {
        char k = key[i % key.size()];
        encrypted += data[i] ^ k;
    }

    return salt + encrypted; // prepend salt to result
}

// Decrypts data using same XOR stream cipher
std::string decrypt(const std::string& encrypted, const std::string& password) {
    const size_t saltLength = 8;
    if (encrypted.size() < saltLength) throw std::runtime_error("Invalid encrypted data");

    std::string salt = encrypted.substr(0, saltLength);
    std::string data = encrypted.substr(saltLength);
    std::string key = deriveKey(password, salt);

    std::string decrypted;
    for (size_t i = 0; i < data.size(); ++i) {
        char k = key[i % key.size()];
        decrypted += data[i] ^ k;
    }

    return decrypted;
}