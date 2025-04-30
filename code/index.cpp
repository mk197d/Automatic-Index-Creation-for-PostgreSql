#include "index.h"
/*
    1. Create an IndexEntry structure for incoming queries if one does not already exist.
    2. Create an index in postgres through the query "CREATE INDEX <index_name> on <table_name>(<col1>,<col2>,...,<coln>);"
    3. <index_name> = <table_name> + current_timestamp
    4. Delete an index entry from indices.
    5. Delete an index from postgres through the query "DROP INDEX IF EXISTS <index_name>;" 
    6. <index_name> is obtained from entry->indexName.
*/
matrix_t frequencyMap;

const int THRESHOLD1 = 10;
int THRESHOLD2 = 0;

static int last_T_updated = -1;
std::map<std::string, int> num_rows_for_each_table;

extern std::string hostname;
extern std::string port;
extern std::string database;
extern std::string username;
extern std::string password;

std::map<std::string, KeywordType> keyword_map = {
    {"SELECT", KeywordType::SELECT},
    {"INSERT", KeywordType::INSERT},
    {"UPDATE", KeywordType::UPDATE},
    {"DELETE", KeywordType::DELETE},
    {"CREATE", KeywordType::CREATE},
    {"DROP", KeywordType::DROP},
    {"ALTER", KeywordType::ALTER},
    {"TABLE", KeywordType::TABLE},
    {"DATABASE", KeywordType::DATABASE},
    {"INDEX", KeywordType::INDEX},
    {"VIEW", KeywordType::VIEW},
    {"TRIGGER", KeywordType::TRIGGER},
    {"FUNCTION", KeywordType::FUNCTION},
    {"PROCEDURE", KeywordType::PROCEDURE},
    {"SCHEMA", KeywordType::SCHEMA},
    {"GRANT", KeywordType::GRANT},
    {"REVOKE", KeywordType::REVOKE},
    {"COMMIT", KeywordType::COMMIT},
    {"ROLLBACK", KeywordType::ROLLBACK},
    {"BEGIN", KeywordType::BEGIN},
    {"END", KeywordType::END},
    {"IF", KeywordType::IF},
    {"ELSE", KeywordType::ELSE_k},
    {"CASE", KeywordType::CASE},
    {"WHEN", KeywordType::WHEN},
    {"THEN", KeywordType::THEN},
    {"ELSEIF", KeywordType::ELSEIF},
    {"LOOP", KeywordType::LOOP},
    {"WHILE", KeywordType::WHILE},
    {"FOR", KeywordType::FOR},
    {"DO", KeywordType::DO},
    {"RETURN", KeywordType::RETURN_k},
    {"WHERE", KeywordType::WHERE},
    {"FROM", KeywordType::FROM},
    {"INTO", KeywordType::INTO},
    {"VALUES", KeywordType::VALUES},
    {"SET", KeywordType::SET},
    {"AS", KeywordType::AS},
    {"ON", KeywordType::ON},
    {"JOIN", KeywordType::JOIN},
    {"INNER JOIN", KeywordType::INNER_JOIN},
    {"LEFT JOIN", KeywordType::LEFT_JOIN}, 
    {"RIGHT JOIN", KeywordType::RIGHT_JOIN},
    {"FULL JOIN", KeywordType::FULL_JOIN},
    {"CROSS JOIN", KeywordType::CROSS_JOIN},
    {"NATURAL JOIN", KeywordType::NATURAL_JOIN},
    {"EXISTS", KeywordType::EXISTS},
    {"ALL", KeywordType::ALL},
    {"ANY", KeywordType::ANY},
    {"SOME", KeywordType::SOME},
    {"DISTINCT", KeywordType::DISTINCT},
    {"GROUP BY", KeywordType::GROUP_BY},
    {"ORDER BY", KeywordType::ORDER_BY},
    {"HAVING", KeywordType::HAVING},
    {"LIMIT", KeywordType::LIMIT},
    {"OFFSET", KeywordType::OFFSET}
};

void updateMap(const std::string& tableName,const std::vector<std::string>& attributes){
    std::cout << tableName << std::endl;
    std::cout << attributes.size() << std::endl;
    if (attributes.size() == 0) return;
    for (int i = 0 ; i < attributes.size() ; i++){
        if (frequencyMap.find({tableName,attributes[i]}) != frequencyMap.end()){
            frequencyMap[{tableName,attributes[i]}]++;
        }  
        else {
            frequencyMap.insert({{tableName,attributes[i]},1});
        }
    }
}

void scanMap(pqxx::work& txn, std::string const & query){
    std::set<std::string>* attrs = new std::set<std::string>();
    std::vector<std::pair<double, std::pair<std::string, std::set<std::string>*>>> costMap;
    for (auto [u,v] : frequencyMap){
        std::cout << "Table Name: " << u.first << " Attribute Name: " << u.second << " Number of Accesses: " << v << std::endl;
        bool condition = (v >= THRESHOLD1);
        if (num_rows_for_each_table.count(u.first))
        {
            condition = (condition || ((v*num_rows_for_each_table[u.first]) >= THRESHOLD2));
        }
        if (condition){
            attrs->insert(u.second);
            double cost = get_hypo_cost(query,u.first,attrs);
            costMap.push_back({cost,{u.first,attrs}});
        }
    }
    std::cout << "Cost Map Size: " << costMap.size() << std::endl;
    if (costMap.size() == 0) return;
    std::sort(costMap.begin(), costMap.end());
    std::reverse(costMap.begin(), costMap.end());
    for (int i = 0 ; i < (costMap.size() + 1)/2; i++) {
        fork_a_child_for_index(costMap[i].second.first, costMap[i].second.second, txn);
    }
}

void indexCreation(pqxx::work& txn, std::string const & query) {
    if (last_T_updated == -1)
    {
        printRowCounts(txn);
        last_T_updated = 0;
    }
    else if (current_timestamp % 50 == 0)
    {
        printRowCounts(txn);
        last_T_updated = current_timestamp;
    }
    if (!existing_child_processes.empty())
    {
        std::set<pid_t> finished_children;
        for (pid_t child_pid : existing_child_processes) {
            int status;
            pid_t result = waitpid(child_pid, &status, WNOHANG);
            if (result > 0) {
                finished_children.insert(child_pid);
            }
        }
        for (pid_t finished_pid : finished_children) {
            existing_child_processes.erase(finished_pid);
        }
    }
    std::ofstream tempQueryFile("tempQuery.sql");
    if (tempQueryFile.is_open()) {
        tempQueryFile << query;
        tempQueryFile.close();
    } else {
        std::cerr << "Failed to open tempQuery.sql for writing." << std::endl;
        return;
    }

    int result = std::system("python3 query_parser.py tempQuery.sql > tempParse.txt");
    if (result != 0) {
        std::cerr << "Failed to execute query_parser.py." << std::endl;
        return;
    }

    std::ifstream tempParseFile("tempParse.txt");
    if (!tempParseFile.is_open()) {
        std::cerr << "Failed to open tempParse.txt for reading." << std::endl;
        return;
    }
    std::string line;
    while (std::getline(tempParseFile, line)) {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            std::cerr << "Invalid line format: " << line << std::endl;
            continue;
        }

        std::string tableName = line.substr(0, colonPos);
        tableName.erase(std::remove(tableName.begin(), tableName.end(), ' '), tableName.end());

        size_t startBracket = line.find('[', colonPos);
        size_t endBracket = line.find(']', startBracket);
        if (startBracket == std::string::npos || endBracket == std::string::npos) {
            std::cerr << "Invalid attribute list format: " << line << std::endl;
            continue;
        }

        std::string attributesStr = line.substr(startBracket + 1, endBracket - startBracket - 1);
        std::istringstream attrStream(attributesStr);
        std::string attribute;
        std::vector<std::string> attributes;
        while (std::getline(attrStream, attribute, ',')) { 
            attribute.erase(std::remove(attribute.begin(), attribute.end(), '\''), attribute.end());
            attribute.erase(std::remove(attribute.begin(), attribute.end(), ' '), attribute.end());
            if(tableName != attribute && attributeExists(txn, tableName, attribute)){
                // count_of_num_accesses[tableName][attribute]++; // redundant now
                attributes.push_back(attribute);
            
            } 
        }
        // std::cout << attributes.size() << std::endl;
        if (attributes.size() == 0) continue;
        updateMap(tableName,attributes);
        scanMap(txn, query);
    }

    tempParseFile.close();
}

void fork_a_child_for_index(const std::string& tableName, std::set<std::string>* const atrs, pqxx::work& txn)
{
    
    if (indexExists(tableName,atrs)){
        updateIndexEntry(tableName,atrs);
        return;
    }
    IndexEntry* entry = new IndexEntry(tableName,atrs);
    indices.push_back(entry);
    pid_t child = fork();
    if (child == 0)
    {
        pqxx::connection conn("dbname=" + database + " user=" + username + " password=" + password + " host=" + hostname + " port=" + port);
        pqxx::work txn1(conn);
        

        try {
            std::set<std::string> cols = *atrs;
    
            std::string idxName = entry->indexName;
            if (atrs->empty()){
                std::string pkQuery = R"(
                    SELECT a.attname
                    FROM pg_index i
                    JOIN pg_class c ON c.oid = i.indrelid
                    JOIN pg_attribute a ON a.attrelid = c.oid AND a.attnum = ANY(i.indkey)
                    WHERE i.indisprimary AND c.relname = $1;
                )";
    
                pqxx::result r = txn1.exec_params(pkQuery, tableName);
                for (const auto& row : r) {
                    cols.insert(row[0].as<std::string>());
                }
    
                if (cols.empty()) {
                    throw std::runtime_error("No columns specified and no primary key found.");
                }
            }
            std::string colList;
            for (auto it = cols.begin(); it != cols.end(); ++it) {
                colList += txn1.quote_name(*it);
                if (std::next(it) != cols.end())
                    colList += ", ";
            }        
            std::string query = "CREATE INDEX IF NOT EXISTS " + txn1.quote_name(idxName) + " ON " + txn1.quote_name(tableName) + " (" + colList + ");";
    
            txn1.exec0(query);
            std::cout << "Index '" << idxName << "' created on table '" << tableName << "'.\n";
        } 
        catch (const std::exception& e) {
            std::cerr << "Failed to create index: " << e.what() << '\n';
        }        
        exit(0);
    }
    else 
    {
        existing_child_processes.insert(child);
    }
}

void showNumAccesses()
{
    for (auto [u,v] : frequencyMap){
        std::cout << "Table Name: " << u.first << " Attribute Name: " << u.second << " Number of Accesses: " << v << std::endl;
    }
}

bool attributeExists(pqxx::work& txn, const std::string &relationName, const std::string &attributeName) {
    try {
        // Query the information_schema.columns table to check if the attribute exists
        std::string attrName = attributeName;
        std::string tableName = relationName;
        std::transform(attrName.begin(), attrName.end(), attrName.begin(), ::tolower); 
        std::transform(tableName.begin(), tableName.end(), tableName.begin(), ::tolower); 

        std::string query = "SELECT COUNT(*) FROM information_schema.columns "
                           "WHERE table_name = '" + tableName + "' "
                           "AND column_name = '" + attrName + "'";
        
        pqxx::result R = txn.exec(query);
        
        // If the result has at least one row and the count is greater than 0, the attribute exists
        if (!R.empty() && R[0][0].as<int>() > 0) {
            return true;
        }
        return false;
    } catch (const std::exception &e) {
        std::cerr << "Error checking attribute existence: " << e.what() << std::endl;
        return false;
    }
}

void clearIndices(pqxx::work& txn){
    switch (p) {
        case POLICY::P1:{
            auto it = indices.begin();
            std::vector<IndexEntry*> indicesToDelete;
            for (; it != indices.end(); ++it) {
                if (current_timestamp - (*it)->getCreateTime() <= 5) {
                    break; 
                }
                indicesToDelete.push_back(*it);
            }
            indices.erase(indices.begin(), it);
            if (indicesToDelete.size() == 0) break;
            try {
            
                for (const auto& name : indicesToDelete) {
                    txn.exec("DROP INDEX IF EXISTS " + txn.quote_name(name->indexName) + ";");
                }
            
                // txn.commit();
                std::cout << "Expired indices removed from DB.\n";
            } 
            catch (const std::exception& e) {
                std::cerr << "Failed to delete from DB: " << e.what() << '\n';
            }
                        
            break;
        }
        case POLICY::P2:{
            auto it = indices.begin();
            std::vector<IndexEntry*> newIndices;
            std::vector<IndexEntry*> indicesToDelete;
            for (; it != indices.end(); ++it) {
                if (4*(*it)->getNumOfAccesses() < current_timestamp - (*it)->getCreateTime()) {
                    indicesToDelete.push_back(*it);
                }
                else {
                    newIndices.push_back(*it);
                }
            }
            if (indicesToDelete.size() == 0) break;
            indices = newIndices;
            try {
            
                for (const auto& name : indicesToDelete) {
                    txn.exec("DROP INDEX IF EXISTS " + txn.quote_name(name->indexName) + ";");
                }
            
                std::cout << "Expired indices removed from DB.\n";
            } 
            catch (const std::exception& e) {
                std::cerr << "Failed to delete from DB: " << e.what() << '\n';
            }            
        }
        default:
            break;
    }
}

void printRowCounts(pqxx::work& txn) {
    try {
        std::string query = R"(
            SELECT relname 
            FROM pg_class 
            WHERE relkind = 'r' 
              AND relnamespace IN (
                SELECT oid FROM pg_namespace 
                WHERE nspname NOT IN ('pg_catalog', 'information_schema')
              )
        )";

        pqxx::result tables = txn.exec(query);

        THRESHOLD2 = 0;
        for (const auto& row : tables) {
            std::string tableName = row[0].as<std::string>();
            try {
                pqxx::result countResult = txn.exec("SELECT COUNT(*) FROM " + txn.quote_name(tableName));
                num_rows_for_each_table[tableName] = countResult[0][0].as<int64_t>();
                // std::cout << "Table: " << tableName << " | Row Count: " << countResult[0][0].as<int64_t>() << std::endl;
                THRESHOLD2 += countResult[0][0].as<int64_t>();

            } catch (const std::exception& e) {
            }
        }

        if (tables.size() > 0) {
            THRESHOLD2 /= tables.size();
        }

        std::cout << "THRESHOLD2: " << THRESHOLD2 << std::endl;
    } catch (const std::exception& e) {
    }
}


double get_hypo_cost(std::string query, std::string tableName, std::set<std::string>* const atrs)
{
    double cost = 0;
    try {
        // std::string query = "SELECT * FROM testable WHERE id = 5";
        std::string indexStmt = "CREATE INDEX ON ";
        indexStmt += tableName + " (";
        for (auto it = atrs->begin(); it != atrs->end(); ++it) {
            indexStmt += *it;
            if (std::next(it) != atrs->end())
                indexStmt += ", ";
        }
        indexStmt += ")";
        std::string command = "python3 get_cost.py \"" + query + "\" \"" + indexStmt + "\"";

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run optimizer script." << std::endl;
            return cost;
        }

        char buffer[128];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        cost = std::stod(result);
    }
    catch (const std::exception& e) {
    }

    return cost;
}
