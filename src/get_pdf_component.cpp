#include "../header/get_pdf_component.h"
#include "../header/addons.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <cctype>

GetPdfComponent::GetPdfComponent() {
    // Initialize private member variables or perform any necessary setup
}

std::unordered_map<std::string, std::string> GetPdfComponent::getPdfComponent(std::vector<uint8_t> pdf_buffer) {

    Addons adns; // Fungsi tambahan

    // Menemukan semua posisi karakter string startxref
    std::vector<size_t> positions = adns.findAllStringPosition(pdf_buffer, "startxref");

    // Mendapatkan nilai dari posisi startxref paling akhir (berbentuk string)
    std::vector<size_t> startxref = std::vector<size_t>(pdf_buffer.begin() + positions.back() + 10, pdf_buffer.end() - 6);
    std::string startxref_string(startxref.begin(), startxref.end());

    // Mengubah nilai startxref dari string menjadi integer
    size_t startxref_position = 0;
    try {
        startxref_position = std::stoi(startxref_string);
    } catch (const std::exception& e) {
        std::cout << "Failed to parse the string as int " << e.what() << std::endl;
    }

    // Mendapatkan nilai trailer paling akhir dari pdf
    std::vector<uint8_t> trailer_vec = std::vector<uint8_t>(pdf_buffer.begin() + startxref_position, pdf_buffer.end() - 6);

    // Trim whitespaces and parse the string into int
    // uint32_t xref_position = startxref_position;

    std::vector<uint8_t> root_ref_func = adns.get_value(trailer_vec, adns.stringToVector("/Root"));
    std::string root_ref = "none";
    if (!root_ref_func.empty()) {
        root_ref = adns.vectorToString(root_ref_func);
    }
    
    std::vector<uint8_t> info_ref_func = adns.get_value(trailer_vec, adns.stringToVector("/Info"));
    std::string info_ref = "none";
    if (!info_ref_func.empty()) {
        info_ref = adns.vectorToString(info_ref_func);
    }
    
    std::vector<uint8_t> id_ref_func = adns.get_value_id(trailer_vec, adns.stringToVector("/ID"));
    // Check if the vector is empty
    std::string id_ref = "none";
    if (!id_ref_func.empty()) {
        id_ref = adns.vectorToString(id_ref_func);
    }
    
    
    size_t max_index_start = 0;
    if (size_t index = adns.findStringPosition(trailer_vec, "/Size", 0)) {
        max_index_start = index;
    }

    std::vector<uint8_t> max_index_ref = std::vector<uint8_t>(trailer_vec.begin() + max_index_start + 6, trailer_vec.end());
    std::string max_index_string(max_index_ref.begin(), max_index_ref.end());
    int maxIndex = 0;
    for (std::string::iterator it = max_index_string.begin(); it != max_index_string.end(); ++it) {
        char c = *it; // Dereference iterator to get character
        if (std::isdigit(c)) {
            maxIndex = maxIndex * 10 + (c - '0');
        } else {
            // Character is not an integer, stop processing
            break;
        }
    }
    std::unordered_map<std::string, std::string> result;
    result["rootRef"] = root_ref;
    result["infoRef"] = info_ref;
    result["idRef"] = id_ref;
    result["trailerStart"] = std::to_string(positions.back());
    result["xRefPosition"] = std::to_string(startxref_position);
    result["maxIndex"] = std::to_string(maxIndex);


    // Placeholder for actual PDF reading logic
    return result;
}