#ifndef OPEN_FILE_H
#define OPEN_FILE_H
#include <iostream>
#include <vector>
#include <cstdint>

// Class declaration
class OpenFile {

public:
    // Constructor
    OpenFile();

    // Member function declaration
    std::vector<uint8_t> open(const char* path, int isProtected);
};

#endif