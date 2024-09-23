#ifndef CMS_H
#define CMS_H
#include <vector>
#include <iostream>
#include <cstdint>

// Class declaration
class CMS {

public:
    // Constructor
    CMS();

    // Member function declaration
    std::string generateCMS_file(const std::string& pkcs12Path, const std::string& password, const std::string& data);
    std::string generateCMS(const std::vector<uint8_t>& p12, const std::string& password, const std::string& data);
};

#endif