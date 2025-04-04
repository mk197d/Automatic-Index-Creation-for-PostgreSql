#include "common-headers.h"

void printResult(pqxx::result &R);
void executeAndPrintQuery(pqxx::connection &C, const std::string &query);