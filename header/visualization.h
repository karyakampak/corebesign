#ifndef ADD_VISUALIZATION_H
#define ADD_VISUALIZATION_H
#include <Python.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <random>
#include <algorithm>

// Class declaration
class Visualization {

public:
    // Constructor
    Visualization();

    // Member function declaration
    std::vector<double> get_position(std::vector<unsigned char> pdf_data, int page_num, const char* target_char);
    std::vector<uint8_t> get_image(std::vector<unsigned char> image_data);
};

#endif