#include "index.h"

void indexCreation(std::string& query) {
    std::string tableName;
    std::string indexName;
    std::string columnName;

    // Parse the query to extract table name, index name, and column name
    std::istringstream iss(query);
    std::string token;

    // Skip "CREATE INDEX"
    iss >> token >> token;

    // Get table name
    iss >> tableName;

    // Skip "("
    iss >> token;

    // Get column name
    iss >> columnName;

    // Skip ")"
    iss >> token;

    std::cout << "Creating index " << indexName << " on table " << tableName << " for column " << columnName << std::endl;
}