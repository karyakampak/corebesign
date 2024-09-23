#ifndef ADD_PLACEHOLDER_H
#define ADD_PLACEHOLDER_H
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Class declaration
class AddPlaceHolder {

public:
    // Constructor
    AddPlaceHolder();

    // Member function declaration
    std::unordered_map<std::string, std::string> addPlaceholder(int type, const char* pdf_path, int isProtected, const char* character, const char* imageorurl, int page, float x, float y, float width, float height, int isSeal);
};

#endif