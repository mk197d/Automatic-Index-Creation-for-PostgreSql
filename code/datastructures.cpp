#include "datastructures.hh" 

int current_timestamp = 0;
std::map<std::string, std::map<std::string, int>> count_of_num_accesses;
std::vector<IndexEntry*> indices;
std::set<pid_t> existing_child_processes;

IndexEntry::IndexEntry(const std::string& tName){
    this->tableName = tName;
    this->indexName = tName + std::to_string(current_timestamp);
    this->createTime = current_timestamp;
    this->numAccesses = 1;
}

IndexEntry::IndexEntry(const std::string& tName, std::set<std::string>* const attributes){
    this->tableName = tName;
    this->indexName = tName + std::to_string(current_timestamp);
    this->setOfAttributes = attributes;
    this->createTime = current_timestamp;
    this->numAccesses = 1;
}

void IndexEntry::setCreateTime(const int& t){
    this->createTime = t;
}

int IndexEntry::getCreateTime(){
    return this->createTime;
}

void IndexEntry::setNumOfAccesses(const int& n){
    this->numAccesses = n;
}

int IndexEntry::getNumOfAccesses(){
    return this->numAccesses;
}

bool IndexEntry::operator<(const IndexEntry& other) {
    return this->createTime < other.createTime;
}

std::ostream& operator<<(std::ostream& out, const IndexEntry& other){
    out << other.tableName << " " << other.indexName << " " << other.createTime << " " << other.numAccesses << "Attributes ";
    for (auto it : *(other.setOfAttributes)){
        out << it << " ";
    } 
    return out;
}

bool indexExists(const std::string& tName,const std::set<std::string>* const attributes){
    for (auto entry : indices ){
        if (entry->tableName == tName) {
            if (*(entry->setOfAttributes) == *attributes){
                return true;
            }
        }
    }   
    return false; 
}

void updateIndexEntry(const std::string& tName, std::set<std::string>* const attributes){
    auto it = std::find_if(indices.begin(),indices.end(), [&](IndexEntry* entry){ return entry->tableName == tName && *(entry->setOfAttributes) == *attributes;});
    if (it != indices.end()){
        IndexEntry* newIndex = new IndexEntry(tName, attributes);
        newIndex->setCreateTime(current_timestamp);
        newIndex->setNumOfAccesses((*it)->getNumOfAccesses() + 1);
        indices.erase(it);
        indices.push_back(newIndex);
    }
}


