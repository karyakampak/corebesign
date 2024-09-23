#include "../header/sign_bsre.h"
#include "../header/addons.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <openssl/core.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/ocsp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#include <openssl/err.h>
#include <openssl/cms.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h> // Include this header for STACK_OF(X509_ATTRIBUTE)
#include <sstream>
#include <chrono>

using json = nlohmann::json;

SignBSrE::SignBSrE() {
    // Initialize private member variables or perform any necessary setup
}void handle_openssl_error() {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error("OpenSSL error");
}

void extract_tsa_from_cms_hex(const std::string& signature_hex) {
    // Decode hex string into binary
    std::vector<unsigned char> signature_data;
    signature_data.reserve(signature_hex.size() / 2);

    for (size_t i = 0; i < signature_hex.size(); i += 2) {
        unsigned char byte = std::stoi(signature_hex.substr(i, 2), nullptr, 16);
        signature_data.push_back(byte);
    }

    // Initialize OpenSSL library
    OPENSSL_init_crypto(0, NULL);

    // Decode CMS structure from binary data
    BIO* cms_bio = BIO_new_mem_buf(signature_data.data(), signature_data.size());
    if (!cms_bio) {
        handle_openssl_error();
    }

    CMS_ContentInfo* cms = d2i_CMS_bio(cms_bio, NULL);
    if (!cms) {
        handle_openssl_error();
    }

    // Extract signer certificates
    STACK_OF(X509)* certs = CMS_get0_signers(cms);
    if (!certs) {
        std::cerr << "Error: No signer information found in CMS." << std::endl;
        CMS_ContentInfo_free(cms);
        BIO_free(cms_bio);
        return;
    }

    // Extract TSA information
    ASN1_OBJECT* obj = OBJ_txt2obj("1.2.840.113549.1.9.16.2.14", 1); // id_aa_signatureTimeStampToken
    if (!obj) {
        handle_openssl_error();
    }

    STACK_OF(CMS_SignerInfo)* signers = CMS_get0_SignerInfos(cms);




    // for (int i = 0; i < signer_count; ++i) {
    //     CMS_SignerInfo* signer_info = sk_CMS_SignerInfo_value(signers, i);

    //     // Extract signer's certificate
    //     STACK_OF(X509)* certs = CMS_get0_signers(cms);
    //     if (!certs) {
    //         std::cerr << "Error: Failed to extract signer's certificate." << std::endl;
    //         continue;
    //     }
    //     X509* signer_cert = sk_X509_value(certs, i);

    //     // Get digest algorithm used by signer
    //     // const ASN1_OBJECT* digest_oid = CMS_signed_get0_digest_algorithm(signer_info);
    //     // char* digest_name = OBJ_nid2sn(OBJ_obj2nid(digest_oid));

    //     // std::cout << "Signer " << i + 1 << ":" << std::endl;
    //     // std::cout << " - Digest Algorithm: " << digest_name << std::endl;

    //     // Other information can be extracted similarly
    //     // For example, get signing time, signer's serial number, etc.

    //     // Clean up
    //     X509_free(signer_cert);
    // }

    int signer_count = sk_CMS_SignerInfo_num(signers);
    std::cout << "Number of signers: " << signer_count << std::endl;
    CMS_SignerInfo* signer_info = sk_CMS_SignerInfo_value(signers, 0);

    // CMS_SignerInfo* signer_info = CMS_get0_signer_info(cms, certs, NULL, 0);
    if (!signer_info) {
        std::cerr << "Error: Failed to retrieve signer information." << std::endl;
        sk_X509_pop_free(certs, X509_free);
        ASN1_OBJECT_free(obj);
        CMS_ContentInfo_free(cms);
        BIO_free(cms_bio);
        return;
    }

    // Get signed attributes (includes TSA information)
    // const STACK_OF(X509_ATTRIBUTE)* signed_attrs = CMS_signed_get0_attr(signer_info);
    // if (!signed_attrs) {
    //     std::cerr << "Error: No signed attributes found." << std::endl;
    //     sk_X509_pop_free(certs, X509_free);
    //     ASN1_OBJECT_free(obj);
    //     CMS_ContentInfo_free(cms);
    //     BIO_free(cms_bio);
    //     return;
    // }

    // // Find and print TSA information
    // for (int i = 0; i < sk_X509_ATTRIBUTE_num(signed_attrs); ++i) {
    //     const X509_ATTRIBUTE* attr = sk_X509_ATTRIBUTE_value(signed_attrs, i);
    //     const ASN1_OBJECT* attr_obj = X509_ATTRIBUTE_get0_object(attr);
    //     if (OBJ_cmp(attr_obj, obj) == 0) {
    //         const ASN1_TYPE* attr_value = X509_ATTRIBUTE_get0_type(attr, 0);
    //         if (attr_value->type == V_ASN1_OCTET_STRING) {
    //             unsigned char* data = ASN1_STRING_get0_data(attr_value->value.octet_string);
    //             int length = ASN1_STRING_length(attr_value->value.octet_string);

    //             std::cout << "TSA Information:" << std::endl;
    //             std::cout << "----------------" << std::endl;
    //             std::cout << std::string(reinterpret_cast<char*>(data), length) << std::endl;
    //             break; // Found TSA information, exit loop
    //         }
    //     }
    // }

    // Clean up
    sk_X509_pop_free(certs, X509_free);
    ASN1_OBJECT_free(obj);
    CMS_ContentInfo_free(cms);
    BIO_free(cms_bio);
}


// Convert PKCS#7 binary data to PKCS#7 object
PKCS7* parsePKCS7(const std::vector<uint8_t>& pkcs7Data) {
    const unsigned char *p = pkcs7Data.data();
    return d2i_PKCS7(NULL, &p, pkcs7Data.size());
}

// Function to convert STACK_OF(X509) to Vec<uint8_t>
std::vector<uint8_t> stack_of_x509_to_vec_uint8(STACK_OF(X509)* stack) {
    std::vector<uint8_t> result;

    // Iterate over each X509 certificate in the stack
    for (int i = 0; i < sk_X509_num(stack); ++i) {
        X509* cert = sk_X509_value(stack, i);

        // Convert the X509 certificate to DER format
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

        // Append the DER data to the result vector
        result.insert(result.end(), der_cert.begin(), der_cert.end());
    }

    return result;
}


// Function to extract the certificate chain from a PKCS7 object
std::vector<uint8_t> extractCertificateChain(PKCS7* pkcs7) {

    if (pkcs7->d.sign != nullptr && pkcs7->d.sign->cert != nullptr) {
        // Iterate through the certificates in the certificate stack
        STACK_OF(X509)* certs = pkcs7->d.sign->cert;

        // X509* cert = sk_X509_value(certs, 1);
        // std::cout << "Certificate " << 1 + 1 << ":" << std::endl;
        // X509_NAME_print_ex_fp(stdout, X509_get_subject_name(cert), 0, XN_FLAG_MULTILINE);
        // std::cout << std::endl;

        std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
        std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
        std::time_t timestamp_time_t = formatted_timestamp.count();
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
        std::string ocsp_name = ss.str() + ".der";

        const char* ocsp_der_file = ocsp_name.c_str();

        return stack_of_x509_to_vec_uint8(certs);
        
    } else {
        std::cerr << "No certificate chain found in PKCS7" << std::endl;
        return std::vector<uint8_t>();
    }

}

// Callback function to receive response
size_t writeCallbackToken(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t writeCallbackSign(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string get_token(const char* id, const char* secret) {
    // Initialize cURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL." << std::endl;
        return std::string("");
    }

    // Set the URL for the API endpoint
    std::string url = "https://api-bsre.bssn.go.id/oauth2/token";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set Basic authentication credentials
    std::string username = id;
    std::string passphrase = secret;
    std::string auth = username + ":" + passphrase;
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, auth.c_str());

    // Set the request method to POST
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Set request headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set the request body
    std::string bodyData = "{\"grant_type\": \"client_credentials\"}";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyData.c_str());

    // Set the write callback function to receive response
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackToken);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    std::string token;
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        token = std::string("Failed to perform request");
        std::cerr << "Failed to perform request: " << curl_easy_strerror(res) << std::endl;
    } else {
        // Parse JSON string
        try {

            json j = json::parse(response);

            // Get data with key
            std::string tkn = j["access_token"];
            token = tkn;

            // Print extracted data
            // std::cout << "Token : " << token << std::endl;
        } catch (const std::exception& e) {
            token = std::string("Error parsing JSON");
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
        // std::cout << "Response: " << response << std::endl;
    }

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return token;
}

void saveToken(const std::string& token) {
    json data;
    data["token"] = token;

    std::ofstream file("bsre.json");
    if (file.is_open()) {
        file << data.dump(4); // Pretty print with 4 spaces
        file.close();
    } else {
        std::cerr << "Failed to open config file for writing\n";
    }
}

std::string loadToken() {
    std::ifstream file("bsre.json");
    if (file.is_open()) {
        json data;
        file >> data;
        file.close();
        // std::cerr << "Token Error x : " << data["token"];
        if (data.contains("token")) {
            return data["token"];
        }
    } else {
        std::cerr << "Failed to open config file for reading\n";
    }
    return ""; // Return empty string if token not found
}

std::unordered_map<std::string, std::vector<uint8_t> > get_signature(std::string bsHash, const char* nik, const char* passphrase, const char* id, const char* secret) {

    Addons adns;

    std::string token = loadToken();
    if (!token.empty()) {
    } else {
        std::cerr << "Token is invalid or expired.\nTry to request new token.\n";
        std::string tokenstring = get_token(id, secret);
        saveToken(tokenstring);
        token = tokenstring;
    }

    std::string signature;
    std::vector<uint8_t> pkcs7Binary;

    CURL *curl;
    CURLcode res;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    std::string bearerToken = std::string("Authorization: Bearer ") + token;

    std::vector<uint8_t> OCSPResp;
    std::vector<uint8_t> stack_cert;

    if (curl) {
        // Set the URL for the request
        curl_easy_setopt(curl, CURLOPT_URL, "https://api-bsre.bssn.go.id/v2/esign/v2/api/entity/sign/hash/pdf");
        

        // Set Bearer token
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, bearerToken.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Create the JSON object
        json body = {
            {"nik", nik},
            {"idSubscriber", nullptr},
            {"passphrase", passphrase},
            {"signaturePropertiesHashMap", {
                {"0", {
                    {"imageBase64", "null"},
                    {"tampilan", "INVISIBLE"},
                    {"fieldId", "null"},
                    {"page", 0},
                    {"originX", 0.0},
                    {"originY", 0.0},
                    {"width", 0.0},
                    {"height", 0.0},
                    {"signatureLevel", "PAdES_BASELINE_LT"},
                    {"certificationPermission", "CHANGES_PERMITTED"},
                    {"signingDate", "2022-06-24T12:12:23.649+0000"},
                    {"certificateChainBase64", nullptr},
                    {"location", "null"},
                    {"reason", "null"},
                    {"contactInfo", "null"},
                    {"documentDigestBase64", bsHash}
                }}
            }}
        };

        // Convert hashmap data to string format
        std::string jsonString = body.dump();

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

        // Set callback function to receive response data
        std::string responseData;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackSign);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            signature = std::string("Failed to perform request");
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
            get_signature(bsHash, nik, passphrase, id, secret);
        } else {
            try {

                // Parse JSON string
                json j = json::parse(responseData);
                // std::cout << responseData << std::endl;
                auto cmsSignedDataHashMap = j["data"]["cmsSignedDataHashMap"];

                if (cmsSignedDataHashMap.contains("0") && !cmsSignedDataHashMap["0"].is_null()) {

                    // Get data with key
                    std::string sgn = cmsSignedDataHashMap["0"];
                    signature = sgn;

                    // Initialize OpenSSL library
                    OpenSSL_add_all_algorithms();
                    ERR_load_crypto_strings();

                    pkcs7Binary = adns.base64_decode(signature);

                    // Parse PKCS#7 binary data into PKCS#7 object
                    PKCS7 *pkcs7Object = parsePKCS7(pkcs7Binary);
                    if (!pkcs7Object) {
                        std::cerr << "Error parsing PKCS#7 data" << std::endl;
                    }

                    // PKCS#7 object is now ready to use
                    stack_cert = extractCertificateChain(pkcs7Object);

                    // Clean up
                    PKCS7_free(pkcs7Object);
                    EVP_cleanup();
                    ERR_free_strings();
                } else {
                    // Handle case where "signature" is null or not present
                    signature = "Error: Signature data missing or null";
                    std::cerr << "Error: Signature data missing or null" << std::endl;
                    std::string tokenstring = get_token(id, secret);
                    saveToken(tokenstring);
                    get_signature(bsHash, nik, passphrase, id, secret);
                }

            } catch (const std::exception& e) {
                signature = std::string("Error parsing JSON");
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }
            // std::cout << "Request sent successfully." << std::endl;
        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        // return signature;
    }

    // Cleanup libcurl
    curl_global_cleanup();


    std::vector<uint8_t> signature_vec;

    // Iterate through each character in the string and convert to uint8_t
    for (char c : signature) {
        signature_vec.push_back(static_cast<uint8_t>(c));
    }

    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    // Populate the map with data
    myMap["signature"] = signature_vec;
    myMap["stack_cert"] = stack_cert;

    return myMap;
}

std::unordered_map<std::string, std::vector<uint8_t> > SignBSrE::sign(std::vector<uint8_t>& placedHolder, const char* nik, const char* passphrase, const char* id, const char* secret) {
    std::unordered_map<std::string, std::vector<uint8_t> > result;

    Addons adns;
    // Sample query string
    const char query_byterange[] = "/ByteRange [0 /********** /********** /**********]";
    size_t byteRangePos = 0;
    size_t index = adns.findStringPosition(placedHolder, query_byterange, 0);
    if (index != -1) {
        byteRangePos = index;
    }

    size_t byteRangeEnd = byteRangePos + sizeof(query_byterange);

    // Find the index where the query occurs in the buffer
    size_t contentsTagPos = 0;
    index = adns.findStringPosition(placedHolder, "/Contents", byteRangeEnd);
    if (index != -1) {
        contentsTagPos = index;
    }

    size_t placeholderPos = 0;
    index = adns.findStringPosition(placedHolder, "<", contentsTagPos);
    if (index != -1) {
        placeholderPos = index;
    }


    size_t placeholderEnd = 0;
    index = adns.findStringPosition(placedHolder, ">", placeholderPos);
    if (index != -1) {
        placeholderEnd = index;
    }


    size_t placeholderLengthWithBrackets = placeholderEnd + 1 - placeholderPos;
    size_t placeholderLength = placeholderLengthWithBrackets - 2;

    
    std::array<int, 4> byteRange = {0, 0, 0, 0};
    byteRange[1] = placeholderPos;
    byteRange[2] = byteRange[1] + placeholderLengthWithBrackets;
    byteRange[3] = placedHolder.size() - byteRange[2];
    std::string actualByteRange = "ByteRange [" + std::to_string(byteRange[0]) + " " + std::to_string(byteRange[1]) + " " + std::to_string(byteRange[2]) + " " + std::to_string(byteRange[3]) + "]";
    actualByteRange += std::string((sizeof(query_byterange) - 2) - actualByteRange.size(), ' ');
    std::vector<uint8_t> updated_pdf = adns.getSubVector(placedHolder, 0, byteRangePos);

    std::vector<uint8_t> part3 = adns.getSubVector(placedHolder, byteRangeEnd-1, placedHolder.size()-1);
    updated_pdf.reserve(updated_pdf.size() + actualByteRange.size());
    std::copy(actualByteRange.begin(), actualByteRange.end(), std::back_inserter(updated_pdf));
    updated_pdf.reserve(updated_pdf.size() + part3.size());
    std::copy(part3.begin(), part3.end(), std::back_inserter(updated_pdf));
    
    std::vector<uint8_t> data_to_sign = adns.getSubVector(updated_pdf, 0, byteRange[1]-1);
    std::vector<uint8_t> digest2 = adns.getSubVector(updated_pdf, byteRange[2], (byteRange[2] + byteRange[3]-1));
    data_to_sign.reserve(data_to_sign.size() + digest2.size());
    std::copy(digest2.begin(), digest2.end(), std::back_inserter(data_to_sign));


    const unsigned char* data = data_to_sign.data();
    unsigned char sha256_digest[SHA256_DIGEST_LENGTH];
    SHA256(data, data_to_sign.size(), sha256_digest);

    std::string bsHash = adns.base64_encode(sha256_digest, SHA256_DIGEST_LENGTH);


    std::ofstream outFile("/Users/pusopskamsinas/Documents/Pribadi/Cpp/dksign_v2/output/data_to_sign.pdf", std::ios::out | std::ios::binary);
    if (!outFile) {
        std::cerr << "\nError opening file: " << "/Users/pusopskamsinas/Documents/Pribadi/Cpp/dksign_v2/output/data_to_sign.pdf" << std::endl;
        // return;
    }

    // Write the PDF buffer to the file
    outFile.write(reinterpret_cast<const char*>(data_to_sign.data()), data_to_sign.size());

    // Check if writing was successful
    if (!outFile) {
        std::cerr << "\nError writing to file: " << "/Users/pusopskamsinas/Documents/Pribadi/Cpp/dksign_v2/output/data_to_sign.pdf" << std::endl;
    } else {
        std::cout << "\nPDF saved to: " << "/Users/pusopskamsinas/Documents/Pribadi/Cpp/dksign_v2/output/data_to_sign.pdf" << std::endl;
    }

    // Close the file
    outFile.close();

    std::cout<< "Hash : " << bsHash << std::endl; 


    std::unordered_map<std::string, std::vector<uint8_t>> signature_vector = get_signature(bsHash, nik, passphrase, id, secret);
    std::string signature(signature_vector.at("signature").begin(), signature_vector.at("signature").end());
    
    std::vector<uint8_t> pkcs7Binary = adns.base64_decode(signature);
    std::string signature_hex = adns.vectorToHexString(pkcs7Binary);

    // extract_tsa_from_cms_hex(signature_hex);


    std::string repeated = "<" + signature_hex + std::string(35000 - signature_hex.length(), '0') + ">";
    std::vector<uint8_t> tte(repeated.begin(), repeated.end());

    std::vector<uint8_t> concatenatedBuffer;
    concatenatedBuffer.reserve(data_to_sign.size() + tte.size());
    concatenatedBuffer.insert(concatenatedBuffer.end(), data_to_sign.begin(), data_to_sign.begin() + byteRange[1]);
    concatenatedBuffer.insert(concatenatedBuffer.end(), tte.begin(), tte.end());
    concatenatedBuffer.insert(concatenatedBuffer.end(), data_to_sign.begin() + byteRange[1], data_to_sign.end());

    

    // Populate the map with data
    result["signed"] = concatenatedBuffer;
    result["stack_cert"] = signature_vector.at("stack_cert");

    return result;
}