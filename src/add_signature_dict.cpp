#include "../header/add_signature_dict.h"
#include "../header/addons.h"
#include "../header/visualization.h"
#include "../header/stb_image.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdio>  // For std::remove
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <opencv2/opencv.hpp>   // For image manipulation (OpenCV)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <zlib.h> 

AddSignatureDict::AddSignatureDict() {
    // Initialize private member variables or perform any necessary setup
}


std::unordered_map<std::string, std::vector<uint8_t> > AddSignatureDict::add_signaturedict(std::vector<unsigned char> pdf_content, int page, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign) {

    Addons adns;

    size_t max_index = 0;
    if (read_pdf.count("maxIndex") > 0) {
        try {
            size_t number = std::stoull(read_pdf.at("maxIndex"));
            if (number <= std::numeric_limits<size_t>::max()) {
                max_index = number;
            } else {
                std::cout << "Parsed number is too large to fit into size_t" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to parse the string as a number" << std::endl;
        }
    }
    std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
    std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
    std::time_t timestamp_time_t = formatted_timestamp.count();
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
    std::string formatted_timestamp_with_z = ss.str() + "Z";
    std::vector<unsigned char> result;
    std::string object_signature_start = "\n" + std::to_string(max_index + 1) + " 0 obj\n<<\n";
    std::string contents = "/Contents <" + std::string(35000, '0') + ">";
    std::string object_signature_end = "\n>>\nendobj";
    // std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /adbe.pkcs7.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /ETSI.CAdES.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string object_annot_start = "\n\n" + std::to_string(max_index + 2) + " 0 obj\n<<\n";
    std::string object_annot_end = "\n>>\nendobj";
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration_since_epoch = current_time.time_since_epoch();
    std::chrono::milliseconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    std::string object_page_ref_annot = object_page_ref[page - 1];
    std::string annot_dict;
    std::vector<unsigned char> img_dict_signature;

    annot_dict = object_annot_start + "/Type /Annot\n/Subtype /Widget\n/FT /Sig\n/Rect [0 0 0 0]\n/V " + std::to_string(max_index + 1) + " 0 R\n/T (Signature_" + std::to_string(milliseconds) + ")\n/F 4\n/P " + object_page_ref_annot + object_annot_end;
    
    std::string object_acroform_start = "\n\n" + std::to_string(max_index + 3) + " 0 obj\n<<\n";
    std::string object_acroform_end = "\n>>\nendobj";
    std::string acroform_dict = object_acroform_start + "/Type /AcroForm\n/SigFlags 3\n/Fields [" + std::to_string(max_index + 2) + " 0 R]" + object_acroform_end;
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        acroform_dict = "\n\n" + check_sign.at("acroform_last_start") + " " + std::to_string(max_index + 2) + " 0 R" + check_sign.at("acroform_last_end");
    }
    // std::string object__start = "\n\n" + std::to_string(max_index + 4) + " 0 obj\n";
    // std::string object__end = "\nendobj";
    // std::string _dict = object__start + "undefined" + object__end;

    std::string page_dict = adns.get_page(pdf_content, object_page_ref_annot);
    std::unordered_map<std::string, std::string> annots_on_page = adns.checkAnnotOnPage(page_dict);

    std::unordered_map<std::string, std::string> read_pdf_dict = read_pdf;
    std::string catalog_dict = adns.get_catalog(pdf_content, read_pdf_dict);
    std::string root_obj = "\n" + catalog_dict.substr(0, catalog_dict.length() - 4) + "\n/AcroForm " + std::to_string(max_index + 3) + " 0 R\n>>\nendobj";
    std::vector<unsigned char> root_obj_vec(root_obj.begin(), root_obj.end());


    std::string page_obj = "\n" + page_dict.substr(0, page_dict.length() - 4) + "/Annots [" + std::to_string(max_index + 2) + " 0 R]\n\n>>\nendobj";
    if (annots_on_page.at("status") == "exist") {
        page_obj = "\n" + annots_on_page.at("page_before") + " " + std::to_string(max_index + 2) + " 0 R" + annots_on_page.at("page_after") + "\nendobj";
    }


    std::vector<unsigned char> page_obj_vec(page_obj.begin(), page_obj.end());

    size_t rootIndex = adns.extract_integer(read_pdf.at("rootRef"));
    size_t pageIndex = adns.extract_integer(page_dict.substr(0, 10));

    std::vector<size_t> x_list;
    result.insert(result.end(), pdf_content.begin(), pdf_content.end());
    result.insert(result.end(), signature_dict.begin(), signature_dict.end());
    x_list.push_back(max_index + 1);
    result.insert(result.end(), annot_dict.begin(), annot_dict.end());
    x_list.push_back(max_index + 2);
    result.insert(result.end(), acroform_dict.begin(), acroform_dict.end());
    x_list.push_back(max_index + 3);
    result.insert(result.end(), _dict.begin(), _dict.end());
    x_list.push_back(max_index + 4);
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    } else {
        result.insert(result.end(), root_obj_vec.begin(), root_obj_vec.end());
        x_list.push_back(rootIndex);
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    }

    std::sort(x_list.begin(), x_list.end());
    std::string added_index_string = adns.vector_size_t_to_string(x_list);
    std::vector<uint8_t> added_index = adns.stringToVector(added_index_string);


    // Populate the map with data
    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    myMap["added_dict"] = result;
    myMap["catalog_dict"] = root_obj_vec;
    myMap["added_index"] = added_index;

    return myMap;
}

std::unordered_map<std::string, std::vector<uint8_t> > AddSignatureDict::add_signaturedict_with_image(std::vector<unsigned char> pdf_content, const char* input_image_path_string, int page, float x, float y, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign) {

    Addons adns;

    size_t max_index = 0;
    if (read_pdf.count("maxIndex") > 0) {
        try {
            size_t number = std::stoull(read_pdf.at("maxIndex"));
            if (number <= std::numeric_limits<size_t>::max()) {
                max_index = number;
            } else {
                std::cout << "Parsed number is too large to fit into size_t" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to parse the string as a number" << std::endl;
        }
    }
    std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
    std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
    std::time_t timestamp_time_t = formatted_timestamp.count();
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
    std::string formatted_timestamp_with_z = ss.str() + "Z";
    std::vector<unsigned char> result;
    std::string object_signature_start = "\n" + std::to_string(max_index + 1) + " 0 obj\n<<\n";
    std::string contents = "/Contents <" + std::string(35000, '0') + ">";
    std::string object_signature_end = "\n>>\nendobj";
    // std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /adbe.pkcs7.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /ETSI.CAdES.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string object_annot_start = "\n\n" + std::to_string(max_index + 2) + " 0 obj\n<<\n";
    std::string object_annot_end = "\n>>\nendobj";
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration_since_epoch = current_time.time_since_epoch();
    std::chrono::milliseconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    std::string object_page_ref_annot = object_page_ref[page - 1];
    std::string annot_dict;
    std::vector<unsigned char> img_dict_signature;


    std::ifstream image_file(input_image_path_string, std::ios::binary);
    if (image_file) {


        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                            std::istreambuf_iterator<char>());

        int widthImage, heightImage, channels;
        std::vector<uint8_t> imageData = adns.decodePNG(input_image_path_string, widthImage, heightImage, channels);



        Visualization vzsl;
        // std::vector<uint8_t> visualization = vzsl.get_image(image_data);
        cv::Mat img = cv::imdecode(image_data, cv::IMREAD_UNCHANGED);
        std::vector<uint8_t> visualization = adns.process_image(img);
        std::string visualization_string(visualization.begin(), visualization.end());

        float rectWidth;
        float rectHeight;
        if (width/height > widthImage/heightImage)
        {
            rectHeight = height;
            rectWidth = rectHeight/heightImage*widthImage;
            x = x + ((width-rectWidth)/2);
        } else{
            rectWidth = width;
            rectHeight = (rectWidth/widthImage)*heightImage;
            y = y + ((height-rectHeight)/2);
        }
        

        annot_dict = object_annot_start + "/Type /Annot\n/Subtype /Widget\n/FT /Sig\n/Rect [" + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(x + rectWidth) + " " + std::to_string(y+rectHeight) +  "]\n/AP <<\n/N " + std::to_string(max_index + 5) + " 0 R\n>>\n/V " + std::to_string(max_index + 1) + " 0 R\n/T (Signature_" + std::to_string(milliseconds) + ")\n/F 4\n/P " + object_page_ref_annot + object_annot_end;
        

        float widthSign = static_cast<float>(widthImage) / (widthImage/rectWidth); // Convert numerator to float before division
        float heightSign = static_cast<float>(heightImage) / (heightImage/rectHeight); // Convert numerator to float before division

        // Copy the contents into a vector
        std::vector<unsigned char> vec(visualization_string.begin(), visualization_string.end());

        std::string img_obj_0_start = "\n\n" + std::to_string(max_index + 5) + " 0 obj\n";
        std::string img_obj_0_end = "\nendobj";
        std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 "+std::to_string(heightSign)+" 0 0 cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        // std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 -"+std::to_string(heightSign)+" 0 "+std::to_string(heightSign)+" cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        std::string img_obj_0_dict = img_obj_0_start + "<<\n/Length " + std::to_string(stream_obj_0.size()) + "\n/Type /XObject\n/Subtype /Form\n/Resources <<\n/XObject <<\n/X" + std::to_string(milliseconds) + " " + std::to_string(max_index + 6) + " 0 R\n>>\n>>\n/FormType 1\n/BBox [0.0 0.0 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+"]\n/Matrix [1 0 0 1 0 0]\n>>\nstream\n" + stream_obj_0 + "\nendstream" + img_obj_0_end;
        std::string img_obj_1_start = "\n\n" + std::to_string(max_index + 6) + " 0 obj\n<<\n";
        std::string img_obj_1_end = "\nendobj";
        std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Filter /DCTDecode\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        // std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        std::string img_obj_1_dict_2 = "\nendstream" + img_obj_1_end;
        std::vector<unsigned char> img_obj_final_vec;
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_0_dict.begin(), img_obj_0_dict.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_1.begin(), img_obj_1_dict_1.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), vec.begin(), vec.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_2.begin(), img_obj_1_dict_2.end());

        img_dict_signature = img_obj_final_vec;

    } else {
        std::cerr << "Error reading from the input image file" << std::endl;
    }

    
    std::string object_acroform_start = "\n\n" + std::to_string(max_index + 3) + " 0 obj\n<<\n";
    std::string object_acroform_end = "\n>>\nendobj";
    std::string acroform_dict = object_acroform_start + "/Type /AcroForm\n/SigFlags 3\n/Fields [" + std::to_string(max_index + 2) + " 0 R]" + object_acroform_end;
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        acroform_dict = "\n\n" + check_sign.at("acroform_last_start") + " " + std::to_string(max_index + 2) + " 0 R" + check_sign.at("acroform_last_end");
    }
    // std::string object__start = "\n\n" + std::to_string(max_index + 4) + " 0 obj\n";
    // std::string object__end = "\nendobj";
    // std::string _dict = object__start + "undefined" + object__end;

    std::string page_dict = adns.get_page(pdf_content, object_page_ref_annot);
    std::unordered_map<std::string, std::string> annots_on_page = adns.checkAnnotOnPage(page_dict);

    std::unordered_map<std::string, std::string> read_pdf_dict = read_pdf;
    std::string catalog_dict = adns.get_catalog(pdf_content, read_pdf_dict);
    std::string root_obj = "\n" + catalog_dict.substr(0, catalog_dict.length() - 4) + "\n/AcroForm " + std::to_string(max_index + 3) + " 0 R\n>>\nendobj";
    std::vector<unsigned char> root_obj_vec(root_obj.begin(), root_obj.end());


    std::string page_obj = "\n" + page_dict.substr(0, page_dict.length() - 4) + "/Annots [" + std::to_string(max_index + 2) + " 0 R]\n\n>>\nendobj";
    if (annots_on_page.at("status") == "exist") {
        page_obj = "\n" + annots_on_page.at("page_before") + " " + std::to_string(max_index + 2) + " 0 R" + annots_on_page.at("page_after") + "\nendobj";
    }


    std::vector<unsigned char> page_obj_vec(page_obj.begin(), page_obj.end());


    size_t rootIndex = adns.extract_integer(read_pdf.at("rootRef"));
    size_t pageIndex = adns.extract_integer(page_dict.substr(0, 10));

    std::vector<size_t> x_list;
    result.insert(result.end(), pdf_content.begin(), pdf_content.end());
    result.insert(result.end(), signature_dict.begin(), signature_dict.end());
    x_list.push_back(max_index + 1);
    result.insert(result.end(), annot_dict.begin(), annot_dict.end());
    x_list.push_back(max_index + 2);
    result.insert(result.end(), acroform_dict.begin(), acroform_dict.end());
    x_list.push_back(max_index + 3);
    result.insert(result.end(), _dict.begin(), _dict.end());
    x_list.push_back(max_index + 4);
    result.insert(result.end(), img_dict_signature.begin(), img_dict_signature.end());
    x_list.push_back(max_index + 5);
    x_list.push_back(max_index + 6);  // XObject
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    } else {
        result.insert(result.end(), root_obj_vec.begin(), root_obj_vec.end());
        x_list.push_back(rootIndex);
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    }

    std::sort(x_list.begin(), x_list.end());
    std::string added_index_string = adns.vector_size_t_to_string(x_list);
    std::vector<uint8_t> added_index = adns.stringToVector(added_index_string);


    // Populate the map with data
    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    myMap["added_dict"] = result;
    myMap["catalog_dict"] = root_obj_vec;
    myMap["added_index"] = added_index;

    return myMap;
}

std::unordered_map<std::string, std::vector<uint8_t> > AddSignatureDict::add_signaturedict_with_qr(std::vector<unsigned char> pdf_content, const char* url, int page, float x, float y, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign) {

    Addons adns;

    size_t max_index = 0;
    if (read_pdf.count("maxIndex") > 0) {
        try {
            size_t number = std::stoull(read_pdf.at("maxIndex"));
            if (number <= std::numeric_limits<size_t>::max()) {
                max_index = number;
            } else {
                std::cout << "Parsed number is too large to fit into size_t" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to parse the string as a number" << std::endl;
        }
    }
    std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
    std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
    std::time_t timestamp_time_t = formatted_timestamp.count();
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
    std::string formatted_timestamp_with_z = ss.str() + "Z";
    std::vector<unsigned char> result;
    std::string object_signature_start = "\n" + std::to_string(max_index + 1) + " 0 obj\n<<\n";
    std::string contents = "/Contents <" + std::string(35000, '0') + ">";
    std::string object_signature_end = "\n>>\nendobj";
    // std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /adbe.pkcs7.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /ETSI.CAdES.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string object_annot_start = "\n\n" + std::to_string(max_index + 2) + " 0 obj\n<<\n";
    std::string object_annot_end = "\n>>\nendobj";
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration_since_epoch = current_time.time_since_epoch();
    std::chrono::milliseconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    std::string object_page_ref_annot = object_page_ref[page - 1];
    std::string annot_dict;
    std::vector<unsigned char> img_dict_signature;


    std::string text = url;
    std::string filename = std::to_string(milliseconds)+".jpg";
    std::vector<unsigned char> image_char = adns.saveQRCodeAsJPG(text, filename);
    // std::vector<unsigned char> image_char = adns.saveQRCodeAsJPG_V2(text, "/Users/pusopskamsinas/Documents/Pribadi/Cpp/logo.png", filename);
    // std::vector<unsigned char> image_char = adns.saveQRCodeAsJPG_2(text);

    std::ifstream image_file(filename.c_str(), std::ios::binary);
    if (image_file) {
        std::remove(filename.c_str());


        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                            std::istreambuf_iterator<char>());

        int widthImage, heightImage, channels;
        stbi_load_from_memory(image_char.data(), image_char.size(), &widthImage, &heightImage, &channels, 0);

        Visualization vzsl;
        // std::vector<uint8_t> visualization = vzsl.get_image(image_data);
        cv::Mat img = cv::imdecode(image_data, cv::IMREAD_UNCHANGED);
        std::vector<uint8_t> visualization = adns.process_image(img);
        std::string visualization_string(visualization.begin(), visualization.end());

        float rectWidth;
        float rectHeight;
        if (width/height > widthImage/heightImage)
        {
            rectHeight = height;
            rectWidth = rectHeight/heightImage*widthImage;
            x = x + ((width-rectWidth)/2);
        } else{
            rectWidth = width;
            rectHeight = (rectWidth/widthImage)*heightImage;
            y = y + ((height-rectHeight)/2);
        }
        

        annot_dict = object_annot_start + "/Type /Annot\n/Subtype /Widget\n/FT /Sig\n/Rect [" + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(x + rectWidth) + " " + std::to_string(y+rectHeight) +  "]\n/AP <<\n/N " + std::to_string(max_index + 5) + " 0 R\n>>\n/V " + std::to_string(max_index + 1) + " 0 R\n/T (Signature_" + std::to_string(milliseconds) + ")\n/F 4\n/P " + object_page_ref_annot + object_annot_end;
        

        float widthSign = static_cast<float>(widthImage) / (widthImage/rectWidth); // Convert numerator to float before division
        float heightSign = static_cast<float>(heightImage) / (heightImage/rectHeight); // Convert numerator to float before division

        // Copy the contents into a vector
        std::vector<unsigned char> vec(visualization_string.begin(), visualization_string.end());

        std::string img_obj_0_start = "\n\n" + std::to_string(max_index + 5) + " 0 obj\n";
        std::string img_obj_0_end = "\nendobj";
        std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 "+std::to_string(heightSign)+" 0 0 cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        // std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 -"+std::to_string(heightSign)+" 0 "+std::to_string(heightSign)+" cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        std::string img_obj_0_dict = img_obj_0_start + "<<\n/Length " + std::to_string(stream_obj_0.size()) + "\n/Type /XObject\n/Subtype /Form\n/Resources <<\n/XObject <<\n/X" + std::to_string(milliseconds) + " " + std::to_string(max_index + 6) + " 0 R\n>>\n>>\n/FormType 1\n/BBox [0.0 0.0 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+"]\n/Matrix [1 0 0 1 0 0]\n>>\nstream\n" + stream_obj_0 + "\nendstream" + img_obj_0_end;
        std::string img_obj_1_start = "\n\n" + std::to_string(max_index + 6) + " 0 obj\n<<\n";
        std::string img_obj_1_end = "\nendobj";
        std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Filter /DCTDecode\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(vec.size()) + "\n>>\nstream\n";
        // std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        std::string img_obj_1_dict_2 = "\nendstream" + img_obj_1_end;
        std::vector<unsigned char> img_obj_final_vec;
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_0_dict.begin(), img_obj_0_dict.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_1.begin(), img_obj_1_dict_1.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), vec.begin(), vec.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_2.begin(), img_obj_1_dict_2.end());

        img_dict_signature = img_obj_final_vec;

    } else {
        std::remove(filename.c_str());
        std::cerr << "Error reading from the input image file" << std::endl;
    }

    
    std::string object_acroform_start = "\n\n" + std::to_string(max_index + 3) + " 0 obj\n<<\n";
    std::string object_acroform_end = "\n>>\nendobj";
    std::string acroform_dict = object_acroform_start + "/Type /AcroForm\n/SigFlags 3\n/Fields [" + std::to_string(max_index + 2) + " 0 R]" + object_acroform_end;
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        acroform_dict = "\n\n" + check_sign.at("acroform_last_start") + " " + std::to_string(max_index + 2) + " 0 R" + check_sign.at("acroform_last_end");
    }
    // std::string object__start = "\n\n" + std::to_string(max_index + 4) + " 0 obj\n";
    // std::string object__end = "\nendobj";
    // std::string _dict = object__start + "undefined" + object__end;

    std::string page_dict = adns.get_page(pdf_content, object_page_ref_annot);
    std::unordered_map<std::string, std::string> annots_on_page = adns.checkAnnotOnPage(page_dict);

    std::unordered_map<std::string, std::string> read_pdf_dict = read_pdf;
    std::string catalog_dict = adns.get_catalog(pdf_content, read_pdf_dict);
    std::string root_obj = "\n" + catalog_dict.substr(0, catalog_dict.length() - 4) + "\n/AcroForm " + std::to_string(max_index + 3) + " 0 R\n>>\nendobj";
    std::vector<unsigned char> root_obj_vec(root_obj.begin(), root_obj.end());


    std::string page_obj = "\n" + page_dict.substr(0, page_dict.length() - 4) + "/Annots [" + std::to_string(max_index + 2) + " 0 R]\n\n>>\nendobj";
    if (annots_on_page.at("status") == "exist") {
        page_obj = "\n" + annots_on_page.at("page_before") + " " + std::to_string(max_index + 2) + " 0 R" + annots_on_page.at("page_after") + "\nendobj";
    }


    std::vector<unsigned char> page_obj_vec(page_obj.begin(), page_obj.end());


    size_t rootIndex = adns.extract_integer(read_pdf.at("rootRef"));
    size_t pageIndex = adns.extract_integer(page_dict.substr(0, 10));

    std::vector<size_t> x_list;
    result.insert(result.end(), pdf_content.begin(), pdf_content.end());
    result.insert(result.end(), signature_dict.begin(), signature_dict.end());
    x_list.push_back(max_index + 1);
    result.insert(result.end(), annot_dict.begin(), annot_dict.end());
    x_list.push_back(max_index + 2);
    result.insert(result.end(), acroform_dict.begin(), acroform_dict.end());
    x_list.push_back(max_index + 3);
    result.insert(result.end(), _dict.begin(), _dict.end());
    x_list.push_back(max_index + 4);
    result.insert(result.end(), img_dict_signature.begin(), img_dict_signature.end());
    x_list.push_back(max_index + 5);
    x_list.push_back(max_index + 6);  // XObject
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    } else {
        result.insert(result.end(), root_obj_vec.begin(), root_obj_vec.end());
        x_list.push_back(rootIndex);
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    }

    std::sort(x_list.begin(), x_list.end());
    std::string added_index_string = adns.vector_size_t_to_string(x_list);
    std::vector<uint8_t> added_index = adns.stringToVector(added_index_string);


    // Populate the map with data
    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    myMap["added_dict"] = result;
    myMap["catalog_dict"] = root_obj_vec;
    myMap["added_index"] = added_index;

    return myMap;
}

std::unordered_map<std::string, std::vector<uint8_t> > AddSignatureDict::add_signaturedict_image_to_char(std::vector<unsigned char> pdf_content, const char* character, const char* input_image_path_string, int page, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign) {

    Addons adns;

    size_t max_index = 0;
    if (read_pdf.count("maxIndex") > 0) {
        try {
            size_t number = std::stoull(read_pdf.at("maxIndex"));
            if (number <= std::numeric_limits<size_t>::max()) {
                max_index = number;
            } else {
                std::cout << "Parsed number is too large to fit into size_t" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to parse the string as a number" << std::endl;
        }
    }
    std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
    std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
    std::time_t timestamp_time_t = formatted_timestamp.count();
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
    std::string formatted_timestamp_with_z = ss.str() + "Z";
    std::vector<unsigned char> result;
    std::string object_signature_start = "\n" + std::to_string(max_index + 1) + " 0 obj\n<<\n";
    std::string contents = "/Contents <" + std::string(35000, '0') + ">";
    std::string object_signature_end = "\n>>\nendobj";
    // std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /adbe.pkcs7.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /ETSI.CAdES.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string object_annot_start = "\n\n" + std::to_string(max_index + 2) + " 0 obj\n<<\n";
    std::string object_annot_end = "\n>>\nendobj";
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration_since_epoch = current_time.time_since_epoch();
    std::chrono::milliseconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    std::string object_page_ref_annot = object_page_ref[page - 1];
    std::string annot_dict;
    std::vector<unsigned char> img_dict_signature;


    std::ifstream image_file(input_image_path_string, std::ios::binary);
    if (image_file) {
        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                            std::istreambuf_iterator<char>());

        int widthImage, heightImage, channels;
        std::vector<uint8_t> imageData = adns.decodePNG(input_image_path_string, widthImage, heightImage, channels);



        Visualization vzsl;
        std::vector<double> positions = vzsl.get_position(pdf_content, (page-1), character);
        // std::vector<uint8_t> visualization = vzsl.get_image(image_data);
        cv::Mat img = cv::imdecode(image_data, cv::IMREAD_UNCHANGED);
        std::vector<uint8_t> visualization = adns.process_image(img);
        std::string visualization_string(visualization.begin(), visualization.end());
        // std::cout << visualization_string << std::endl;

        // Jika menggunnakan tanda #
        int x = positions[0];
        int y = positions[1];

        float rectWidth;
        float rectHeight;
        if (width/height > widthImage/heightImage)
        {
            rectHeight = height;
            rectWidth = rectHeight/heightImage*widthImage;
            x = x + ((width-rectWidth)/2);
        } else{
            rectWidth = width;
            rectHeight = (rectWidth/widthImage)*heightImage;
            y = y + ((height-rectHeight)/2);
        }

        // Jika menggunnakan tanda #
        x = positions[0] - (rectWidth/2);
        y = positions[1] - (rectHeight/2);
        

        annot_dict = object_annot_start + "/Type /Annot\n/Subtype /Widget\n/FT /Sig\n/Rect [" + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(x + rectWidth) + " " + std::to_string(y+rectHeight) +  "]\n/AP <<\n/N " + std::to_string(max_index + 5) + " 0 R\n>>\n/V " + std::to_string(max_index + 1) + " 0 R\n/T (Signature_" + std::to_string(milliseconds) + ")\n/F 4\n/P " + object_page_ref_annot + object_annot_end;
        

        float widthSign = static_cast<float>(widthImage) / (widthImage/rectWidth); // Convert numerator to float before division
        float heightSign = static_cast<float>(heightImage) / (heightImage/rectHeight); // Convert numerator to float before division

        // Copy the contents into a vector
        std::vector<unsigned char> vec(visualization_string.begin(), visualization_string.end());

        std::string img_obj_0_start = "\n\n" + std::to_string(max_index + 5) + " 0 obj\n";
        std::string img_obj_0_end = "\nendobj";
        std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 "+std::to_string(heightSign)+" 0 0 cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        // std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 -"+std::to_string(heightSign)+" 0 "+std::to_string(heightSign)+" cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        std::string img_obj_0_dict = img_obj_0_start + "<<\n/Length " + std::to_string(stream_obj_0.size()) + "\n/Type /XObject\n/Subtype /Form\n/Resources <<\n/XObject <<\n/X" + std::to_string(milliseconds) + " " + std::to_string(max_index + 6) + " 0 R\n>>\n>>\n/FormType 1\n/BBox [0.0 0.0 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+"]\n/Matrix [1 0 0 1 0 0]\n>>\nstream\n" + stream_obj_0 + "\nendstream" + img_obj_0_end;
        std::string img_obj_1_start = "\n\n" + std::to_string(max_index + 6) + " 0 obj\n<<\n";
        std::string img_obj_1_end = "\nendobj";
        std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Filter /DCTDecode\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        // std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        std::string img_obj_1_dict_2 = "\nendstream" + img_obj_1_end;
        std::vector<unsigned char> img_obj_final_vec;
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_0_dict.begin(), img_obj_0_dict.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_1.begin(), img_obj_1_dict_1.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), vec.begin(), vec.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_2.begin(), img_obj_1_dict_2.end());

        img_dict_signature = img_obj_final_vec;

    } else {
        std::cerr << "Error reading from the input image file" << std::endl;
    }

    std::string object_acroform_start = "\n\n" + std::to_string(max_index + 3) + " 0 obj\n<<\n";
    std::string object_acroform_end = "\n>>\nendobj";
    std::string acroform_dict = object_acroform_start + "/Type /AcroForm\n/SigFlags 3\n/Fields [" + std::to_string(max_index + 2) + " 0 R]" + object_acroform_end;
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        acroform_dict = "\n\n" + check_sign.at("acroform_last_start") + " " + std::to_string(max_index + 2) + " 0 R" + check_sign.at("acroform_last_end");
    }
    // std::string object__start = "\n\n" + std::to_string(max_index + 4) + " 0 obj\n";
    // std::string object__end = "\nendobj";
    // std::string _dict = object__start + "undefined" + object__end;

    std::string page_dict = adns.get_page(pdf_content, object_page_ref_annot);
    std::unordered_map<std::string, std::string> annots_on_page = adns.checkAnnotOnPage(page_dict);

    std::unordered_map<std::string, std::string> read_pdf_dict = read_pdf;
    std::string catalog_dict = adns.get_catalog(pdf_content, read_pdf_dict);
    std::string root_obj = "\n" + catalog_dict.substr(0, catalog_dict.length() - 4) + "\n/AcroForm " + std::to_string(max_index + 3) + " 0 R\n>>\nendobj";
    std::vector<unsigned char> root_obj_vec(root_obj.begin(), root_obj.end());


    std::string page_obj = "\n" + page_dict.substr(0, page_dict.length() - 4) + "/Annots [" + std::to_string(max_index + 2) + " 0 R]\n\n>>\nendobj";
    if (annots_on_page.at("status") == "exist") {
        page_obj = "\n" + annots_on_page.at("page_before") + " " + std::to_string(max_index + 2) + " 0 R" + annots_on_page.at("page_after") + "\nendobj";
    }


    std::vector<unsigned char> page_obj_vec(page_obj.begin(), page_obj.end());


    size_t rootIndex = adns.extract_integer(read_pdf.at("rootRef"));
    size_t pageIndex = adns.extract_integer(page_dict.substr(0, 10));

    std::vector<size_t> x_list;
    result.insert(result.end(), pdf_content.begin(), pdf_content.end());
    result.insert(result.end(), signature_dict.begin(), signature_dict.end());
    x_list.push_back(max_index + 1);
    result.insert(result.end(), annot_dict.begin(), annot_dict.end());
    x_list.push_back(max_index + 2);
    result.insert(result.end(), acroform_dict.begin(), acroform_dict.end());
    x_list.push_back(max_index + 3);
    result.insert(result.end(), _dict.begin(), _dict.end());
    x_list.push_back(max_index + 4);
    result.insert(result.end(), img_dict_signature.begin(), img_dict_signature.end());
    x_list.push_back(max_index + 5);
    x_list.push_back(max_index + 6);  // XObject
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    } else {
        result.insert(result.end(), root_obj_vec.begin(), root_obj_vec.end());
        x_list.push_back(rootIndex);
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    }

    std::sort(x_list.begin(), x_list.end());
    std::string added_index_string = adns.vector_size_t_to_string(x_list);
    std::vector<uint8_t> added_index = adns.stringToVector(added_index_string);

    // Populate the map with data
    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    myMap["added_dict"] = result;
    myMap["catalog_dict"] = root_obj_vec;
    myMap["added_index"] = added_index;

    return myMap;
}

std::unordered_map<std::string, std::vector<uint8_t> > AddSignatureDict::add_signaturedict_qr_to_char(std::vector<unsigned char> pdf_content, const char* character, const char* url, int page, float width, float height, std::vector<std::string> object_page_ref, std::unordered_map<std::string, std::string> read_pdf, std::unordered_map<std::string, std::string> check_sign) {

    Addons adns;

    size_t max_index = 0;
    if (read_pdf.count("maxIndex") > 0) {
        try {
            size_t number = std::stoull(read_pdf.at("maxIndex"));
            if (number <= std::numeric_limits<size_t>::max()) {
                max_index = number;
            } else {
                std::cout << "Parsed number is too large to fit into size_t" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to parse the string as a number" << std::endl;
        }
    }
    std::chrono::system_clock::time_point utc_now = std::chrono::system_clock::now();
    std::chrono::seconds formatted_timestamp = std::chrono::duration_cast<std::chrono::seconds>(utc_now.time_since_epoch());
    std::time_t timestamp_time_t = formatted_timestamp.count();
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&timestamp_time_t), "%Y%m%d%H%M%S");
    std::string formatted_timestamp_with_z = ss.str() + "Z";
    std::vector<unsigned char> result;
    std::string object_signature_start = "\n" + std::to_string(max_index + 1) + " 0 obj\n<<\n";
    std::string contents = "/Contents <" + std::string(35000, '0') + ">";
    std::string object_signature_end = "\n>>\nendobj";
    // std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /adbe.pkcs7.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string signature_dict = object_signature_start + "/Type /Sig\n/Filter /Adobe.PPKLite\n/SubFilter /ETSI.CAdES.detached\n/ByteRange [0 /********** /********** /**********]\n" + contents + "\n/Reason (Signed Certificate.)\n/M (D:" + formatted_timestamp_with_z + ")\n/ContactInfo (sign@example.com)\n/Name (Example)\n/Location (Jakarta)" + object_signature_end;
    std::string object_annot_start = "\n\n" + std::to_string(max_index + 2) + " 0 obj\n<<\n";
    std::string object_annot_end = "\n>>\nendobj";
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration_since_epoch = current_time.time_since_epoch();
    std::chrono::milliseconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count();
    std::string object_page_ref_annot = object_page_ref[page - 1];
    std::string annot_dict;
    std::vector<unsigned char> img_dict_signature;


    std::string text = url;
    std::string filename = std::to_string(milliseconds)+".jpg";
    std::vector<unsigned char> image_char = adns.saveQRCodeAsJPG(text, filename);
    // std::vector<unsigned char> image_char = adns.saveQRCodeAsJPG_2(text);

    std::ifstream image_file(filename.c_str(), std::ios::binary);
    if (image_file) {
        std::remove(filename.c_str());


        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                            std::istreambuf_iterator<char>());

        int widthImage, heightImage, channels;
        stbi_load_from_memory(image_char.data(), image_char.size(), &widthImage, &heightImage, &channels, 0);



        Visualization vzsl;
        // std::vector<uint8_t> visualization = vzsl.get_image(image_data);
        cv::Mat img = cv::imdecode(image_data, cv::IMREAD_UNCHANGED);
        std::vector<uint8_t> visualization = adns.process_image(img);
        std::string visualization_string(visualization.begin(), visualization.end());
        std::vector<double> positions = vzsl.get_position(pdf_content, (page-1), character);


        // Jika menggunnakan tanda #
        int x = positions[0];
        int y = positions[1];

        float rectWidth;
        float rectHeight;
        if (width/height > widthImage/heightImage)
        {
            rectHeight = height;
            rectWidth = rectHeight/heightImage*widthImage;
            x = x + ((width-rectWidth)/2);
        } else{
            rectWidth = width;
            rectHeight = (rectWidth/widthImage)*heightImage;
            y = y + ((height-rectHeight)/2);
        }

        // Jika menggunnakan tanda #
        x = positions[0] - (rectWidth/2);
        y = positions[1] - (rectHeight/2);
        

        annot_dict = object_annot_start + "/Type /Annot\n/Subtype /Widget\n/FT /Sig\n/Rect [" + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(x + rectWidth) + " " + std::to_string(y+rectHeight) +  "]\n/AP <<\n/N " + std::to_string(max_index + 5) + " 0 R\n>>\n/V " + std::to_string(max_index + 1) + " 0 R\n/T (Signature_" + std::to_string(milliseconds) + ")\n/F 4\n/P " + object_page_ref_annot + object_annot_end;
        

        float widthSign = static_cast<float>(widthImage) / (widthImage/rectWidth); // Convert numerator to float before division
        float heightSign = static_cast<float>(heightImage) / (heightImage/rectHeight); // Convert numerator to float before division

        // Copy the contents into a vector
        std::vector<unsigned char> vec(visualization_string.begin(), visualization_string.end());

        std::string img_obj_0_start = "\n\n" + std::to_string(max_index + 5) + " 0 obj\n";
        std::string img_obj_0_end = "\nendobj";
        std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 "+std::to_string(heightSign)+" 0 0 cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        // std::string stream_obj_0 = "q\nq\n"+std::to_string(widthSign)+" 0 0 -"+std::to_string(heightSign)+" 0 "+std::to_string(heightSign)+" cm\n/X" + std::to_string(milliseconds) + " Do\nQ\n1 0 0 1 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+" cm\nQ\n";
        std::string img_obj_0_dict = img_obj_0_start + "<<\n/Length " + std::to_string(stream_obj_0.size()) + "\n/Type /XObject\n/Subtype /Form\n/Resources <<\n/XObject <<\n/X" + std::to_string(milliseconds) + " " + std::to_string(max_index + 6) + " 0 R\n>>\n>>\n/FormType 1\n/BBox [0.0 0.0 "+std::to_string(widthSign)+" "+std::to_string(heightSign)+"]\n/Matrix [1 0 0 1 0 0]\n>>\nstream\n" + stream_obj_0 + "\nendstream" + img_obj_0_end;
        std::string img_obj_1_start = "\n\n" + std::to_string(max_index + 6) + " 0 obj\n<<\n";
        std::string img_obj_1_end = "\nendobj";
        std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Filter /DCTDecode\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(vec.size()) + "\n>>\nstream\n";
        // std::string img_obj_1_dict_1 = img_obj_1_start + "/Type /XObject\n/Subtype /Image\n/Width " + std::to_string(widthImage) + "\n/Height " + std::to_string(heightImage) + "\n/ColorSpace /DeviceRGB\n/BitsPerComponent 8\n/Length " + std::to_string(imageData.size()) + "\n>>\nstream\n";
        std::string img_obj_1_dict_2 = "\nendstream" + img_obj_1_end;
        std::vector<unsigned char> img_obj_final_vec;
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_0_dict.begin(), img_obj_0_dict.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_1.begin(), img_obj_1_dict_1.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), vec.begin(), vec.end());
        img_obj_final_vec.insert(img_obj_final_vec.end(), img_obj_1_dict_2.begin(), img_obj_1_dict_2.end());

        img_dict_signature = img_obj_final_vec;

    } else {
        std::remove(filename.c_str());
        std::cerr << "Error reading from the input image file" << std::endl;
    }
    
    std::string object_acroform_start = "\n\n" + std::to_string(max_index + 3) + " 0 obj\n<<\n";
    std::string object_acroform_end = "\n>>\nendobj";
    std::string acroform_dict = object_acroform_start + "/Type /AcroForm\n/SigFlags 3\n/Fields [" + std::to_string(max_index + 2) + " 0 R]" + object_acroform_end;
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        acroform_dict = "\n\n" + check_sign.at("acroform_last_start") + " " + std::to_string(max_index + 2) + " 0 R" + check_sign.at("acroform_last_end");
    }
    // std::string object__start = "\n\n" + std::to_string(max_index + 4) + " 0 obj\n";
    // std::string object__end = "\nendobj";
    // std::string _dict = object__start + "undefined" + object__end;

    std::string page_dict = adns.get_page(pdf_content, object_page_ref_annot);
    std::unordered_map<std::string, std::string> annots_on_page = adns.checkAnnotOnPage(page_dict);

    std::unordered_map<std::string, std::string> read_pdf_dict = read_pdf;
    std::string catalog_dict = adns.get_catalog(pdf_content, read_pdf_dict);
    std::string root_obj = "\n" + catalog_dict.substr(0, catalog_dict.length() - 4) + "\n/AcroForm " + std::to_string(max_index + 3) + " 0 R\n>>\nendobj";
    std::vector<unsigned char> root_obj_vec(root_obj.begin(), root_obj.end());


    std::string page_obj = "\n" + page_dict.substr(0, page_dict.length() - 4) + "/Annots [" + std::to_string(max_index + 2) + " 0 R]\n\n>>\nendobj";
    if (annots_on_page.at("status") == "exist") {
        page_obj = "\n" + annots_on_page.at("page_before") + " " + std::to_string(max_index + 2) + " 0 R" + annots_on_page.at("page_after") + "\nendobj";
    }


    std::vector<unsigned char> page_obj_vec(page_obj.begin(), page_obj.end());


    size_t rootIndex = adns.extract_integer(read_pdf.at("rootRef"));
    size_t pageIndex = adns.extract_integer(page_dict.substr(0, 10));

    std::vector<size_t> x_list;
    result.insert(result.end(), pdf_content.begin(), pdf_content.end());
    result.insert(result.end(), signature_dict.begin(), signature_dict.end());
    x_list.push_back(max_index + 1);
    result.insert(result.end(), annot_dict.begin(), annot_dict.end());
    x_list.push_back(max_index + 2);
    result.insert(result.end(), acroform_dict.begin(), acroform_dict.end());
    x_list.push_back(max_index + 3);
    result.insert(result.end(), _dict.begin(), _dict.end());
    x_list.push_back(max_index + 4);
    result.insert(result.end(), img_dict_signature.begin(), img_dict_signature.end());
    x_list.push_back(max_index + 5);
    x_list.push_back(max_index + 6);  // XObject
    if (check_sign.count("check") > 0 && check_sign.at("check") == "exist") {
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    } else {
        result.insert(result.end(), root_obj_vec.begin(), root_obj_vec.end());
        x_list.push_back(rootIndex);
        result.insert(result.end(), page_obj_vec.begin(), page_obj_vec.end());
        x_list.push_back(pageIndex);
    }

    std::sort(x_list.begin(), x_list.end());
    std::string added_index_string = adns.vector_size_t_to_string(x_list);
    std::vector<uint8_t> added_index = adns.stringToVector(added_index_string);

    // Populate the map with data
    std::unordered_map<std::string, std::vector<uint8_t> > myMap;
    myMap["added_dict"] = result;
    myMap["catalog_dict"] = root_obj_vec;
    myMap["added_index"] = added_index;

    return myMap;
}