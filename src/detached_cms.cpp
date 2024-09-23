// #include "../header/addons.h"
// #include "../header/detached_cms.h"
// #include <openssl/pkcs12.h>
// #include <openssl/pem.h>
// #include <openssl/x509.h>
// #include <openssl/evp.h>
// #include <openssl/err.h>
// #include <openssl/sha.h>
// #include <openssl/cms.h>
// #include <openssl/obj_mac.h>
// #include <openssl/bio.h>
// #include <openssl/buffer.h>
// #include <openssl/x509v3.h> // Include this header for STACK_OF(X509_ATTRIBUTE)
// #include <openssl/asn1.h>
// #include <openssl/objects.h>
// #include <ctime>
// #include <vector>
// #include <string>
// #include <iostream>
// #include <fstream>
// #include <iomanip>
// #include <sstream>
// #include <cstdint> // For uint8_t


// DetachedCMS::DetachedCMS() {
//     // Initialize private member variables or perform any necessary setup
// }

// // Function to create a detached CMS signature and save it as a hex string
// std::string createDetachedCMS2(const std::string& data, const std::string& plain_data, EVP_PKEY* pkey, X509* cert, STACK_OF(X509)* ca) {
//     Addons adns;

//     // Create CMS_ContentInfo object
//     CMS_ContentInfo* cms = CMS_ContentInfo_new();
//     if (!cms) {
//         throw std::runtime_error("Failed to create CMS_ContentInfo structure");
//     }

//     // Add signer certificate and private key
//     CMS_SignerInfo* signer_info = CMS_add1_signer(cms, cert, pkey, EVP_sha256(), CMS_DETACHED | CMS_BINARY | CMS_CADES);
//     if (!signer_info) {
//         CMS_ContentInfo_free(cms);
//         throw std::runtime_error("Failed to add signer to CMS");
//     }

//     // Add each certificate in the chain to the CMS
//     for (int i = 0; i < sk_X509_num(ca); ++i) {
//         X509* x509 = sk_X509_value(ca, i);
//         if (!CMS_add1_cert(cms, x509)) {
//             throw std::runtime_error("Failed to add certificate to CMS");
//         }
//     }

//     // Create a BIO for the pre-hashed data
//     std::vector<uint8_t> data_vec = adns.base64_decode_2(data);
//     std::string data_string = std::string(data_vec.begin(), data_vec.end());
//     const unsigned char* data_ptr = reinterpret_cast<const unsigned char*>(data_string.c_str());
//     size_t prehashed_data_len = sizeof(data_ptr);

//     if (!CMS_final_digest(cms, data_ptr, 32, nullptr, CMS_DETACHED | CMS_BINARY | CMS_CADES)) {
//         std::cerr << "Error finalizing CMS structure" << std::endl;
//         adns.handleErrors();
//     }

//     // Write CMS to a memory BIO
//     BIO* mem = BIO_new(BIO_s_mem());
//     if (!i2d_CMS_bio_stream(mem, cms, nullptr, CMS_DETACHED | CMS_BINARY | CMS_CADES)) {
//         std::cerr << "Unable to write CMS to memory BIO" << std::endl;
//         BIO_free(mem);
//         CMS_ContentInfo_free(cms);
//         return "";
//     }

//     // Read the content of the memory BIO into a buffer
//     BUF_MEM* bptr;
//     BIO_get_mem_ptr(mem, &bptr);
//     std::string hexStr = adns.binaryToHex_2((const unsigned char*)bptr->data, bptr->length);


//     BIO_free(mem);
//     CMS_ContentInfo_free(cms);
//     return hexStr;
// }


// std::string DetachedCMS::detached_cms_file(const std::string& hash, const std::string& data, const char* p12Path, const char* passphrase) {
//     try {

//         Addons adns;
//         // Verifikasi vrfy;
        
//         EVP_PKEY* privateKey = nullptr;
//         X509* cert = nullptr;
//         STACK_OF(X509)* ca = nullptr;

//         std::string p12(p12Path);
//         std::string password(passphrase);
//         if (!adns.loadPKCS12_2(p12, password, privateKey, cert, ca)) {
//             std::cerr << "Failed to load PKCS#12 file" << std::endl;
//             return "";
//         }

//         // Create CMS and add certificate chain
//         std::string signature_hex = createDetachedCMS2(hash, data, privateKey, cert, ca);

//         // std::cout << "CMSNYA : " << signature_hex <<std::endl;

//         // Clean up
//         EVP_PKEY_free(privateKey);
//         X509_free(cert);
//         sk_X509_free(ca);

//         // std::cout << "CMS signed file created successfully" << std::endl;

//         return signature_hex;

//     } catch (const std::exception& ex) {
//         std::cerr << "Error: " << ex.what() << std::endl;
//         return "";
//     }

//     return "";
// }


// std::string DetachedCMS::detached_cms(const std::string& hash, const std::string& data, const std::vector<uint8_t>& p12, const char* passphrase) {
//     try {

//         Addons adns;
//         // Verifikasi vrfy;
        
//         EVP_PKEY* privateKey = nullptr;
//         X509* cert = nullptr;
//         STACK_OF(X509)* ca = nullptr;

//         std::string password(passphrase);
//         if (!adns.loadPKCS12_from_base64(p12, password, privateKey, cert, ca)) {
//             std::cerr << "Failed to load PKCS#12 file" << std::endl;
//             return "";
//         }

//         // Create CMS and add certificate chain
//         std::string signature_hex = createDetachedCMS2(hash, data, privateKey, cert, ca);

//         // std::cout << "CMSNYA : " << signature_hex <<std::endl;

//         // Clean up
//         EVP_PKEY_free(privateKey);
//         X509_free(cert);
//         sk_X509_free(ca);

//         // std::cout << "CMS signed file created successfully" << std::endl;

//         return signature_hex;

//     } catch (const std::exception& ex) {
//         std::cerr << "Error: " << ex.what() << std::endl;
//         return "";
//     }

//     return "";
// }