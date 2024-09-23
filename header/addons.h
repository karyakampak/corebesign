#ifndef ADDONS_H
#define ADDONS_H
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint> // For uint8_t
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <regex>
#include <openssl/x509.h>
#include <openssl/x509v3.h> // Include this header for STACK_OF(X509_ATTRIBUTE)
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/cms.h>
#include <opencv2/opencv.hpp>
#include <qrencode.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <zlib.h> 

// Class declaration
class Addons {

public:
    // Constructor
    Addons();

    // Member function declaration
    std::vector<uint8_t> getSubVector(const std::vector<uint8_t>& vec, size_t x, size_t y);
    std::string vectorToString(const std::vector<uint8_t>& vec);
    std::vector<uint8_t> stringToVector(const std::string& str);
    std::vector<uint8_t> get_value(std::vector<uint8_t> vec_data, const std::vector<uint8_t>& key);
    std::vector<uint8_t> get_value_id(std::vector<uint8_t> vec_data, const std::vector<uint8_t>& key);
    std::vector<size_t> findAllStringPosition(const std::vector<uint8_t>& data, const std::string& query);
    size_t findStringPosition(const std::vector<uint8_t>& data, const std::string& query, size_t start);
    std::vector<std::string> extractPatterns(const std::string& input);
    std::string generateUUID2();
    std::vector<uint8_t> decodePNG(const char* filename, int& width, int& height, int& channels);
    std::string vectorToHexString(const std::vector<uint8_t>& vec);
    std::string base64_encode(const unsigned char* input, int length);
    std::vector<uint8_t> base64_decode(const std::string& base64Data);
    STACK_OF(X509)* vec_uint8_to_stack_of_x509(const std::vector<uint8_t>& vec);
    std::vector<std::vector<uint8_t> > stack_of_x509_to_vec_list(STACK_OF(X509)* stack);
    size_t get_offset(size_t pdf_awal_len, const std::vector<uint8_t>& pdf_akhir, const std::string& object_number);
    std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
    std::vector<uint8_t> hexStringToBytes(const std::string& hex);
    bool save_pdf(const std::string& filename, const std::vector<uint8_t>& pdf_data);
    bool has_pdf_extension(const std::string& filename);
    std::string constant(const char* type);
    std::string size_t_to_string(size_t value);
    std::string vector_size_t_to_string(const std::vector<size_t>& input);
    std::vector<unsigned char> saveQRCodeAsJPG(const std::string &text, const std::string &filename);
    std::vector<unsigned char> saveQRCodeAsJPG_2(const std::string &text);
    std::vector<unsigned char> saveQRCodeAsJPG_V2(const std::string& text, const std::string& image_path, const std::string& filename);
    std::string get_page(const std::vector<uint8_t>& pdf_content, const std::string& object_page_ref);
    bool containsAnnots(const std::vector<uint8_t>& pdfData);
    std::unordered_map<std::string, std::string> checkAnnotOnPage(std::string page_dict);
    std::string get_catalog(const std::vector<uint8_t>& pdf_content, const std::unordered_map<std::string, std::string>& read_pdf);
    size_t extract_integer(const std::string& input);
    std::vector<unsigned char> process_image(cv::Mat img);





    // Member function declaration
    void print_attribute_oid(const X509_ATTRIBUTE* attr);
    bool loadPKCS12(const std::string& pkcs12Path, const std::string& password, EVP_PKEY*& pkey, X509*& cert, STACK_OF(X509)*& ca);
    bool loadPKCS12_from_base64(const std::vector<uint8_t>& pkcs12_vec, const std::string& password, EVP_PKEY*& pkey, X509*& cert, STACK_OF(X509)*& ca);
    std::vector<unsigned char> readData(std::string filePath);
    std::string digest(std::vector<unsigned char> data);
    std::string vectorToStringHex_2(const std::vector<uint8_t>& vec);
    std::string asn1_string_to_hex_string(const ASN1_STRING *asn1_string);
    std::string get_digest_hex_from_signer_info(const CMS_SignerInfo* signer_info);
    void find_and_replace(std::string &str, const std::string &to_find, const std::string &replace_with);
    void handleErrors();
};

#endif