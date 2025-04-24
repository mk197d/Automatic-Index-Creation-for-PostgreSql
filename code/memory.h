#ifndef MEMORY_H
#define MEMORY_H
#include "common-headers.h"
#include "keywords.h"
extern int current_timestamp;
extern std::set<IndexEntry*> indices;

class IndexEntry {
    public:
    std::string tableName;
    std::string attributeName;
    std::set<std::string>* setOfAttributes;
    std::string indexName;
    int createTime;
    int num_accesses;

    IndexEntry(const std::string&);
    IndexEntry(const std::string&, std::set<std::string>* const);
};

bool indexExists(const std::string& tName,const std::set<std::string>* const attributes){
    for (auto entry : indices ){
        if (entry->tableName == tName) {
            if (*(entry->setOfAttributes) == *attributes){
                return true;
            }
        }
    }    
}
#endif