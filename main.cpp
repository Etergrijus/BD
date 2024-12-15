#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <list>
#include <map>
#include <windows.h>
#include <sstream>
#include <numeric>

typedef std::string Query;

class DataBaseConsoleHandler {
public:
    DataBaseConsoleHandler(const std::string &dbname, const std::string &user,
                           const std::string &password, const std::string &port) :
            c("dbname=" + dbname + " user=" + user + " password=" + password +
              " port=" + port), w(c) {
        if (c.is_open()) {
            std::cout << "DATABASE CONNECTED SUCCESSFULLY\n";
            menuItems = {{1, "Квалификации"},
                         {2, "Контактные данные"},
                         {3, "Исполнители"},
                         {4, "Виды работ"},
                         {5, "Домовладения"},
                         {6, "Жильцы"},
                         {7, "Задолженности жильцов"},
                         {8, "Запросы домовладений"},
                         {9, "Число работ, которые доступны для 1 исполнителя за год"},
                         {10, "Жильцы-должники"},
                         {0, "Выход"}};

            queryInfo = {{1, {"SELECT * FROM qualifications;",                       {"ID", "Description"}}},

                         {2, {"SELECT * FROM contact_details",                       {"ID", "Phone", "FCs"}}},

                         {3, {"SELECT e.id, phone, fcs FROM executors e\n"
                              "JOIN\n"
                              "contact_details cd ON cd.id = e.contact_details_id;", {"ID", "Phone", "FCs"}}},

                         {4, {"SELECT w.title, w.description, w.complexity, ws.season_number\n"
                              "FROM works w\n"
                              "JOIN works_seasonality ws on w.title = ws.work_title;",
                              {"title", "Description", "Complexity", "Season number"}}},

                         {5, {"SELECT * FROM home_ownerships;", {"Address", "Building Parameters",
                                                                         "Additional Info"}}},
                         {6, {"SELECT t.id, t.age, t.marital_status, cd.phone,\n"
                              "cd.fcs, t.flat_number, t.flat_owner FROM tenants t\n"
                              "JOIN tenants_list tl on t.id = tl.tenant_id\n"
                              "JOIN contact_details cd on cd.id = t.contact_details_id;",
                              {"ID", "Age", "Marital Status", "Phone", "FCs", "Flat Number", "Flat Owner"}}},

                         {7, {"SELECT td.id,\n"
                              "t.id,\n"
                              "td.water_debt_size,\n"
                              "td.gas_debt_size,\n"
                              "td.electricity_debt_size\n"
                              "FROM total_debts td\n"
                              "JOIN tenants t on t.id = td.tenant_id;", {"ID", "Tenant ID", "Water Debt Size",
                                                                         "Gas Debt Size", "Electricity Debt Size"}}},

                         {8, {"SELECT r.id, r.urgency, r.description, ho.address, r.work_title\n"
                              "FROM requests r\n"
                              "JOIN home_ownerships ho ON r.home_ownership_address = ho.address\n"
                              "JOIN works w on w.title = r.work_title;", {"ID", "Urgency", "Description",
                                                                          "Address", "Work Title"}}},
                         {9, {"SELECT e.executor_id,\n"
                              "COUNT(s.season_number) AS total_seasons\n"
                              "FROM executors_list e\n"
                              "JOIN\n"
                              "works w ON e.work_title = w.title\n"
                              "JOIN\n"
                              "works_seasonality s ON w.title = s.work_title\n"
                              "GROUP BY e.executor_id ORDER BY e.executor_id;", {"Executor ID", "Count Of Seasons"}}},
                         {10, {"SELECT *\n"
                               "FROM total_debts\n"
                               "WHERE total_debts.water_debt_size != 0\n"
                               "OR total_debts.gas_debt_size != 0\n"
                               "OR total_debts.electricity_debt_size != 0\n"
                               "ORDER BY water_debt_size DESC;", {"ID", "Tenant ID", "Water Debt Size",
                                                                  "Gas Debt Size", "Electricity Debt Size"}}}
                         };

            while (true) {
                printMenu();
                printQueryResult();
                //std::cout << "\n-> ";
            }
        } else {
            std::cerr << "DATABASE FAILED ON CONNECTION";
            return;
        }
    }

private:
    pqxx::connection c;
    pqxx::work w;
    std::map<int, std::string> menuItems;
    std::map<int, std::pair<Query, std::vector<std::string>>> queryInfo;
    int currentMenuItem;

    void printMenu() {
        std::cout << "Выберите необходимый пункт меню:\n";
        for (auto &i: menuItems)
            std::cout << i.first << ". " << i.second << '\n';
        std::cout << "-> ";

        std::string inputted;
        while (!std::getline(std::cin, inputted)) {}
        std::istringstream(inputted) >> currentMenuItem;
    }

    void printQueryResult() {
        if (currentMenuItem > std::max_element(menuItems.begin(), menuItems.end())->first ||
            currentMenuItem < 0) {
            std::cout << "Неожиданное значение, проверьте ввод\n";
        } else if (currentMenuItem == 0)
            exit(0);
        else {
            std::string text;
            std::vector<int> sizes = {};
            for (const auto &j: queryInfo[currentMenuItem].second) {
                auto length = j.length();
                if (length > 13)
                    sizes.push_back(length + 2);
                else
                    sizes.push_back(15);
            }
            sizes[0]++;
            auto delimLine = "+"
                             + std::string(std::reduce(sizes.begin(), sizes.end())
                                           + queryInfo[currentMenuItem].second.size() - 2, '=')
                             + "+\n";

            text.append(delimLine);

            int columnName = 0;
            for (auto i = 0; i < sizes.size(); i++, columnName++) {
                std::string columnTitle =
                        getSpacesStr(sizes[i], queryInfo[currentMenuItem].second[columnName].length())
                        + queryInfo[currentMenuItem].second[columnName] + "|";
                text.append(columnTitle);
            }
            text += "\n";
            text += delimLine;

            auto r = w.exec(queryInfo[currentMenuItem].first);
            size_t nCols = r.columns();
            for (auto rowNum = 0; rowNum < r.size(); rowNum++) {
                pqxx::row row = r[rowNum];
                for (auto colNum = 0; colNum < nCols; colNum++) {
                    pqxx::field field = row[colNum];
                    if (field.is_null()) {
                        text += std::string(sizes[colNum], ' ') + "|";
                        continue;
                    }

                    auto gotValue = field.as<std::string>();
                    //Строка кириллическая?
                    if (std::find_if(gotValue.begin(), gotValue.end(),
                                     [](char ch) {return ch < 0;}) != gotValue.end()) {
                        //C++ и юникод форева. Ищем однобайтные символы в строке с изобилием
                        //двухбайтных кириллических для красивого вывода
                        auto nAscIISymbolsInField =
                                std::count_if(gotValue.begin(), gotValue.begin() + ((sizes[colNum] - 3) * 2),
                                              [](char ch) {return ch >= 32 && ch <= 127;});
                        //Строка больше размеров ячейки?
                        if ((gotValue.length() + nAscIISymbolsInField) / 2 > sizes[colNum]) {
                            gotValue.assign(gotValue.begin(), gotValue.begin() +
                                                (((sizes[colNum] - 3) * 2) - nAscIISymbolsInField));
                            gotValue.append("...");
                            text += gotValue + "|";
                        } else {
                            nAscIISymbolsInField = std::count_if(gotValue.begin(), gotValue.end(),
                                                                 [](char ch) { return ch >= 32 && ch <= 127; });
                            text += getSpacesStr(sizes[colNum], gotValue.length() / 2 + nAscIISymbolsInField)
                                    + gotValue + "|";
                        }
                    } else { //Без кириллицы всё просто
                        if (gotValue.length() > sizes[colNum]) {
                            gotValue.assign(gotValue.begin(), gotValue.begin() + 12);
                            gotValue.append("...");
                            text += gotValue + "|";
                        } else
                            text += getSpacesStr(sizes[colNum], gotValue.length()) + gotValue + "|";
                    }

                }
                text += "\n";
            }
            text += delimLine;

            std::cout << text;
        }
    }

    static std::string getSpacesStr(unsigned int a, unsigned int b) {
        return std::string(a - b, ' ');
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    DataBaseConsoleHandler dbch{"postgres", "postgres", "postgres", "5432"};

    return 0;
}