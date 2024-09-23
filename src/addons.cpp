#include "../header/addons.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <regex>
#include <uuid/uuid.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../header/stb_image.h"
#include <sstream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h> // Include this header for STACK_OF(X509_ATTRIBUTE)
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/pkcs12.h>
#include <iomanip>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <qrencode.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <zlib.h>

#define PIXEL_SIZE 10  // Scale of each QR code pixel block

Addons::Addons() {
    // Initialize private member variables or perform any necessary setup
}


// Function to search for a sequence of bytes within a vector
std::vector<uint8_t>::iterator findSequence(std::vector<uint8_t>& vec, const std::string& str, size_t start_position) {
    return std::search(vec.begin(), vec.end(), str.begin(), str.end());
}


std::vector<uint8_t> Addons::getSubVector(const std::vector<uint8_t>& vec, size_t x, size_t y) {
    if (x >= vec.size() || y >= vec.size() || x > y) {
        // Invalid indices
        return std::vector<uint8_t>(); // Return an empty vector
    }
    return std::vector<uint8_t>(vec.begin() + x, vec.begin() + y + 1);
}

std::string Addons::vectorToString(const std::vector<uint8_t>& vec) {
    return std::string(vec.begin(), vec.end());
}

std::vector<uint8_t> Addons::stringToVector(const std::string& str) {
    std::vector<uint8_t> vec;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        vec.push_back(static_cast<uint8_t>(*it));
    }
    return vec;
}

std::vector<uint8_t> Addons::get_value(std::vector<uint8_t> vec_data, const std::vector<uint8_t>& key) {
    
    std::vector<uint8_t>::iterator index_result = std::search(vec_data.begin(), vec_data.end(), key.begin(), key.end());
    if (index_result != vec_data.end()) {
        vec_data = std::vector<uint8_t>(index_result, vec_data.end());
        std::string pdf_stringxxx(vec_data.begin(), vec_data.end());
    }
    std::vector<uint8_t> query_slice;
    query_slice.push_back(' ');
    query_slice.push_back('R');
    std::vector<uint8_t>::iterator index = std::search(vec_data.begin(), vec_data.end(), query_slice.begin(), query_slice.end());
    if (index != vec_data.end()) {
        vec_data = std::vector<uint8_t>(vec_data.begin() + key.size() + 1, index + 2);
    }
    return vec_data;
}

std::vector<uint8_t> Addons::get_value_id(std::vector<uint8_t> vec_data, const std::vector<uint8_t>& key) {
    
    std::vector<uint8_t>::iterator index_result = std::search(vec_data.begin(), vec_data.end(), key.begin(), key.end());
    if (index_result != vec_data.end()) {
        vec_data = std::vector<uint8_t>(index_result, vec_data.end());
        std::string pdf_stringxxx(vec_data.begin(), vec_data.end());
    } else {
        return std::vector<uint8_t>();
    }
    std::vector<uint8_t> query_slice;
    query_slice.push_back('>');
    // query_slice.push_back(']');
    std::vector<uint8_t>::iterator index = std::search(vec_data.begin(), vec_data.end(), query_slice.begin(), query_slice.end());
    if (index != vec_data.end()) {
        vec_data = std::vector<uint8_t>(vec_data.begin() + key.size(), index);
    }
    return vec_data;
}

std::vector<size_t> Addons::findAllStringPosition(const std::vector<uint8_t>& data, const std::string& query) {
    std::vector<size_t> indices;
    size_t querySize = query.size();
    size_t dataSize = data.size();

    for (size_t i = 0; i <= dataSize - querySize; ++i) {
        if (std::equal(data.begin() + i, data.begin() + i + querySize, query.begin())) {
            indices.push_back(i);
        }
    }

    return indices;
}

size_t Addons::findStringPosition(const std::vector<uint8_t>& data, const std::string& query, size_t start) {
    size_t dataSize = data.size();
    size_t querySize = query.size();

    // Ensure start is within bounds
    if (start >= dataSize) {
        return std::string::npos;
    }

    for (size_t i = start; i <= dataSize - querySize; ++i) {
        if (std::equal(data.begin() + i, data.begin() + i + querySize, query.begin())) {
            return i; // Return the index where the query string is found
        }
    }

    return std::string::npos; // Return npos if not found
}


std::vector<std::string> Addons::extractPatterns(const std::string& input) {
    std::vector<std::string> result;
    std::regex pattern("\\d+ 0 R");
    std::smatch match;

    std::string::const_iterator it = input.begin();
    while (std::regex_search(it, input.end(), match, pattern)) {
        result.push_back(match[0]);
        it = match.suffix().first; // Move iterator to the end of the match
    }

    return result;
}


std::string Addons::generateUUID2() {
    uuid_t uuid;
    uuid_generate(uuid);

    char uuidStr[37]; // 36 characters + null terminator
    uuid_unparse(uuid, uuidStr);

    std::string uuidString(uuidStr);
    // Remove hyphens from the UUID string
    uuidString.erase(std::remove(uuidString.begin(), uuidString.end(), '-'), uuidString.end());

    return uuidString;
}

std::vector<uint8_t> Addons::decodePNG(const char* filename, int& width, int& height, int& channels) {
    std::vector<uint8_t> imageData;

    // Load the image with desired settings for better quality
    stbi_set_flip_vertically_on_load(true); // Flip the image vertically if needed
    unsigned char* data = stbi_load(filename, &width, &height, &channels, STBI_default);
    if (!data) {
        throw std::runtime_error("Failed to load image.");
    }

    // Determine the loaded image format
    int format = STBI_default;
    if (channels == STBI_rgb_alpha) {
        format = STBI_rgb_alpha;
    } else if (channels == STBI_rgb) {
        format = STBI_rgb;
    }

    // // Compress the image data using zlib
    // uLongf compressedSize = compressBound(width * height * channels);
    // imageData.resize(compressedSize);

    // int ret = compress(reinterpret_cast<Bytef*>(imageData.data()), &compressedSize,
    //                    reinterpret_cast<const Bytef*>(data), width * height * channels);
    // stbi_image_free(data);

    // if (ret != Z_OK) {
    //     throw std::runtime_error("Compression failed.");
    // }

    // // Resize vector to actual compressed size
    // imageData.resize(compressedSize);

    // return imageData;

    // Copy the pixel data into a vector
    imageData.assign(data, data + width * height * channels);

    // Free the loaded image data
    stbi_image_free(data);

    return imageData;
}


std::string Addons::base64_encode(const unsigned char* input, int length) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);
    BIO_write(b64, input, length);
    BIO_flush(b64);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);

    std::string base64_output(bufferPtr->data, bufferPtr->length); // Exclude null terminator

    BIO_free_all(b64);

    return base64_output;
}


std::vector<uint8_t> Addons::base64_decode(const std::string& base64Data) {
    BIO *bio, *b64;
    std::vector<uint8_t> decodedData;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf((void*)base64Data.c_str(), base64Data.length());
    bio = BIO_push(b64, bio);

    // Allocate enough memory for decoding
    decodedData.resize(base64Data.length());

    // Perform decoding
    int len = BIO_read(bio, decodedData.data(), decodedData.size());
    if (len > 0) {
        // Resize vector to actual decoded length
        decodedData.resize(len);
    } else {
        // Handle decoding error
        std::cerr << "Error decoding Base64 data" << std::endl;
        decodedData.clear();
    }

    // Free BIOs
    BIO_free_all(bio);

    return decodedData;
}

std::string Addons::vectorToHexString(const std::vector<uint8_t>& vec) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < vec.size(); ++i) {
        uint8_t byte = vec[i];
        ss << std::setw(2) << static_cast<int>(byte);
    }
    
    return ss.str();
}

// Function to convert Vec<uint8_t> to STACK_OF(X509)*
STACK_OF(X509)* Addons::vec_uint8_to_stack_of_x509(const std::vector<uint8_t>& vec) {
    STACK_OF(X509)* stack = sk_X509_new_null();
    if (!stack) {
        std::cerr << "Error creating new STACK_OF(X509)." << std::endl;
        return nullptr;
    }

    const unsigned char* p = vec.data();
    const unsigned char* end = p + vec.size();

    while (p < end) {
        X509* cert = d2i_X509(nullptr, &p, end - p);
        if (!cert) {
            std::cerr << "Error decoding X509 certificate: "
                      << ERR_reason_error_string(ERR_get_error()) << std::endl;
            sk_X509_pop_free(stack, X509_free);
            return nullptr;
        }

        if (!sk_X509_push(stack, cert)) {
            std::cerr << "Error adding X509 certificate to stack." << std::endl;
            X509_free(cert);
            sk_X509_pop_free(stack, X509_free);
            return nullptr;
        }
    }

    return stack;
}

// Function to convert STACK_OF(X509)* to a list of Vec<uint8_t>
std::vector<std::vector<uint8_t> > Addons::stack_of_x509_to_vec_list(STACK_OF(X509)* stack) {
    std::vector<std::vector<uint8_t> > result;

    for (int i = 0; i < sk_X509_num(stack); ++i) {
        X509* cert = sk_X509_value(stack, i);

        int len = i2d_X509(cert, nullptr);
        if (len < 0) {
            std::cerr << "Error converting X509 certificate to DER format: " 
                      << ERR_reason_error_string(ERR_get_error()) << std::endl;
            continue;
        }

        std::vector<uint8_t> der_cert(len);
        unsigned char* p = der_cert.data();
        len = i2d_X509(cert, &p);
        if (len < 0) {
            std::cerr << "Error converting X509 certificate to DER format: " 
                      << ERR_reason_error_string(ERR_get_error()) << std::endl;
            continue;
        }

        result.push_back(der_cert);
    }

    return result;
}

size_t Addons::get_offset(size_t pdf_awal_len, const std::vector<uint8_t>& pdf_akhir, const std::string& object_number) {
    Addons adns;
    std::string query = "\n" + object_number + " 0 obj";
    size_t offset_start = 0;
    size_t index = adns.findStringPosition(pdf_akhir, query, pdf_awal_len);
    if (index != std::string::npos) {
        offset_start = index;
    }
    
    return offset_start;
}

std::vector<uint8_t> Addons::sha256(const std::vector<uint8_t>& data) {
    // Create a context for the hash operation
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context) {
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    // Initialize the context for SHA-256
    if (1 != EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("EVP_DigestInit_ex failed");
    }

    // Update the context with the data
    if (1 != EVP_DigestUpdate(context, data.data(), data.size())) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("EVP_DigestUpdate failed");
    }

    // Finalize the hash and retrieve the result
    std::vector<uint8_t> hash(EVP_MD_size(EVP_sha256()));
    unsigned int length = 0;
    if (1 != EVP_DigestFinal_ex(context, hash.data(), &length)) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("EVP_DigestFinal_ex failed");
    }

    // Clean up the context
    EVP_MD_CTX_free(context);

    return hash;
}



std::vector<uint8_t> Addons::hexStringToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

// Print the OID of the X509_ATTRIBUTE
void Addons::print_attribute_oid(const X509_ATTRIBUTE* attr) {
    const ASN1_OBJECT* obj = X509_ATTRIBUTE_get0_object(const_cast<X509_ATTRIBUTE*>(attr));
    char buffer[128];
    OBJ_obj2txt(buffer, sizeof(buffer), obj, 1);
    std::cout << "OID: " << buffer << std::endl;
}

// Function to load PKCS#12 file and extract certificate, private key, and the chain of certificates
bool Addons::loadPKCS12(const std::string& pkcs12Path, const std::string& password, EVP_PKEY*& pkey, X509*& cert, STACK_OF(X509)*& ca) {
    FILE* fp = fopen(pkcs12Path.c_str(), "rb");
    if (!fp) {
        std::cerr << "Unable to open PKCS#12 file" << std::endl;
        return false;
    }

    PKCS12* p12 = d2i_PKCS12_fp(fp, nullptr);
    fclose(fp);

    if (!p12) {
        std::cerr << "Unable to parse PKCS#12 file" << std::endl;
        return false;
    }

    if (!PKCS12_parse(p12, password.c_str(), &pkey, &cert, &ca)) {
        std::cerr << "Unable to parse PKCS#12 structure" << std::endl;
        PKCS12_free(p12);
        return false;
    }

    PKCS12_free(p12);
    return true;
}

// Function to load PKCS#12 file and extract certificate, private key, and the chain of certificates
bool Addons::loadPKCS12_from_base64(const std::vector<uint8_t>& pkcs12_vec, const std::string& password, EVP_PKEY*& pkey, X509*& cert, STACK_OF(X509)*& ca) {
    std::vector<unsigned char> pkcs12_data(pkcs12_vec.begin(), pkcs12_vec.end());
    // Create a BIO for the decoded data
    BIO* bio = BIO_new_mem_buf(pkcs12_data.data(), pkcs12_data.size());
    if (!bio) {
        std::cerr << "Unable to create BIO" << std::endl;
        return false;
    }

    // Load PKCS#12 structure
    PKCS12* p12 = d2i_PKCS12_bio(bio, nullptr);
    BIO_free(bio);

    if (!p12) {
        std::cerr << "Unable to parse PKCS#12 file" << std::endl;
        return false;
    }

    if (!PKCS12_parse(p12, password.c_str(), &pkey, &cert, &ca)) {
        std::cerr << "Unable to parse PKCS#12 structure" << std::endl;
        PKCS12_free(p12);
        return false;
    }

    PKCS12_free(p12);
    return true;
}

void Addons::handleErrors() {
    // Print OpenSSL errors to stderr
    ERR_print_errors_fp(stderr);
    
    // Optionally, throw an exception to allow for graceful error handling
    throw std::runtime_error("An error occurred in OpenSSL. Check stderr for details.");
}

std::vector<unsigned char> Addons::readData(std::string filePath) {
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open data file.");
    }

    // Read the entire file into the vector
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    return data;
}

std::string Addons::digest(std::vector<unsigned char> data) {
    // Create a SHA256 hash
    unsigned char sha256_digest[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), sha256_digest);

    // Encode the hash as a base64 string
    std::string hash = base64_encode(sha256_digest, SHA256_DIGEST_LENGTH);

    return hash;
}

// Function to convert std::vector<uint8_t> to an octet string (hexadecimal representation)
std::string Addons::vectorToStringHex_2(const std::vector<uint8_t>& vec) {
    std::ostringstream oss;
    
    // Iterate over each byte in the vector
    for (auto byte : vec) {
        // Print each byte as two-digit hexadecimal, uppercase
        oss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        // Optionally add a space between bytes for readability
        // oss << ' ';
    }
    
    return oss.str();
}

// Function to convert ASN1_STRING to a hexadecimal std::string
std::string Addons::asn1_string_to_hex_string(const ASN1_STRING *asn1_string) {
    const unsigned char *data = ASN1_STRING_get0_data(asn1_string);
    int length = ASN1_STRING_length(asn1_string);

    std::ostringstream hex_stream;

    // Convert each byte to a two-digit hexadecimal value
    for (int i = 0; i < length; i++) {
        hex_stream << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }

    return hex_stream.str();
}

// Function to convert ASN1_STRING to a hexadecimal std::string
std::string Addons::get_digest_hex_from_signer_info(const CMS_SignerInfo* signer_info) {
    // Define the NID for messageDigest attribute
    int nid_message_digest = NID_pkcs9_messageDigest;
    // Get the count of signed attributes
    int attr_count = CMS_signed_get_attr_count(signer_info);
    if (attr_count < 0) {
        fprintf(stderr, "Error getting attribute count\n");
        return "";
    }
    // Try to find the digest attribute by NID
    int attr_pos = CMS_signed_get_attr_by_NID(signer_info, nid_message_digest, -1);
    if (attr_pos < 0) {
        fprintf(stderr, "Digest attribute not found\n");
        return "";
    }
    // Get the attribute at the found position
    X509_ATTRIBUTE *attr = CMS_signed_get_attr(signer_info, attr_pos);
    if (!attr) {
        fprintf(stderr, "Error getting attribute\n");
        return "";
    }
    // Extract and print the digest data
    const ASN1_TYPE *attr_value = X509_ATTRIBUTE_get0_type(attr, 0);
    if (attr_value) {
        if (attr_value->type == V_ASN1_OCTET_STRING) {
            ASN1_STRING *datal = attr_value->value.asn1_string;
            const unsigned char *datan = ASN1_STRING_get0_data(datal);
            int length = ASN1_STRING_length(datal);
            std::ostringstream hex_stream;
            for (int i = 0; i < length; i++) {
                hex_stream << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << static_cast<int>(datan[i]);
            }
            return hex_stream.str();
        } else {
            fprintf(stderr, "Attribute value is not an ASN1_OCTET_STRING\n");
        return "";
        }
    } else {
        fprintf(stderr, "Error extracting attribute value\n");
        return "";
    }
}


void Addons::find_and_replace(std::string &str, const std::string &to_find, const std::string &replace_with) {
    std::string::size_type pos = 0;

    // Loop to find and replace all occurrences
    while ((pos = str.find(to_find, pos)) != std::string::npos) {
        str.replace(pos, to_find.length(), replace_with);
        pos += replace_with.length(); // Move past the replaced part
    }
}

// Function to save a std::vector<uint8_t> to a PDF file
bool Addons::save_pdf(const std::string& filename, const std::vector<uint8_t>& pdf_data) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    
    // Check if the file opened successfully
    if (!file) {
        std::cerr << "Error: Could not open file '" << filename << "' for writing.\n";
        return false;
    }

    // Write the binary data from the vector to the file
    file.write(reinterpret_cast<const char*>(pdf_data.data()), pdf_data.size());

    // Check if the writing was successful
    if (!file) {
        std::cerr << "Error: Writing to file '" << filename << "' failed.\n";
        return false;
    }

    file.close();
    return true;
}

bool Addons::has_pdf_extension(const std::string& filename) {
    // Find the last dot in the filename
    size_t dot_position = filename.rfind('.');
    if (dot_position == std::string::npos) {
        return false;  // No dot found, no extension present
    }

    // Get the extension and compare it to ".pdf"
    std::string extension = filename.substr(dot_position);
    return extension == ".pdf";
}

std::string Addons::constant(const char* type) {
    std::string p12 = "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";
    std::string pdf = "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KL3RbNgzQdJbE3EVtOqlCSN/naDRWRLj4nmUpdYIN+7N2mKJirLsz5IMtuS4+ERLyAQpNkMyWpPj45IidFLmBkvUMO7wMRO0G23xFsPI0eTxqj5gMBm0WrxJC0fO8Um81CjNSXbKydJMFIeEGEz6kafO9Y6mkaP4pRZrUDz6laZdcwN+ZcpgM9Q1iIxmyWHWBCnhsGWnQ5bkw7pduIquxwtn1oln+WSHWxIOd1l26uTDXShUr5ViIvkNdOwiaRi73IlTQNM+3rO6qrqzzJFSsbp6/mr3uCusKUVpckGKNUouHJ+Z7zBLs+bu8E/OXbS9tnpbY0HR4q3VxYumJiUW1xdXLC6MSZ5Wz6TRmtEPtGt1hfjGuJiP9ziLwl+sWazSnCj+p2smaW4U/xu2R6zSXKNVh7udlOfKc7kw7EGEuMdhpAa+xOm4m0l8GWNxuxuvi4hOjI61m7RG7aW6iBh7TIw9XGs0hoYatEZzVLgu1GjSaw0R0abRUVY1+oGuGyPIwOchK345g2pN+CxkYAb8bE8PiUXiXswfenyTzmbyR6WrvLlrZUvPyv8HylKRiAplbmRzdHJlYW0KZW5kb2JqCjE4IDAgb2JqCjw8IC9Qcm9kdWNlciAobWFjT1MgVmVyc2lvbiAxNC40IFwoQnVpbGQgMjNFMjE0XCkgUXVhcnR6IFBERkNvbnRleHQpIC9DcmVhdGlvbkRhdGUKKEQ6MjAyNDA5MTYwNzA5MjlaMDAnMDAnKSAvTW9kRGF0ZSAoRDoyMDI0MDkxNjA3MDkyOVowMCcwMCcpID4+CmVuZG9iagp4cmVmCjAgMTkKMDAwMDAwMDAwMCA2NTUzNSBmIAowMDAwMDAxMDQyIDAwMDAwIG4gCjAwMDAwMDQwMDMgMDAwMDAgbiAKMDAwMDAwMDAyMiAwMDAwMCBuIAowMDAwMDAxMTQ2IDAwMDAwIG4gCjAwMDAwMDM5NjcgMDAwMDAgbiAKMDAwMDAwMDAwMCAwMDAwMCBuIAowMDAwMDA0MTM2IDAwMDAwIG4gCjAwMDAwMDAwMDAgMDAwMDAgbiAKMDAwMDAxMjgzNyAwMDAwMCBuIAowMDAwMDAxMjU0IDAwMDAwIG4gCjAwMDAwMDQwODYgMDAwMDAgbiAKMDAwMDAwNDc0NSAwMDAwMCBuIAowMDAwMDA0MzU5IDAwMDAwIG4gCjAwMDAwMDQ5ODYgMDAwMDAgbiAKMDAwMDAxMzM1MiAwMDAwMCBuIAowMDAwMDEzMDE5IDAwMDAwIG4gCjAwMDAwMTM1ODggMDAwMDAgbiAKMDAwMDAyMjA0OCAwMDAwMCBuIAp0cmFpbGVyCjw8IC9TaXplIDE5IC9Sb290IDExIDAgUiAvSW5mbyAxOCAwIFIgL0lEIFsgPGM3OGUwMDY1MmU5MTNhZmRiZDNmZGFjZTE0ODI4OTljPgo8Yzc4ZTAwNjUyZTkxM2FmZGJkM2ZkYWNlMTQ4Mjg5OWM+IF0gPj4Kc3RhcnR4cmVmCjIyMjExCiUlRU9GCg==";
    std::string image = "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";

    if (std::string(type) == "p12") {
        return p12;
    } else if (std::string(type) == "image") {
        return image;
    } else if (std::string(type) == "pdf") {
        return pdf;
    } else {
        return "Unknown file type.";
    }
}

// Convert a single size_t value to its string representation
std::string Addons::size_t_to_string(size_t value) {
    return std::to_string(value);
}

// Convert a vector of size_t values to a string
std::string Addons::vector_size_t_to_string(const std::vector<size_t>& input) {
    std::string output;
    for (size_t i = 0; i < input.size(); ++i) {
        output += size_t_to_string(input[i]);
        if (i < input.size() - 1) {
            output += " "; // Use space as delimiter for all except the last value
        }
    }
    return output;
}


std::vector<unsigned char> Addons::saveQRCodeAsJPG(const std::string &text, const std::string &filename) {
    // Create the QR code
    QRcode *qr = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    if (qr == nullptr) {
        std::cerr << "Failed to generate QR code" << std::endl;
        return {};
    }

    // Set the image size (QR code size * pixel scale)
    int imageSize = qr->width * PIXEL_SIZE;
    cv::Mat qrImage = cv::Mat::zeros(imageSize, imageSize, CV_8UC3);

    // Fill the image with the QR code data
    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            cv::Rect rect(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE);
            if (qr->data[y * qr->width + x] & 1) {
                cv::rectangle(qrImage, rect, cv::Scalar(0, 0, 0), cv::FILLED);  // Black for QR code modules
            } else {
                cv::rectangle(qrImage, rect, cv::Scalar(255, 255, 255), cv::FILLED);  // White background
            }
        }
    }

    // Save the image as JPG
    if (cv::imwrite(filename, qrImage)) {
        // std::cout << "QR code saved as " << filename << std::endl;
    } else {
        std::cerr << "Failed to save the QR code as JPG" << std::endl;
    }

    // Encode the image to a vector in memory (JPG format)
    std::vector<unsigned char> buffer;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 90};  // JPEG quality (90%)
    
    if (!cv::imencode(".jpg", qrImage, buffer, compression_params)) {
        std::cerr << "Failed to encode the QR code as JPG" << std::endl;
        return {};
    }

    // Clean up QR code
    QRcode_free(qr);

    // Return the encoded JPG data as a vector
    return buffer;
}

std::vector<unsigned char> Addons::saveQRCodeAsJPG_V2(const std::string& text, const std::string& image_path, const std::string& filename) {
    // Generate the main QR code
    QRcode* qr = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    if (!qr) {
        std::cerr << "Failed to generate QR code." << std::endl;
        return {};
    }

    // Create an OpenCV matrix for the main QR code
    int qr_size = qr->width * PIXEL_SIZE;
    cv::Mat qr_image(qr_size, qr_size, CV_8UC1, cv::Scalar(255)); // White background

    // Draw the QR code matrix onto the image
    int scale = qr_size / qr->width;
    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            cv::Rect rect(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE);
            if (qr->data[y * qr->width + x] & 1) {
                cv::rectangle(qr_image, rect, cv::Scalar(0, 0, 0), cv::FILLED);  // Black for QR code modules
            } else {
                cv::rectangle(qr_image, rect, cv::Scalar(255, 255, 255), cv::FILLED);  // White background
            }
        }
    }

    // Load the image to be placed on top of the QR code
    cv::Mat overlay_image = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    if (overlay_image.empty()) {
        std::cerr << "Failed to load overlay image." << std::endl;
        return {};
    }

    // Resize the overlay image to be smaller than the small QR code
    // cv::resize(overlay_image, overlay_image, cv::Size(small_qr_size - 20, small_qr_size - 20)); // Make it smaller
    cv::resize(overlay_image, overlay_image, cv::Size(qr_size/5, qr_size/5)); // Make it smaller

    // Ensure the overlay has 3 channels (BGR)
    if (overlay_image.channels() == 1) {
        cv::cvtColor(overlay_image, overlay_image, cv::COLOR_GRAY2BGR);
    } else if (overlay_image.channels() == 4) {
        cv::cvtColor(overlay_image, overlay_image, cv::COLOR_BGRA2BGR);
    }

    // Create a white background for the overlay
    cv::Mat overlay_background(overlay_image.rows, overlay_image.cols, CV_8UC3, cv::Scalar(255, 255, 255));

    // Blend the overlay image onto the white background
    cv::Mat blended_overlay;
    cv::addWeighted(overlay_background, 1.0, overlay_image, 1.0, 0.0, blended_overlay);

    // Calculate the position to place the blended overlay in the center
    int center_x = (qr_size - blended_overlay.cols) / 2;
    int center_y = (qr_size - blended_overlay.rows) / 2;

    // Ensure the QR image has 3 channels for copying
    if (qr_image.channels() == 1) {
        cv::cvtColor(qr_image, qr_image, cv::COLOR_GRAY2BGR);
    }

    // Place the blended overlay on the QR code
    blended_overlay.copyTo(qr_image(cv::Rect(center_x, center_y, blended_overlay.cols, blended_overlay.rows)));

    // Generate a smaller QR code to place in the top left
    QRcode* small_qr = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    if (!small_qr) {
        std::cerr << "Failed to generate small QR code." << std::endl;
        return {};
    }

    // Create an OpenCV matrix for the small QR code
    cv::Mat small_qr_image((qr_size/3)+(qr_size/12), (qr_size/3)+(qr_size/12), CV_8UC1, cv::Scalar(255)); // White background

    // Draw the small QR code matrix onto the image
    scale = ((qr_size/3)+(qr_size/12)) / small_qr->width;
    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            cv::Rect rect(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE);
            if (qr->data[y * qr->width + x] & 1) {
                cv::rectangle(small_qr_image, cv::Point(x * scale, y * scale), cv::Point((x + 1) * scale, (y + 1) * scale), cv::Scalar(0, 0, 0), cv::FILLED);  // Black for QR code modules
            } else {
                cv::rectangle(small_qr_image, cv::Point(x * scale, y * scale), cv::Point((x + 1) * scale, (y + 1) * scale), cv::Scalar(255, 255, 255), cv::FILLED);  // White background
            }
        }
    }

    // Ensure the small QR image has 3 channels
    if (small_qr_image.channels() == 1) {
        cv::cvtColor(small_qr_image, small_qr_image, cv::COLOR_GRAY2BGR);
    }

    // Place the small QR code in the top left corner
    small_qr_image.copyTo(qr_image(cv::Rect(0, 0, small_qr_image.cols, small_qr_image.rows)));

    // Step 4: Load the logo image and resize it
    int logo_size = (qr_size/5);
    cv::Mat logo = cv::imread(image_path, cv::IMREAD_UNCHANGED);
    if (logo.empty()) {
        std::cerr << "Failed to load logo image" << std::endl;
        QRcode_free(qr);
        return {};
    }
    cv::resize(logo, logo, cv::Size(logo_size, logo_size));

    // If the logo has an alpha channel (transparency), blend it with the QR code
    if (logo.channels() == 4) {
        for (int y = 0; y < logo_size; ++y) {
            for (int x = 0; x < logo_size; ++x) {
                cv::Vec4b pixel = logo.at<cv::Vec4b>(y, x);
                if (pixel[3] > 0) {  // Check for transparency (alpha > 0)
                    // Blend logo pixel with QR code pixel
                    cv::Vec3b& qr_pixel = qr_image.at<cv::Vec3b>(center_y + y, center_x + x);
                    qr_pixel[0] = (qr_pixel[0] * (255 - pixel[3]) + pixel[0] * pixel[3]) / 255;
                    qr_pixel[1] = (qr_pixel[1] * (255 - pixel[3]) + pixel[1] * pixel[3]) / 255;
                    qr_pixel[2] = (qr_pixel[2] * (255 - pixel[3]) + pixel[2] * pixel[3]) / 255;
                }
            }
        }
    } else {
        // If no transparency, just place the logo
        logo.copyTo(qr_image(cv::Rect(center_x, center_y, logo_size, logo_size)));
    }

    // Save the final image to a file

    // Save the image as JPG
    if (cv::imwrite(filename, qr_image)) {
        // std::cout << "QR code saved as " << filename << std::endl;
    } else {
        std::cerr << "Failed to save the QR code as JPG" << std::endl;
    }

    // Encode the image to a vector in memory (JPG format)
    std::vector<unsigned char> buffer;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 90};  // JPEG quality (90%)
    
    if (!cv::imencode(".jpg", qr_image, buffer, compression_params)) {
        std::cerr << "Failed to encode the QR code as JPG" << std::endl;
        return {};
    }

    // Free the QR codes
    QRcode_free(qr);
    QRcode_free(small_qr);

    return buffer;
}

std::vector<unsigned char> Addons::saveQRCodeAsJPG_2(const std::string &text) {
    // Create the QR code
    QRcode *qr = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
    if (qr == nullptr) {
        std::cerr << "Failed to generate QR code" << std::endl;
        return {};
    }

    // Set the image size (QR code size * pixel scale)
    int imageSize = qr->width * PIXEL_SIZE;
    cv::Mat qrImage = cv::Mat::zeros(imageSize, imageSize, CV_8UC3);


    // Fill the image with the QR code data
    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            cv::Rect rect(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE);
            if (qr->data[y * qr->width + x] & 1) {
                cv::rectangle(qrImage, rect, cv::Scalar(0, 0, 0), cv::FILLED);  // Black for QR code modules
            } else {
                cv::rectangle(qrImage, rect, cv::Scalar(255, 255, 255), cv::FILLED);  // White background
            }
        }
    }

    // Encode the image to a vector in memory (JPG format)
    std::vector<unsigned char> buffer;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 90};  // JPEG quality (90%)
    
    if (!cv::imencode(".jpg", qrImage, buffer, compression_params)) {
        std::cerr << "Failed to encode the QR code as JPG" << std::endl;
        return {};
    }

    // Clean up QR code
    QRcode_free(qr);

    // Return the encoded JPG data as a vector
    return buffer;
}


std::string Addons::get_page(const std::vector<uint8_t>& pdf_content, const std::string& object_page_ref) {
    
    Addons adns;

    std::string raw_string_doc(pdf_content.begin(), pdf_content.end());
    std::string query = "\n" + object_page_ref.substr(0, object_page_ref.length() - 2) + " obj";
    std::vector<uint8_t> query_offset(query.begin(), query.end());


    std::vector<size_t> positions = adns.findAllStringPosition(pdf_content, query);


    size_t endobjPosition = 0;
    if (size_t index = adns.findStringPosition(pdf_content, "endobj", positions.back())) {
        endobjPosition = index;
    }

    std::vector<uint8_t> binding(pdf_content.begin() + positions.back(), pdf_content.begin() + endobjPosition);
    std::string raw_string_result(binding.begin(), binding.end());
    // std::cout << "Page Result 2: " << raw_string_result << std::endl;
    return raw_string_result;
}



bool Addons::containsAnnots(const std::vector<uint8_t>& pdfData) {
    std::string annotsString = "/Annots";
    size_t annotsPos = 0;

    // Search for "/Annots" in the PDF data
    for (size_t i = 0; i < pdfData.size(); ++i) {
        if (pdfData[i] == annotsString[annotsPos]) {
            ++annotsPos;
            if (annotsPos == annotsString.size()) {
                return true; // Found "/Annots" in the PDF data
            }
        } else {
            annotsPos = 0; // Reset position if mismatch
        }
    }

    return false; // "/Annots" not found in the PDF data
}

std::unordered_map<std::string, std::string> Addons::checkAnnotOnPage(std::string page_dict) {

    Addons adns;

    std::unordered_map<std::string, std::string> result;
    std::string page_before;
    std::string page_after;

    std::vector<uint8_t> page_vec(page_dict.begin(), page_dict.end());

    // Check if the PDF data contains "/Annots"
    if (containsAnnots(page_vec)) {
        size_t annotPosition = 0;
        if (size_t index_1 = adns.findStringPosition(page_vec, "/Annots", 0)) {
            if (size_t index_2 = adns.findStringPosition(page_vec, "]", index_1)) {
                annotPosition = index_2;
                std::vector<uint8_t> page_before_vec(page_vec.begin(), page_vec.begin() + index_2);
                std::string page_beforex(page_before_vec.begin(), page_before_vec.end());
                page_before = page_beforex;

                std::vector<uint8_t> page_after_vec(page_vec.begin() + index_2, page_vec.end());
                std::string page_afterx(page_after_vec.begin(), page_after_vec.end());
                page_after = page_afterx;
            }
        }
    } else {
        result["status"] = "null";
        result["page_before"] = "null";
        result["page_after"] = "null";
        return result;
    }

    
    result["status"] = "exist";
    result["page_before"] = page_before;
    result["page_after"] = page_after;

    // std::unordered_map<std::string, std::string>::iterator it;
    // for (it = result.begin(); it != result.end(); ++it) {
    //     std::cout << "Key ReadPdf: " << it->first << ", Value: " << it->second << std::endl;
    // }

    return result;

}

std::string Addons::get_catalog(const std::vector<uint8_t>& pdf_content, const std::unordered_map<std::string, std::string>& read_pdf) {
    std::vector<uint8_t> pdf = pdf_content;
    std::string raw_string_doc(pdf_content.begin(), pdf_content.end());
    std::string rootRef = read_pdf.at("rootRef");
    std::string query = "\n" + rootRef.substr(0, rootRef.length() - 2) + " obj";
    std::vector<uint8_t> query_offset(query.begin(), query.end());
    size_t offset_start = 0;
    std::vector<uint8_t>::iterator it = std::search(pdf.begin(), pdf.end(), query_offset.begin(), query_offset.end());
    if (it != pdf.end()) {
        offset_start = std::distance(pdf.begin(), it);
    }
    std::string query_offset2 = "endobj";
    std::vector<uint8_t> query_offset_vec2(query_offset2.begin(), query_offset2.end());
    size_t offset_start2 = 0;
    it = std::search(pdf.begin() + offset_start, pdf.end(), query_offset_vec2.begin(), query_offset_vec2.end());
    if (it != pdf.end()) {
        offset_start2 = std::distance(pdf.begin(), it);
    }
    std::vector<uint8_t> binding(pdf.begin() + offset_start, pdf.begin() + offset_start2);
    std::string raw_string_result(binding.begin(), binding.end());
    return raw_string_result;
}

size_t Addons::extract_integer(const std::string& input) {
    // Find the position of the first non-whitespace character
    size_t start_pos = input.find_first_not_of(" \t\n\v\f\r");
    if (start_pos == std::string::npos) {
        // No non-whitespace character found
        throw std::invalid_argument("No integer found in the input string.");
    }

    // Find the position of the first non-digit character after the start position
    size_t pos = start_pos;
    while (pos < input.size() && std::isdigit(input[pos])) {
        ++pos;
    }

    // Convert the substring containing the integer part to size_t
    return std::stoul(input.substr(start_pos, pos - start_pos));
}


// Function to process the image
std::vector<unsigned char> Addons::process_image(cv::Mat img) {
    // Check if image has transparency (alpha channel)
    if (img.channels() == 4) {
        // Split the RGBA channels
        std::vector<cv::Mat> rgba_channels(4);
        cv::split(img, rgba_channels);

        // Extract the alpha channel
        cv::Mat alpha_channel = rgba_channels[3];

        // Create a new image with a white background (RGB only)
        cv::Mat white_bg(img.size(), CV_8UC3, cv::Scalar(255, 255, 255));

        // Convert the original RGBA image to RGB
        cv::Mat img_rgb;
        cv::cvtColor(img, img_rgb, cv::COLOR_BGRA2BGR);

        // Use the alpha channel as a mask to composite the image over the white background
        img_rgb.copyTo(white_bg, alpha_channel);

        // Set the result to the composited image
        img = white_bg;
    } else if (img.channels() == 3) {
        // Convert image to RGB (if it's not in RGB already)
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
    }

    // Rotate the image by 180 degrees (optional)
    // cv::rotate(img, img, cv::ROTATE_180);

    // Encode the image to JPEG in memory
    std::vector<unsigned char> img_encoded;
    std::vector<int> jpeg_params = {cv::IMWRITE_JPEG_QUALITY, 90};  // JPEG quality
    cv::imencode(".jpg", img, img_encoded, jpeg_params);

    // Encode the JPEG data to Base64
    // std::string encoded_string = base64_encode(img_encoded);

    return reinterpret_cast<std::vector<uint8_t>&>(img_encoded);
}