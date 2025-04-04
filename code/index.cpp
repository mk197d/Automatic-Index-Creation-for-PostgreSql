#include "index.h"


void indexCreation(std::string const & query) {
    std::string tableName;
    std::string attributeListStr;
    std::vector<std::string> attributes;

    std::istringstream iss(query);
    std::string token;

    iss >> token; 
    
    while (iss >> token && token != "FROM") {
        if (!attributeListStr.empty()) {
            attributeListStr += " ";
        }
        attributeListStr += token;
    }

    iss >> tableName;

    std::istringstream attrStream(attributeListStr);
    std::string attr;
    while (std::getline(attrStream, attr, ',')) {
        size_t start = attr.find_first_not_of(" \t");
        size_t end = attr.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            attributes.push_back(attr.substr(start, end - start + 1));
        } else if (!attr.empty()) {
            attributes.push_back(attr);
        }
    }

    std::cout << "Table name: " << tableName << std::endl;
    std::cout << "Attributes: ";
    for (const auto& a : attributes) {
        std::cout << a << " ";
    }
    std::cout << std::endl;
}
