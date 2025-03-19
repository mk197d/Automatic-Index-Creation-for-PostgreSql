#include <iostream>
#include <pqxx/pqxx> // PostgreSQL C++ library
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace pqxx;

void printResult(result &R) {
    if (R.empty()) {
        cout << "Query executed successfully. No results to display." << endl;
        return;
    }

    for (int i = 0; i < R.columns(); ++i) {
        cout << setw(15) << left << R.column_name(i);
    }
    cout << endl;

    for (int i = 0; i < R.columns(); ++i) {
        cout << setw(15) << setfill('-') << "";
    }
    cout << setfill(' ') << endl;

    for (const auto &row : R) {
        for (int i = 0; i < R.columns(); ++i) {
            cout << setw(15) << left << row[i].c_str();
        }
        cout << endl;
    }
}

void executeAndPrintQuery(connection &C, const string &query) {
    try {
        cout<<query<<endl;
        nontransaction N(C);
        result R = N.exec(query);
        printResult(R);

    } catch (const exception &e) {
        cerr << "Error executing query: " << e.what() << endl;
    }
}

int main() {
    string hostname = "localhost";
    string port = "5432";
    string database = "ecommerce";
    string username = "";
    string password = "";

    try {
        connection C("dbname=" + database + " user=" + username + " password=" + password +
                     " host=" + hostname + " port=" + port);
        if (!C.is_open()) {
            cerr << "Can't open database" << endl;
            return 1;
        }

        string cmd;
        while (true) {
            cout << "pgshell# ";
            getline(cin, cmd);

            if (cmd == "\\q") {
                cout << "Exiting..." << endl;
                break;
            }

            if (cmd.empty()) {
                continue;
            }

            executeAndPrintQuery(C, cmd);
        }

    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
