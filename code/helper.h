#include "common-headers.h"
#include "index.h"

void printResult(pqxx::result &R);
void executeAndPrintQuery(pqxx::connection &C, const std::string &query);