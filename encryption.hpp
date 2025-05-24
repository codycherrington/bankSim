#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <string>

std::string generateSalt(size_t length = 8);
std::string deriveKey(const std::string& password, const std::string& salt);
std::string encrypt(const std::string& data, const std::string& password);
std::string decrypt(const std::string& encrypted, const std::string& password);

#endif