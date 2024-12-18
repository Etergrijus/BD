//
// Created by Павел on 16.12.2024.
//

#ifndef DATABASE_1_DATABASEHANDLER_H
#define DATABASE_1_DATABASEHANDLER_H

#include <pqxx/pqxx>

class DataBaseHandler {
public:
    explicit DataBaseHandler(const std::string& dataBaseParams);

    pqxx::result selectPrintQuery(const std::string& query);

private:
    pqxx::connection c;
};


#endif //DATABASE_1_DATABASEHANDLER_H
