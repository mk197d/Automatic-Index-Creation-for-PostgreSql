#include "memory.h" 

int current_timestamp = 0;
std::map<std::string, std::map<std::string, int>> count_of_num_accesses;


IndexEntry::IndexEntry(const std::string& tName){
    this->tableName = tName;
    this->createTime = current_timestamp;
    this->num_accesses = 1;
}

IndexEntry::IndexEntry(const std::string& tName, std::set<std::string>* const attributes){
    this->tableName = tName;
    this->setOfAttributes = attributes;

}

bool comp(IndexEntry* const a, IndexEntry* const b) {
    return a->createTime < b->createTime;
}

// std::set<IndexEntry*,comp> indices;
