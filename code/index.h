#ifndef INDEX_H
#define INDEX_H
#include "common-headers.h"
#include "keywords.h"
#include "fifo.hh"
extern std::map<std::string, KeywordType> keyword_map;
extern std::map<std::string, std::map<std::string, int>> count_of_num_accesses;
void indexCreation(pqxx::connection &C, std::string const & query);
bool attributeExists(pqxx::connection &C, const std::string &relationName, const std::string &attributeName);
void showNumAccesses();

#endif