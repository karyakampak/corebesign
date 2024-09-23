#include "../header/add_placeholder.h"
#include "../header/open_file.h"
#include "../header/get_pdf_component.h"
#include "../header/get_page_reference.h"
#include "../header/check_signature.h"
#include "../header/add_signature_dict.h"
#include "../header/add_seal_dict.h"
#include "../header/get_added_index.h"
#include "../header/add_trailer.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

AddPlaceHolder::AddPlaceHolder() {
    // Initialize private member variables or perform any necessary setup
}

std::unordered_map<std::string, std::string> AddPlaceHolder::addPlaceholder(int type, const char* pdf_path, int isProtected, const char* character, const char* imageorurl, int page, float x, float y, float width, float height, int isSeal) {

    OpenFile opf;
    std::vector<uint8_t> pdfContent = opf.open(pdf_path, isProtected);

    GetPdfComponent rdpdf;
    std::unordered_map<std::string, std::string> pdf_component = rdpdf.getPdfComponent(pdfContent);
    // Cek output dari fungsi GetPdfComponent
    // std::cout << "\n" << std::endl;
    // std::unordered_map<std::string, std::string>::iterator it;
    // for (it = pdf_component.begin(); it != pdf_component.end(); ++it) {
    //     std::cout << "Key ReadPdf: " << it->first << ", Value: " << it->second << std::endl;
    // }

    GetPageReference gpr;
    std::vector<std::string> page_reference = gpr.getPage(pdfContent, pdf_component);
    // Cek output dari fungsi GetPageReference
    // std::cout << "\n" << std::endl;
    // for (std::vector<std::string>::const_iterator it = page_reference.begin(); it != page_reference.end(); ++it) {
    //     std::cout << *it << std::endl;
    // }

    CheckSignature cksig;
    std::unordered_map<std::string, std::string> signature_reference = cksig.check_signature(pdfContent);
    // Cek output dari fungsi CheckSignature
    // std::cout << "\n" << std::endl;
    // for (it = signature_reference.begin(); it != signature_reference.end(); ++it) {
    //     std::cout << "Key CheckSignature: " << it->first << ", Value: " << it->second << std::endl;
    // }

    AddSignatureDict adsig;
    AddSealDict adseal;

    std::unordered_map<std::string, std::vector<uint8_t> > signature_object_added;
    if(isSeal){
        if(type == 1){
            signature_object_added = adseal.add_sealdict_with_image(pdfContent, imageorurl, page, x, y, width, height, page_reference, pdf_component, signature_reference);
        } else if (type == 2) {
            signature_object_added = adseal.add_sealdict_with_qr(pdfContent, imageorurl, page, x, y, width, height, page_reference, pdf_component, signature_reference);
        }else if (type == 3) {
            signature_object_added = adseal.add_sealdict_image_to_char(pdfContent, character, imageorurl, page, width, height, page_reference, pdf_component, signature_reference);
        }else if (type == 4) {
            signature_object_added = adseal.add_sealdict_qr_to_char(pdfContent, character, imageorurl, page, width, height, page_reference, pdf_component, signature_reference);
        } else {
            signature_object_added = adseal.add_sealdict(pdfContent, page, page_reference, pdf_component, signature_reference);
        }
    } else{
        if(type == 1){
            signature_object_added = adsig.add_signaturedict_with_image(pdfContent, imageorurl, page, x, y, width, height, page_reference, pdf_component, signature_reference);
        } else if (type == 2) {
            signature_object_added = adsig.add_signaturedict_with_qr(pdfContent, imageorurl, page, x, y, width, height, page_reference, pdf_component, signature_reference);
        }else if (type == 3) {
            signature_object_added = adsig.add_signaturedict_image_to_char(pdfContent, character, imageorurl, page, width, height, page_reference, pdf_component, signature_reference);
        }else if (type == 4) {
            signature_object_added = adsig.add_signaturedict_qr_to_char(pdfContent, character, imageorurl, page, width, height, page_reference, pdf_component, signature_reference);
        } else {
            signature_object_added = adsig.add_signaturedict(pdfContent, page, page_reference, pdf_component, signature_reference);
        }
    }



    

    GetAddedIndex getaddin;
    std::vector<size_t> added_index = getaddin.get_index_after_add_dict(signature_object_added.at("added_index"));
    // Cek output dari fungsi GetAddedIndex
    // std::cout << "\n" << std::endl;
    // for (std::vector<size_t>::const_iterator it = added_index.begin(); it != added_index.end(); ++it) {
    //     std::cout << "New Object Index " << *it << std::endl;
    // }

    AddTrailer adtrl;
    std::unordered_map<std::string, std::string> placedHolder = adtrl.add_trailer(pdfContent, signature_object_added.at("added_dict"), added_index, pdf_component);

    std::string catalog_dict(signature_object_added.at("catalog_dict").begin(), signature_object_added.at("catalog_dict").end());
    

    std::unordered_map<std::string, std::string> rslt;
    
    rslt["catalog_dict"] = catalog_dict;

    rslt["placedHolder"] = placedHolder.at("placedHolder");
    rslt["pdf_component_size"] = placedHolder.at("pdf_component_size");
    rslt["pdf_component_root"] = placedHolder.at("pdf_component_root");
    rslt["pdf_component_info"] = placedHolder.at("pdf_component_info");
    rslt["pdf_component_id"] = placedHolder.at("pdf_component_id");
    rslt["pdf_component_prev"] = placedHolder.at("pdf_component_prev");

    return rslt;

}