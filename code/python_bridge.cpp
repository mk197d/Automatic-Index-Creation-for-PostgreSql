#include "python_bridge.h"
#include "memory.h"
#include "index.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string callPythonParser(const std::string& query) {
    // Create a temporary file to store the query
    char tmpFilename[] = "/tmp/query_XXXXXX";
    int fd = mkstemp(tmpFilename);
    
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    
    // Write the query to the temporary file
    FILE* tmpFile = fdopen(fd, "w");
    
    fprintf(tmpFile, "%s", query.c_str());
    fclose(tmpFile);
    
    // Create the command to run the Python script
    std::string cmd = "python3 query_parser.py \"" + 
                      query + "\" 2>/dev/null";
    
    // Execute the command and capture the output
    std::array<char, 1024> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    
    if (!pipe) {
        remove(tmpFilename);
        throw std::runtime_error("Failed to run Python parser");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    // Clean up the temporary file
    remove(tmpFilename);
    
    return result;
}

void updateAccessCounts(const std::string& query) {
    try {
        // Call the Python parser to get table and attribute information
        std::string jsonResult = callPythonParser(query);
        
        // Parse the JSON output
        auto parsedData = json::parse(jsonResult);
        
        // Check if there was an error in parsing
        if (parsedData.contains("error")) {
            std::cerr << "Error from Python parser: " << parsedData["error"] << std::endl;
            return;
        }
        
        // Print debug information
        std::cout << "Parsed tables and attributes:" << std::endl;
        
        // Update the count_of_num_accesses map based on the parsed data
        for (auto& [tableName, attributes] : parsedData.items()) {
            std::cout << "Table: " << tableName << std::endl;
            std::cout << "Attributes: ";
            
            for (const auto& attr : attributes) {
                std::string attrName = attr.get<std::string>();
                count_of_num_accesses[tableName][attrName]++;
                std::cout << attrName << " ";
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error updating access counts: " << e.what() << std::endl;
    }
}