#ifndef INDEX_H
#define INDEX_H
#include "keywords.h"
#include "datastructures.hh"
extern std::map<std::string, KeywordType> keyword_map;
extern std::map<std::string, std::map<std::string, int>> count_of_num_accesses;
enum class POLICY{
    P1,
    P2
};
extern POLICY p;
void indexCreation(pqxx::work &C, std::string const & query);
bool attributeExists(pqxx::work& txn, const std::string &relationName, const std::string &attributeName);
void showNumAccesses();
void clearIndices(pqxx::work&);
#endif