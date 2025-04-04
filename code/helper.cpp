#include "helper.h"

void printResult(pqxx::result &R) {
    if (R.empty()) {
        std::cout << "Query executed successfully. No results to display." << std::endl;
        return;
    }

    for (int i = 0; i < R.columns(); ++i) {
        std::cout << std::setw(15) << std::left << R.column_name(i);
    }
    std::cout << std::endl;

    for (int i = 0; i < R.columns(); ++i) {
        std::cout << std::setw(15) << std::setfill('-') << "";
    }
    std::cout << std::setfill(' ') << std::endl;

    for (const auto &row : R) {
        for (int i = 0; i < R.columns(); ++i) {
            std::cout << std::setw(15) << std::left << row[i].c_str();
        }
        std::cout << std::endl;
    }
}

void executeAndPrintQuery(pqxx::connection &C, const std::string &query) {
    try {
        std::cout << query << std::endl;
        pqxx::nontransaction N(C);
        pqxx::result R = N.exec(query);
        printResult(R);
    } catch (const std::exception &e) {
        std::cerr << "Error executing query: " << e.what() << std::endl;
    }
}