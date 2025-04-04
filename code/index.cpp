#include "index.h"

std::map<std::string, KeywordType> keyword_map = {
    {"SELECT", KeywordType::SELECT},
    {"INSERT", KeywordType::INSERT},
    {"UPDATE", KeywordType::UPDATE},
    {"DELETE", KeywordType::DELETE},
    {"CREATE", KeywordType::CREATE},
    {"DROP", KeywordType::DROP},
    {"ALTER", KeywordType::ALTER},
    {"TABLE", KeywordType::TABLE},
    {"DATABASE", KeywordType::DATABASE},
    {"INDEX", KeywordType::INDEX},
    {"VIEW", KeywordType::VIEW},
    {"TRIGGER", KeywordType::TRIGGER},
    {"FUNCTION", KeywordType::FUNCTION},
    {"PROCEDURE", KeywordType::PROCEDURE},
    {"SCHEMA", KeywordType::SCHEMA},
    {"GRANT", KeywordType::GRANT},
    {"REVOKE", KeywordType::REVOKE},
    {"COMMIT", KeywordType::COMMIT},
    {"ROLLBACK", KeywordType::ROLLBACK},
    {"BEGIN", KeywordType::BEGIN},
    {"END", KeywordType::END},
    {"IF", KeywordType::IF},
    {"ELSE", KeywordType::ELSE_k},
    {"CASE", KeywordType::CASE},
    {"WHEN", KeywordType::WHEN},
    {"THEN", KeywordType::THEN},
    {"ELSEIF", KeywordType::ELSEIF},
    {"LOOP", KeywordType::LOOP},
    {"WHILE", KeywordType::WHILE},
    {"FOR", KeywordType::FOR},
    {"DO", KeywordType::DO},
    {"RETURN", KeywordType::RETURN_k},
    {"WHERE", KeywordType::WHERE},
    {"FROM", KeywordType::FROM},
    {"INTO", KeywordType::INTO},
    {"VALUES", KeywordType::VALUES},
    {"SET", KeywordType::SET},
    {"AS", KeywordType::AS},
    {"ON", KeywordType::ON},
    {"JOIN", KeywordType::JOIN},
    {"INNER JOIN", KeywordType::INNER_JOIN},
    {"LEFT JOIN", KeywordType::LEFT_JOIN}, 
    {"RIGHT JOIN", KeywordType::RIGHT_JOIN},
    {"FULL JOIN", KeywordType::FULL_JOIN},
    {"CROSS JOIN", KeywordType::CROSS_JOIN},
    {"NATURAL JOIN", KeywordType::NATURAL_JOIN},
    {"EXISTS", KeywordType::EXISTS},
    {"ALL", KeywordType::ALL},
    {"ANY", KeywordType::ANY},
    {"SOME", KeywordType::SOME},
    {"DISTINCT", KeywordType::DISTINCT},
    {"GROUP BY", KeywordType::GROUP_BY},
    {"ORDER BY", KeywordType::ORDER_BY},
    {"HAVING", KeywordType::HAVING},
    {"LIMIT", KeywordType::LIMIT},
    {"OFFSET", KeywordType::OFFSET}
};


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
