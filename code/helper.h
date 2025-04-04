#ifndef HELPER_H
#define HELPER_H
#include "common-headers.h"
#include "index.h"

void printResult(pqxx::result &R);
void executeAndPrintQuery(pqxx::connection &C, const std::string &query);

#endif