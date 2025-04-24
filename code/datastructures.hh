#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "common-headers.h"
extern int current_timestamp;

class IndexEntry {
    private:
    int createTime;
    int numAccesses;
    public:
    std::string tableName;
    std::string attributeName;
    std::set<std::string>* setOfAttributes;
    std::string indexName;
    /*
        Modify these two constructors to create an index name from tableName and current_timestamp
    */

    IndexEntry(const std::string&);
    IndexEntry(const std::string&, std::set<std::string>* const);
    bool operator<(const IndexEntry& other);
    void setCreateTime(const int&);
    int getCreateTime();
    void setNumOfAccesses(const int&);
    int getNumOfAccesses();
};

bool indexExists(const std::string&, const std::set<std::string>* const);
void updateIndexEntry(const std::string& ,std::set<std::string>* const);
extern std::set<IndexEntry*> indices;
#endif