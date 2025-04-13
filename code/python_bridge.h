#ifndef PYTHON_BRIDGE_H
#define PYTHON_BRIDGE_H

#include "common-headers.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

/**
 * Executes a Python script with the given SQL query and returns the parsed result
 * 
 * @param query The SQL query to parse
 * @return JSON string with table and attribute information
 */
std::string callPythonParser(const std::string& query);

/**
 * Updates the count_of_num_accesses map based on Python parser output
 * 
 * @param query The SQL query to analyze
 */
void updateAccessCounts(const std::string& query);

#endif // PYTHON_BRIDGE_H