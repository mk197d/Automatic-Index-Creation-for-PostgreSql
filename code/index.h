#ifndef INDEX_H
#define INDEX_H
#include "keywords.h"
#include "datastructures.hh"
#define THRESHOLD1 10
#define THRESHOLD2 60
typedef std::pair<std::string,std::string> pair_t;
typedef std::map<pair_t,int> matrix_t;
extern std::map<std::string, KeywordType> keyword_map;
extern std::map<std::string, std::map<std::string, int>> count_of_num_accesses;
enum class POLICY{
    P1,
    P2
};
extern POLICY p;
void indexCreation(pqxx::work&, std::string const& );
void updateMap(const std::string&, const std::vector<std::string>& );
void scanMap(pqxx::work&, std::string const&);
bool attributeExists(pqxx::work&, const std::string&, const std::string&);
void showNumAccesses();
void clearIndices(pqxx::work&);
void fork_a_child_for_index(const std::string&, std::set<std::string>* const , pqxx::work&);
void printRowCounts(pqxx::work& txn);
double get_hypo_cost(std::string query, std::string tableName, std::set<std::string>* const atrs);
#endif