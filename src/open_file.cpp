#include "../header/open_file.h"
#include <Python.h>
#include <iostream>
#include <fstream>
#include <vector>

OpenFile::OpenFile() {
    // Initialize private member variables or perform any necessary setup
}

std::vector<uint8_t> get_decrypted_pdf(const std::vector<uint8_t>& pdf_data, const std::string& password) {
    // Initialize the Python interpreter
    Py_Initialize();

    // Load the Python script
    PyObject* pName = PyUnicode_DecodeFSDefault("besign_helper");
    PyObject* pModule = PyImport_Import(pName);
    Py_XDECREF(pName);
    if (pModule == nullptr) {
        PyErr_Print();
        std::cerr << "Failed to load Python module.\n";
        Py_Finalize();
        return {};
    }

    // Get the function from the module
    PyObject* pFunc = PyObject_GetAttrString(pModule, "decrypt_pdf_from_bytes");
    if (pFunc == nullptr || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        std::cerr << "Failed to find function 'decrypt_pdf_from_bytes'.\n";
        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
        Py_Finalize();
        return {};
    }

    // Convert input data to Python objects
    PyObject* pPdfData = PyBytes_FromStringAndSize(reinterpret_cast<const char*>(pdf_data.data()), pdf_data.size());
    PyObject* pPassword = PyUnicode_FromString(password.c_str());

    // Call the Python function
    PyObject* pArgs = PyTuple_Pack(2, pPdfData, pPassword);
    PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
    Py_XDECREF(pArgs);
    Py_XDECREF(pPdfData);
    Py_XDECREF(pPassword);
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    if (pResult == nullptr) {
        PyErr_Print();
        std::cerr << "Failed to call Python function.\n";
        Py_Finalize();
        return {};
    }

    // Convert result to std::vector<uint8_t>
    std::vector<uint8_t> result;
    if (PyBytes_Check(pResult)) {
        Py_ssize_t size = PyBytes_Size(pResult);
        const char* data = PyBytes_AsString(pResult);
        result.assign(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + size);
    } else {
        std::cerr << "Python function did not return bytes.\n";
    }

    Py_XDECREF(pResult);
    Py_Finalize();
    return result;
}

std::vector<uint8_t> OpenFile::open(const char* path, int isProtected) {
        // Open file stream
        std::ifstream fileStream(path, std::ios::binary);
        if (!fileStream.is_open()) {
            std::cerr << "Error opening file: " << path << std::endl;
        }

        // Get file size
        fileStream.seekg(0, std::ios::end);
        std::streampos fileSize = fileStream.tellg();
        fileStream.seekg(0, std::ios::beg);

        // Read file content into vector
        std::vector<uint8_t> fileContent(fileSize);
        if (!fileStream.read(reinterpret_cast<char*>(fileContent.data()), fileSize)) {
            std::cerr << "Error reading file: " << path << std::endl;
        }

        // Close file stream
        fileStream.close();

        if(isProtected == 1){

            // Example usage
            std::string password = "karyakampak";
            std::vector<uint8_t> decrypted_pdf = get_decrypted_pdf(fileContent, password);

            if (!decrypted_pdf.empty()) {
                std::cout << "Decrypted PDF size: " << decrypted_pdf.size() << " bytes\n";
                // Further processing of decrypted_pdf
            } else {
                std::cerr << "Failed to decrypt PDF.\n";
            }
            return decrypted_pdf;
        }

        return fileContent;
}