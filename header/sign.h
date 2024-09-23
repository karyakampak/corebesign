#ifndef SIGN_H
#define SIGN_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

// Class declaration
class Sign {

public:
    // Constructor
    Sign();

    // Member function declaration
    std::vector<uint8_t> sign(std::vector<uint8_t>& placedHolder);
};

#endif