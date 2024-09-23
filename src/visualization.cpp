#include <Python.h>
#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <stdexcept>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "../header/addons.h"
#include "../header/visualization.h"
#include <random>
#include <algorithm>

Visualization::Visualization() {
    // Initialize private member variables or perform any necessary setup
}

// Function to decode a Base64-encoded string using OpenSSL
std::string base64_decode(const std::string& encoded) {
    std::string decoded;

    // Create a BIO for Base64 decoding
    BIO *bio = BIO_new(BIO_f_base64());
    BIO *bio_mem = BIO_new_mem_buf(encoded.data(), encoded.size());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Disable newline handling

    // Chain the memory BIO with the Base64 BIO
    bio = BIO_push(bio, bio_mem);

    // Decode
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = BIO_read(bio, buffer, sizeof(buffer))) > 0) {
        decoded.append(buffer, bytes_read);
    }

    // Clean up
    BIO_free_all(bio);

    return decoded;
}

std::vector<uint8_t> process_image(std::vector<unsigned char> image_data) {
    Py_Initialize();

    // Import the Cython module
    PyObject* pName = PyUnicode_DecodeFSDefault("besign_helper");
    PyObject* pModule = PyImport_Import(pName);
    Py_XDECREF(pName);

    if (pModule != nullptr) {
        // Get reference to the function
        PyObject* pFunc = PyObject_GetAttrString(pModule, "process_image");
        if (pFunc && PyCallable_Check(pFunc)) {
            // Prepare arguments for the Python function
            PyObject* pArgs = PyTuple_New(1);  // 7 arguments for add_qr_to_char
            PyTuple_SetItem(pArgs, 0, PyBytes_FromStringAndSize(reinterpret_cast<const char*>(image_data.data()), image_data.size()));

            // Call the function
            PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
            if (pValue != nullptr) {
                // Process the result
                std::string result = PyUnicode_AsUTF8(pValue);
                std::string decoded_data = base64_decode(result);
                std::vector<uint8_t> pdf(decoded_data.begin(), decoded_data.end());
                // std::cout << decoded_data << std::endl;
                Py_DECREF(pValue);
                return pdf;
            } else {
                PyErr_Print();
            }
            Py_XDECREF(pArgs);
        } else {
            PyErr_Print();
        }
        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
    } else {
        PyErr_Print();
    }

    Py_Finalize();
    
    return {};
}

std::vector<double> find_char_position(std::vector<unsigned char> pdf_data, int page_num, const char* target_char) {
    std::vector<double> result;
    // Initialize the Python interpreter
    Py_Initialize();

    
    // Import the Cython module
    PyObject* pName = PyUnicode_DecodeFSDefault("besign_helper");
    PyObject* pModule = PyImport_Import(pName);
    Py_XDECREF(pName);
    
    if (pModule != nullptr) {
        // Get the function
        PyObject* pFunc = PyObject_GetAttrString(pModule, "find_char_position");
        
        if (pFunc && PyCallable_Check(pFunc)) {
            // Prepare arguments
            PyObject* pArgs = PyTuple_Pack(3,
                PyBytes_FromStringAndSize(reinterpret_cast<const char*>(pdf_data.data()), pdf_data.size()),
                PyLong_FromLong(page_num),
                PyUnicode_FromString(target_char)
            );
            
            // Call the function
            PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
            Py_XDECREF(pArgs);

            if (pValue != nullptr) {
                // Check if the returned value is a tuple
                if (PyTuple_Check(pValue)) {
                    // Get the number of elements in the tuple
                    Py_ssize_t tuple_size = PyTuple_Size(pValue);
                    
                    // Resize the vector to fit the number of elements
                    result.resize(tuple_size);
                    
                    // Extract elements from the tuple and store them in the vector
                    for (Py_ssize_t i = 0; i < tuple_size; ++i) {
                        PyObject* item = PyTuple_GetItem(pValue, i);
                        if (PyFloat_Check(item)) {
                            result[i] = PyFloat_AsDouble(item);
                        } else {
                            std::cerr << "Tuple item is not a float" << std::endl;
                        }
                    }
                    return result;
                } else {
                    std::cerr << "Returned value is not a tuple" << std::endl;
                }
                Py_XDECREF(pValue);
            } else {
                PyErr_Print();
                fprintf(stderr, "Call failed\n");
            }
            Py_XDECREF(pFunc);
        } else {
            PyErr_Print();
            fprintf(stderr, "Cannot find function\n");
        }
        Py_XDECREF(pModule);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load module\n");
    }
    
    // Finalize the Python interpreter
    Py_Finalize();

    return result;
}

std::vector<double> Visualization::get_position(std::vector<unsigned char> pdf_data, int page_num, const char* target_char){
    std::vector<double> obj = find_char_position(pdf_data, page_num, target_char);
    // std::vector<uint8_t> obj = process_qr(url);
    
    return obj;
}

std::vector<uint8_t> Visualization::get_image(std::vector<unsigned char> image_data){\
    std::vector<uint8_t> obj = process_image(image_data);
    
    return obj;
}
