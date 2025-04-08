#include "common-headers.h"
#include "keywords.h"
#include "helper.h"

extern int current_timestamp;
// using namespace std;
// using namespace pqxx;

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

            if (cmd == "\\show")
            {
                showNumAccesses();
                continue;
            }

            if (cmd == "\\q") {
                std::cout << "Exiting..." << std::endl;
                break;
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
