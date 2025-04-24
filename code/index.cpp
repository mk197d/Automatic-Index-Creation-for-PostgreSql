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

void indexCreation(pqxx::connection &C, std::string const & query) {
    std::ofstream tempQueryFile("tempQuery.sql");
    if (tempQueryFile.is_open()) {
        tempQueryFile << query;
        tempQueryFile.close();
    } else {
        std::cerr << "Failed to open tempQuery.sql for writing." << std::endl;
        return;
    }

    int result = std::system("python3 query_parser.py tempQuery.sql > tempParse.txt");
    if (result != 0) {
        std::cerr << "Failed to execute query_parser.py." << std::endl;
        return;
    }

    std::ifstream tempParseFile("tempParse.txt");
    if (!tempParseFile.is_open()) {
        std::cerr << "Failed to open tempParse.txt for reading." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(tempParseFile, line)) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            std::cerr << "Invalid line format: " << line << std::endl;
            continue;
        }

        std::string tableName = line.substr(0, colonPos);
        tableName.erase(std::remove(tableName.begin(), tableName.end(), ' '), tableName.end());

        size_t startBracket = line.find('[', colonPos);
        size_t endBracket = line.find(']', startBracket);
        if (startBracket == std::string::npos || endBracket == std::string::npos) {
            std::cerr << "Invalid attribute list format: " << line << std::endl;
            continue;
        }

        std::string attributesStr = line.substr(startBracket + 1, endBracket - startBracket - 1);
        std::istringstream attrStream(attributesStr);
        std::string attribute;
        while (std::getline(attrStream, attribute, ',')) {            
            attribute.erase(std::remove(attribute.begin(), attribute.end(), '\''), attribute.end());
            attribute.erase(std::remove(attribute.begin(), attribute.end(), ' '), attribute.end());

            if(tableName != attribute && attributeExists(C, tableName, attribute)) count_of_num_accesses[tableName][attribute]++;
        }
    }

    tempParseFile.close();
}

void showNumAccesses()
{
    for (auto & tableName: count_of_num_accesses)
    {
        std::cout << "Table name: " << tableName.first << std::endl;
        for (const auto& a : tableName.second) {
            std::cout << a.first << " " << a.second << std::endl;
        }
        std::cout << "\n";
    }
}

bool attributeExists(pqxx::connection &C, const std::string &relationName, const std::string &attributeName) {
    try {
        // Query the information_schema.columns table to check if the attribute exists
        std::string query = "SELECT COUNT(*) FROM information_schema.columns "
                           "WHERE table_name = '" + relationName + "' "
                           "AND column_name = '" + attributeName + "'";
        
        pqxx::nontransaction N(C);
        pqxx::result R = N.exec(query);
        
        // If the result has at least one row and the count is greater than 0, the attribute exists
        if (!R.empty() && R[0][0].as<int>() > 0) {
            return true;
        }
        return false;
    } catch (const std::exception &e) {
        std::cerr << "Error checking attribute existence: " << e.what() << std::endl;
        return false;
    }
}