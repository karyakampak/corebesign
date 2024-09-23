#include "../header/add_placeholder.h"
#include "../header/calculate_hash.h"
#include "../header/sign_with_p12.h"
#include "../header/sign_bsre.h"
#include "../header/sign.h"
#include "../header/save_pdf.h"
#include "../header/get_ocsp_response.h"
#include "../header/add_ocsp_dict.h"
#include "../header/get_added_index.h"
#include "../header/add_ocsp_trailer.h"
#include "../header/addons.h"
#include "../header/place_signature.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <filesystem>  // C++17

using json = nlohmann::json;

extern "C" {
    
    // Function to free the allocated memory
    void free_string(const char* buffer) {
        free((void*)buffer);
    }

    char* placeHolder(int type, const char* pdf_path, int isProtected, const char* character, const char* imageorurl, int page, float x, float y, float width, float height, int isSeal){

        AddPlaceHolder addPlaceHolder;
        std::unordered_map<std::string, std::string> placeholderData = addPlaceHolder.addPlaceholder(type, pdf_path, isProtected, character, imageorurl, page, x, y, width, height, isSeal);
        std::string placedHolder = placeholderData.at("placedHolder");
        std::string catalogDict = placeholderData.at("catalog_dict");


        CalculateHash clhash;
        std::vector<uint8_t> holder = std::vector<uint8_t>(placedHolder.begin(), placedHolder.end());
        std::unordered_map<std::string, std::string> signed_pdf = clhash.calculate_hash(holder);

        Addons adns;
    
        // Allocate a buffer large enough to hold the string
        std::string pdfs = adns.vectorToHexString(adns.stringToVector(signed_pdf.at("pdf")));
        std::string catalog = adns.vectorToHexString(adns.stringToVector(catalogDict));


        json j_obj;
        j_obj["pdfs"] = pdfs;
        j_obj["hash"] = signed_pdf.at("hash");
        j_obj["byterange0"] = signed_pdf.at("byterange0");
        j_obj["byterange1"] = signed_pdf.at("byterange1");
        j_obj["byterange2"] = signed_pdf.at("byterange2");
        j_obj["byterange3"] = signed_pdf.at("byterange3");
        j_obj["catalog"] = catalog;


        // Convert json object to string
        std::string json_str = j_obj.dump();

        char* cstr = new char[json_str.size() + 1];
        std::strcpy(cstr, json_str.c_str());

        // std::cout << signed_pdf.at("pdf") << std::endl;
        // std::cout << encodedData << std::endl;
        
        // SavePdf svpdf;
        // svpdf.savePDF(adns.stringToVector(signed_pdf.at("pdf")), "./xxx.pdf");
        

        return cstr;
    }

    char* place(const char* pdf_char, const char* signature_char, const char* catalog_char, const char* byteRange0_char, const char* byteRange1_char, const char* byteRange2_char, const char* byteRange3_char, const char* outputPath) {
        std::string pdf(pdf_char);
        std::string signature(signature_char);
        std::string catalog(catalog_char);
        int byteRange0 = std::stoi(byteRange0_char);
        int byteRange1 = std::stoi(byteRange1_char);
        int byteRange2 = std::stoi(byteRange2_char);
        int byteRange3 = std::stoi(byteRange3_char);

        Addons adns;

        std::vector<uint8_t> pkcs7Binary = adns.base64_decode(signature);
        std::string signature_hex = adns.vectorToHexString(pkcs7Binary);

        std::vector<uint8_t> holder_vec = adns.hexStringToBytes(pdf);

        PlaceSignature plcSig;
        std::vector<uint8_t> pdf_vec = plcSig.place_signature(holder_vec, signature_hex, byteRange0, byteRange1, byteRange2, byteRange3);
        std::string pdf_str = adns.vectorToHexString(pdf_vec);

        char* cstr = new char[pdf_str.size() + 1];
        std::strcpy(cstr, pdf_str.c_str());
        
        SavePdf svpdf;
        svpdf.savePDF(pdf_vec, outputPath);
        

        return cstr;
    }
    
    void sampleSign(int type, const char* pdf_path, const char* character, const char* imageorurl, int page, float x, float y, float width, float height, int isSeal){
        std::string output = "output.pdf";
        AddPlaceHolder addPlaceHolder;
        std::unordered_map<std::string, std::string> placeholderData = addPlaceHolder.addPlaceholder(type, pdf_path, 0, character, imageorurl, page, x, y, width, height, isSeal);
        std::string placedHolder = placeholderData.at("placedHolder");
        std::string catalogDict = placeholderData.at("catalog_dict");

        Sign sgn;
        std::vector<uint8_t> holder = std::vector<uint8_t>(placedHolder.begin(), placedHolder.end());
        std::vector<uint8_t> signed_pdf = sgn.sign(holder);

        SavePdf svpdf;
        svpdf.savePDF(signed_pdf, output.c_str());
    }

    void signWithP12(int type, const char* pdf_path, int isProtected, const char* character, const char* imageorurl, const char* output_path, int page, float x, float y, float width, float height, const char* p12Path, const char* passphrase, int isSeal){

        AddPlaceHolder addPlaceHolder;
        std::unordered_map<std::string, std::string> placeholderData = addPlaceHolder.addPlaceholder(type, pdf_path, isProtected, character, imageorurl, page, x, y, width, height, isSeal);
        std::string placedHolder = placeholderData.at("placedHolder");
        std::string catalogDict = placeholderData.at("catalog_dict");

        SignWithP12 snp12;
        std::vector<uint8_t> holder = std::vector<uint8_t>(placedHolder.begin(), placedHolder.end());
        std::vector<uint8_t> signed_pdf = snp12.sign(holder, p12Path, passphrase);

        SavePdf svpdf;
        svpdf.savePDF(signed_pdf, output_path);
    }

    void signBSrE(int type, const char* pdf_path, int isProtected, const char* character, const char* imageorurl, const char* output_path, int page, float x, float y, float width, float height, const char* nik, const char* passphrase, const char* id, const char* secret, int isLTV, int isSeal){

        AddPlaceHolder addPlaceHolder;
        std::unordered_map<std::string, std::string> placeholderData = addPlaceHolder.addPlaceholder(type, pdf_path, isProtected, character, imageorurl, page, x, y, width, height, isSeal);
        std::string placedHolder = placeholderData.at("placedHolder");
        std::string catalogDict = placeholderData.at("catalog_dict");


        SignBSrE sBSrEnp12;
        std::vector<uint8_t> holder = std::vector<uint8_t>(placedHolder.begin(), placedHolder.end());
        std::unordered_map<std::string, std::vector<uint8_t> > signedData = sBSrEnp12.sign(holder, nik, passphrase, id, secret);
        std::vector<uint8_t> signed_pdf = signedData.at("signed");
        std::vector<uint8_t> stack_cert = signedData.at("stack_cert");

        if(isLTV == 0){
            SavePdf svpdf;
            svpdf.savePDF(signed_pdf, output_path);
        } else {
            GetOCSPResponse getOCSP;
            std::vector<uint8_t> ocsp = getOCSP.get_ocsp(stack_cert);

            // std::string ocspResponse(ocsp.begin(), ocsp.end());
            // std::cout << ocspResponse << std::endl;

            AddOCSPDict adocspdict;
            std::vector<uint8_t> catalog_dict = std::vector<uint8_t>(catalogDict.begin(), catalogDict.end());
            std::unordered_map<std::string, std::vector<uint8_t> > ocspDictAdded = adocspdict.add_ocsp_dict(placeholderData.at("pdf_component_size"), signedData, catalog_dict, ocsp);

            // std::string ocspResponse(ocspDictAdded.begin(), ocspDictAdded.end());
            // std::cout << ocspResponse << std::endl;
            GetAddedIndex getaddin;
            std::vector<size_t> added_index = getaddin.get_index_after_add_dict(ocspDictAdded.at("added_index"));
            // for (std::vector<size_t>::const_iterator it = added_index.begin(); it != added_index.end(); ++it) {
            //     std::cout << "New Object Index " << *it << std::endl;
            // }

            AddOCSPTrailer addOcspTrailer;
            std::vector<uint8_t> pdf_ltv_signature = addOcspTrailer.add_ocsp_trailer(signed_pdf, ocspDictAdded.at("signed_pdf"), added_index, placeholderData);
            
            SavePdf svpdf;
            svpdf.savePDF(pdf_ltv_signature, output_path);
        }
    }

}

void displayType() {
    std::cout << "\n=== Tipe ===\n";
    std::cout << "1. Tanda Tangan\n";
    std::cout << "2. Segel\n";
    std::cout << "Pilih tipe (1/2): ";
}

void displayJenis(std::string tipe) {
    std::cout << "\n=== Jenis " << tipe << " ===\n";
    std::cout << "1. Invisible\n";
    std::cout << "2. Visualisai Gambar\n";
    std::cout << "3. Visualisasi QRCode\n";
    std::cout << "4. Posisikan gambar pada karakter\n";
    std::cout << "5. Posisikan QRCode pada karakter\n";
    std::cout << "Pilih jenis tanda tangan (1/2/3/4/5): ";
}

void sgn(int type, const char* pdf_path, const char* character, const char* imageorurl, int page, float x, float y, float width, float height, int isSeal){
    sampleSign(type, pdf_path, character, imageorurl, page, x, y, width, height, isSeal);
}


int main(int argc, char* argv[]) {
    std::string version = "besign v1.0.0";
    std::string help = "HELP\n\nStandart commands :\ncreate <filename.pdf>\nsign <filename.pdf>\n";
    // Check if enough arguments are passed (including the program name)
    if (argc == 1) {
        std::cout << "Usage: besign <command> <filename.pdf>" << std::endl;
        return 1;  // Return error code if the file exists
    }
    if (argc == 2) {
        std::string command = argv[1];  // First argument ("create")
        if (command == "version" || command == "-version" || command == "--version" || command == "-v" || command == "-V") {
            std::cout << version << std::endl;
        } else if (command == "help" || command == "-help" || command == "--help" || command == "-h" || command == "-H") {
            std::cout << help << std::endl;
        } else {
            std::cout << "Usage: besign <command> <filename.pdf>" << std::endl;
            return 1;  // Return error code if the file exists
        }
        
    }
    if (argc == 3) {
        Addons adns;
        std::string command = argv[1];  // First argument ("create")
        std::string filename = argv[2]; // Second argument ("pdf")

        if (command == "create") {
            // Check for specific commands
            std::string output_filename = filename;
            if (adns.has_pdf_extension(output_filename)) {
                std::string pdf = adns.constant("pdf");
                std::vector<uint8_t> pdf_vec = adns.base64_decode(pdf);
                if (adns.save_pdf(output_filename, pdf_vec)) {
                    std::cout << "PDF saved successfully to '" << output_filename << "'\n";
                    return 0;
                } else {
                    std::cerr << "Failed to save the PDF.\n";
                    return 1;  // Return error code if the file exists
                }
            } else {
                std::cout << "Usage: besign create <filename.pdf>" << std::endl;
                return 1;  // Return error code if the file exists
            }
        } else if (command == "sign") {
            std::string dummy = "";
            float fdummy = 50;
            int tipe;
            std::string tipe_str;
            int jenis;
            std::string imageorurl;
            std::string posisi;
            std::string karakter;
            int page;
            float x;
            float y;
            float width;
            float height;
            float isSeal;
            // Check if the file exists using filesystem
            if (std::filesystem::exists(filename)) {
                displayType();
                std::cin >> tipe;
                if (tipe != 1 && tipe != 2) {
                    tipe_str = "Tanda tangan";
                }
                
                switch (tipe) {
                    case 1:
                        tipe_str = "Tanda tangan";
                        break;
                    case 2:
                        tipe_str = "Segel";
                        break;
                    
                }
                displayJenis(tipe_str);
                std::cin >> jenis;
                if (jenis != 1 && jenis != 2 && jenis != 3 && jenis != 4 && jenis != 5) {
                    sgn(0, filename.c_str(), dummy.c_str(), dummy.c_str(), 1, fdummy, fdummy, fdummy, fdummy, (tipe-1));
                    std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                }
                switch (jenis) {
                    case 1:
                        sgn(0, filename.c_str(), dummy.c_str(), dummy.c_str(), 1, fdummy, fdummy, fdummy, fdummy, (tipe-1));
                        std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                        break;
                    case 2: {
                        std::cout << "Masukkan path gambar (e.g /path/to/image.jpg): ";
                        std::cin >> imageorurl;
                        std::cout << "Gambar akan di tempel pada halaman 1\n";
                        std::cout << "Masukkan posisi x,y,width,height tanpa spasi (e.g 100,100,200,50): ";
                        std::cin >> posisi;
                        // Use a stringstream to parse the values
                        std::stringstream ss1(posisi);
                        // Parse x, y as integers, and width, height as floats
                        char comma1;  // To skip commas
                        ss1 >> x >> comma1 >> y >> comma1 >> width >> comma1 >> height;
                        sgn(1, filename.c_str(), dummy.c_str(), imageorurl.c_str(), 1, x, y, width, height, (tipe-1));
                        std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                        break;
                    }
                    case 3: {
                        std::cout << "Masukkan path url (e.g https://google.com): ";
                        std::cin >> imageorurl;
                        std::cout << "QRCode akan di tempel pada halaman 1\n";
                        std::cout << "Masukkan posisi x,y,width,height tanpa spasi (e.g 100,100,200,50): ";
                        std::cin >> posisi;
                        // Use a stringstream to parse the values
                        std::stringstream ss2(posisi);
                        // Parse x, y as integers, and width, height as floats
                        char comma2;  // To skip commas
                        ss2 >> x >> comma2 >> y >> comma2 >> width >> comma2 >> height;
                        sgn(2, filename.c_str(), dummy.c_str(), imageorurl.c_str(), 1, x, y, width, height, (tipe-1));
                        std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                        break;
                    }
                    case 4:
                        std::cout << "Masukkan path gambar (e.g /path/to/image.jpg): ";
                        std::cin >> imageorurl;
                        std::cout << "Note: Karakter harus ada di halaman 1\nMasukkan karakter untuk penempatan Gambar (e.g #): ";
                        std::cin >> karakter;
                        sgn(3, filename.c_str(), karakter.c_str(), imageorurl.c_str(), 1, fdummy, fdummy, fdummy, fdummy, (tipe-1));
                        std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                        break;
                    case 5:
                        std::cout << "Masukkan path url (e.g https://google.com): ";
                        std::cin >> imageorurl;
                        std::cout << "Note: Karakter harus ada di halaman 1\nMasukkan karakter untuk penempatan QRCode (e.g #): ";
                        std::cin >> karakter;
                        sgn(4, filename.c_str(), karakter.c_str(), imageorurl.c_str(), 1, fdummy, fdummy, fdummy, fdummy, (tipe-1));
                        std::cout << "Proses " << tipe_str << "Selesai\nOutput disimpan dengan nama 'output.pdf'";
                        break;
                }

                return 0;
            } else {
                std::cout << "File not exist" << std::endl;
                return 1;  // Return error code if the file exists
            }
        } else {
            std::cout << "Usage: besign <command> <filename.pdf>" << std::endl;
            return 1;  // Return error code if the file exists
        }
    }
    if (argc >= 3) {
        std::cout << "Usage: besign <command> <filename.pdf>" << std::endl;
        return 1;  // Return error code if the file exists
    }
}