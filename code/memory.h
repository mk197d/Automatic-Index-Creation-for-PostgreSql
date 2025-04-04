#ifndef MEMORY_H
#define MEMORY_H
#include "common-headers.h"
#include "keywords.h"


struct IndexEntry {
    std::string tableName;
    std::string attributeName;
    std::string indexName;
    int createTime;
    int num_accesses;
};

#endif