#include "DataBaseHandler.h"

#include <QMessageBox>

DataBaseHandler::DataBaseHandler(const std::string& dataBaseParams) :
        c(dataBaseParams) {}

pqxx::result DataBaseHandler::selectPrintQuery(const std::string &query) {
    pqxx::work w(c);
    auto r = w.exec(query);
    w.commit();
    return r;

    //auto u = r.
    //auto t = w.query<int>("");
}
