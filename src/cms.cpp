#include "../header/cms.h"
#include "../header/addons.h"
#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/cms.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>

CMS::CMS() {
    // Initialize private member variables or perform any necessary setup
}

// Function to convert binary data to hex string
std::string binaryToHex(const unsigned char* data, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}



// Function to create a detached CMS signature and save it as a hex string
std::string createDetachedCMS(const std::string& data, EVP_PKEY* pkey, X509* cert, STACK_OF(X509)* ca) {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    std::vector<char> buffer = std::vector<char>(data.begin(), data.end());

    BIO* in = BIO_new_mem_buf(buffer.data(), buffer.size());
    if (!in) {
        std::cerr << "Unable to create BIO for input data" << std::endl;
        return "";
    }

    // Create CMS structure with the certificate chain
    CMS_ContentInfo* cms = CMS_sign(cert, pkey, ca, in, CMS_DETACHED | CMS_BINARY | CMS_CADES);
    BIO_free(in);
    if (!cms) {
        std::cerr << "Unable to create CMS structure" << std::endl;
        return "";
    }
    
    // Write CMS to a memory BIO
    BIO* mem = BIO_new(BIO_s_mem());
    if (!i2d_CMS_bio_stream(mem, cms, nullptr, CMS_BINARY)) {
        std::cerr << "Unable to write CMS to memory BIO" << std::endl;
        BIO_free(mem);
        CMS_ContentInfo_free(cms);
        return "";
    }

    // Read the content of the memory BIO into a buffer
    BUF_MEM* bptr;
    BIO_get_mem_ptr(mem, &bptr);
    std::string hexStr = binaryToHex((const unsigned char*)bptr->data, bptr->length);

    BIO_free(mem);
    CMS_ContentInfo_free(cms);
    return hexStr;
}

std::string CMS::generateCMS_file(const std::string& pkcs12Path, const std::string& password, const std::string& data) {
    Addons adns;
    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;
    STACK_OF(X509)* ca = nullptr;

    if (!adns.loadPKCS12(pkcs12Path, password, pkey, cert, ca)) {
        std::cerr << "Failed to load PKCS#12 file" << std::endl;
        return "";
    }

    std::string signature = createDetachedCMS(data, pkey, cert, ca);

    EVP_PKEY_free(pkey);
    X509_free(cert);
    sk_X509_pop_free(ca, X509_free);

    std::cout << "CMS signature created successfully" << std::endl;
    return signature;
}

std::string CMS::generateCMS(const std::vector<uint8_t>& p12, const std::string& password, const std::string& data) {

    Addons adns;
    // Verifikasi vrfy;
    
    EVP_PKEY* privateKey = nullptr;
    X509* cert = nullptr;
    STACK_OF(X509)* ca = nullptr;

    if (!adns.loadPKCS12_from_base64(p12, password, privateKey, cert, ca)) {
        std::cerr << "Failed to load PKCS#12 file" << std::endl;
        return "";
    }

    std::string signature = createDetachedCMS(data, privateKey, cert, ca);

    EVP_PKEY_free(privateKey);
    X509_free(cert);
    sk_X509_pop_free(ca, X509_free);

    // std::cout << "CMS signature created successfully" << std::endl;
    return signature;
}
