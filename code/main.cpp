#include "common-headers.h"
#include "keywords.h"
#include "helper.h"
extern int current_timestamp;
POLICY p;

int main() {
    std::string hostname = "localhost";
    std::string port = "5432";
    std::string database = "ImDB";
    std::string username = "test";
    std::string password = "test";

    try {
        pqxx::connection C("dbname=" + database + " user=" + username + " password=" + password +
                     " host=" + hostname + " port=" + port);
        if (!C.is_open()) {
            std::cerr << "Can't open database" << std::endl;
            return 1;
        }
        p = POLICY::P2;
        while (true) {
            char *input = readline("pgshell# ");
            if (!input) {
                std::cout << "Exiting..." << std::endl;
                break;
            }

            std::string cmd(input);
            free(input);

            if (cmd == "\\show")
            {
                showNumAccesses();
                continue;
            }

            if (cmd == "\\q") {
                std::cout << "Exiting..." << std::endl;
                break;
            }

            if (cmd == "\\d") {
                pqxx::work W(C);
                pqxx::result R = W.exec(
                    "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public';"
                );
                W.commit();

                std::cout << "List of relations" << std::endl;
                std::cout << "-----------------" << std::endl;
                for (const auto &row : R) {
                    std::cout << row[0].c_str() << std::endl;
                }
                continue;
            }

            if (cmd.empty()) {
                continue;
            }

            add_history(cmd.c_str());
            current_timestamp++;
            executeAndPrintQuery(C, cmd);
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
