#include <iostream>
#include <pqxx/pqxx> // PostgreSQL C++ library

int main() {
    try {
        pqxx::connection C("dbname=ecommerce user=mknined password=Mayan@197d host=localhost port=5432");
        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() << std::endl;
        } else {
            std::cerr << "Can't open database" << std::endl;
            return 1;
        }

        // pqxx::work W(C);
        
        // Query data
        pqxx::nontransaction N(C);
        pqxx::result R = N.exec("SELECT * FROM users;");
        
        for (const auto& row : R) {
            std::cout << "ID: " << row["user_id"].as<int>()<< std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
