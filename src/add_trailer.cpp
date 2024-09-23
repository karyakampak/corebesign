#include "../header/add_trailer.h"
#include "../header/addons.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <openssl/err.h>
#include <uuid/uuid.h>
#include <iostream>

AddTrailer::AddTrailer() {
    // Initialize private member variables or perform any necessary setup
}

std::unordered_map<std::string, std::string> create_buffer_trailer(const std::vector<uint8_t>& pdf_awal, const std::vector<uint8_t>& pdf_akhir, const std::vector<size_t>& object_id, const std::unordered_map<std::string, std::string>& read_pdf) {
    
    Addons adns;

    std::string trailer;
    trailer += "\n\nxref\n0 1\n0000000000 65535 f \n";
    size_t pdf_awal_len = pdf_awal.size();
    for (size_t i = 0; i < object_id.size(); ++i) {
        std::vector<uint8_t> pdf_to_query = pdf_akhir; // Assuming get_offset is a function you have defined elsewhere
        size_t offset = adns.get_offset(pdf_awal_len, pdf_to_query, std::to_string(object_id[i])); // This needs to be defined or replaced with equivalent logic
        std::string padded_offset = std::string(10 - std::to_string(offset + 1).length(), '0') + std::to_string(offset + 1);
        trailer += std::to_string(object_id[i]) + " 1\n" + padded_offset + " 00000 n \n";
    }
    trailer += "trailer\n<<";
    trailer += "\n/Size " + std::to_string(object_id.back() + 1);
    trailer += "\n/Root " + read_pdf.at("rootRef");
    if(read_pdf.at("infoRef") != "none"){
        trailer += "\n/Info " + read_pdf.at("infoRef");
    }
    // trailer += "\n/Info " + read_pdf.at("infoRef");
    if(read_pdf.at("idRef") != "none"){
        trailer += "\n/ID " + read_pdf.at("idRef")+"> <"+adns.generateUUID2()+">]";
        // trailer += "\n/ID " + read_pdf.at("idRef");
    }
    trailer += "\n/Prev " + read_pdf.at("xRefPosition");
    trailer += "\n>>\nstartxref\n" + std::to_string(pdf_akhir.size() + 1) + "\n%%EOF";

    std::unordered_map<std::string, std::string> result;
    // result["trailer"] = std::vector<uint8_t>(trailer.begin(), trailer.end());
    result["trailer"] = trailer;
    result["pdf_component_size"] = std::to_string(object_id.back() + 1);
    result["pdf_component_root"] = read_pdf.at("rootRef");
    result["pdf_component_info"] = read_pdf.at("infoRef");
    result["pdf_component_id"] = read_pdf.at("idRef");
    result["pdf_component_prev"] = std::to_string(pdf_akhir.size() + 1);

    return result;
}

std::unordered_map<std::string, std::string> AddTrailer::add_trailer(const std::vector<uint8_t>& pdf_content, const std::vector<uint8_t>& pdf_buffer, const std::vector<size_t>& object_id, const std::unordered_map<std::string, std::string>& read_pdf) {
    std::vector<uint8_t> result;
    std::vector<uint8_t> pdf_buffer_clone = pdf_buffer;
    std::unordered_map<std::string, std::string> trailerMap = create_buffer_trailer(pdf_content, pdf_buffer, object_id, read_pdf);
    result.insert(result.end(), pdf_buffer_clone.begin(), pdf_buffer_clone.end());
    result.insert(result.end(), trailerMap.at("trailer").begin(), trailerMap.at("trailer").end());


    std::unordered_map<std::string, std::string> rslt;

    std::string placedHolder(result.begin(), result.end());
    rslt["placedHolder"] = placedHolder;


    rslt["pdf_component_size"] = trailerMap.at("pdf_component_size");
    rslt["pdf_component_root"] = trailerMap.at("pdf_component_root");
    rslt["pdf_component_info"] = trailerMap.at("pdf_component_info");
    rslt["pdf_component_id"] = trailerMap.at("pdf_component_id");
    rslt["pdf_component_prev"] = trailerMap.at("pdf_component_prev");

    return rslt;
}