#ifndef MEMORY_H
#define MEMORY_H
#include "common-headers.h"
#include "keywords.h"
extern int current_timestamp;

struct IndexEntry {
    std::string tableName;
    std::string attributeName;
    std::string indexName;
    int createTime;
    int num_accesses;
};

#endif