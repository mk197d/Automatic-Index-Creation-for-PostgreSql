#include "common-headers.h"
#include "keywords.hh"

// using namespace std;
// using namespace pqxx;

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

int main() {
    std::string hostname = "localhost";
    std::string port = "5432";
    std::string database = "ecommerce";
    std::string username = "test";
    std::string password = "test";

    try {
        pqxx::connection C("dbname=" + database + " user=" + username + " password=" + password +
                     " host=" + hostname + " port=" + port);
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return 1;
        }

        while (true) {
            char *input = readline("pgshell# ");
            if (!input) {
                std::cout << "Exiting..." << std::endl;
                break;
            }

            std::string cmd(input);
            free(input);

            if (cmd == "\\q") {
                std::cout << "Exiting..." << std::endl;
                break;
            }

            if (cmd.empty()) {
                continue;
            }

            add_history(cmd.c_str());
            executeAndPrintQuery(C, cmd);
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
