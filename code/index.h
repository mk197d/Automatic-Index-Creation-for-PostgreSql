#ifndef INDEX_H
#define INDEX_H
#include "common-headers.h"
#include "keywords.h"

extern std::map<std::string, KeywordType> keyword_map;
extern std::map<std::string, std::map<std::string, int>> count_of_num_accesses;
void indexCreation(std::string const & query);
void showNumAccesses();

#endif