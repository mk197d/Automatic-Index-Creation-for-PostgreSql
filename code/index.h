#ifndef INDEX_H
#define INDEX_H
#include "common-headers.h"
#include "keywords.h"

extern std::map<std::string, KeywordType> keyword_map;

void indexCreation(std::string const & query);

#endif