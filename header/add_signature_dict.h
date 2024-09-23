#ifndef ADD_SIGNATURE_DICT_H
#define ADD_SIGNATURE_DICT_H
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Class declaration
class AddSignatureDict {

public:
    // Constructor
    AddSignatureDict();

    // Member function declaration
    std::unordered_map<std::string, std::vector<uint8_t> > add_signaturedict(std::vector<unsigned char> pdf_content, int page, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign);
    std::unordered_map<std::string, std::vector<uint8_t> > add_signaturedict_with_image(std::vector<unsigned char> pdf_content, const char* input_image_path_string, int page, float x, float y, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign);
    std::unordered_map<std::string, std::vector<uint8_t> > add_signaturedict_with_qr(std::vector<unsigned char> pdf_content, const char* url, int page, float x, float y, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign);
    std::unordered_map<std::string, std::vector<uint8_t> > add_signaturedict_image_to_char(std::vector<unsigned char> pdf_content, const char* character, const char* input_image_path_string, int page, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign);
    std::unordered_map<std::string, std::vector<uint8_t> > add_signaturedict_qr_to_char(std::vector<unsigned char> pdf_content, const char* character, const char* url, int page, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign);
};

#endif