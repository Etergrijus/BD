#ifndef DATABASE_1_MAINWINDOW_H
#define DATABASE_1_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>

#include "DataBaseHandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(const std::string &dataBaseParams, QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    std::map<int, std::pair<std::string, QStringList>> queryInfo =
            {{0, {"SELECT * FROM qualifications;", {"ID", "Description"}}},

             {1, {"SELECT * FROM contact_details", {"ID", "Phone", "FCs"}}},

             {2, {"SELECT e.id, phone, fcs FROM executors e\n"
                  "JOIN\n"
                  "contact_details cd ON cd.id = e.contact_details_id;", {"ID", "Phone", "FCs"}}},

             {3, {"SELECT w.title, w.description, w.complexity, ws.season_number\n"
                  "FROM works w\n"
                  "JOIN works_seasonality ws on w.title = ws.work_title;",{"Title","Description",
                                                                           "Complexity", "Season number"}}},

             {4, {"SELECT * FROM home_ownerships;",{"Address", "Building Parameters", "Additional Info"}}},

             {5, {"SELECT t.id, t.age, t.marital_status, cd.phone,\n"
                  "cd.fcs, t.flat_number, t.flat_owner FROM tenants t\n"
                  "JOIN tenants_list tl on t.id = tl.tenant_id\n"
                  "JOIN contact_details cd on cd.id = t.contact_details_id;",{"ID", "Age", "Marital Status", "Phone",
                                                                              "FCs", "Flat Number", "Flat Owner"}}},

             {6, {"SELECT td.id, cd.fcs, td.water_debt_size, td.gas_debt_size, td.electricity_debt_size\n"
                  "FROM total_debts AS td JOIN tenants t on td.tenant_id = t.id\n"
                  "JOIN contact_details cd on cd.id = t.contact_details_id;", {"ID", "FCs", "Water Debt Size",
                                                            "Gas Debt Size", "Electricity Debt Size"}}},

             {7, {"SELECT r.id, r.urgency, r.description, ho.address, r.work_title\n"
                  "FROM requests r\n"
                  "JOIN home_ownerships ho ON r.home_ownership_address = ho.address\n"
                  "JOIN works w on w.title = r.work_title;",{"ID", "Urgency", "Description",
                                                             "Address", "Work Title"}}},

             {8, {"SELECT e.executor_id,\n"
                  "COUNT(s.season_number) AS total_seasons\n"
                  "FROM executors_list e\n"
                  "JOIN\n"
                  "works w ON e.work_title = w.title\n"
                  "JOIN\n"
                  "works_seasonality s ON w.title = s.work_title\n"
                  "GROUP BY e.executor_id ORDER BY e.executor_id;",      {"Executor ID", "Count Of Seasons"}}},

             {9, {"SELECT *\n"
                  "FROM total_debts\n"
                  "WHERE total_debts.water_debt_size != 0\n"
                  "OR total_debts.gas_debt_size != 0\n"
                  "OR total_debts.electricity_debt_size != 0\n"
                  "ORDER BY water_debt_size DESC;",{"ID", "Tenant ID", "Water Debt Size",
                                                    "Gas Debt Size", "Electricity Debt Size"}}}
            };

    std::map<int, std::string> codesOfTables = {{0, "qualifications"},
                                                {1, "contact_details"},
                                                {2, "executors"},
                                                {3, "works"},
                                                {4, "home_ownerships"},
                                                {5, "tenants"},
                                                {6, "total_debts"},
                                                {7, "requests"},
                                                {8, "count_of_works"},
                                                {9, "tenants_with_debts"}};

    //Ассоц. Массив, где хранятся соответствия имён столбцов в БД
    //и тех имён, которые выводятся в GUI
    std::map<int, QStringList> columnMatching = {
             {0, {"id", "description"}},

             {1, {"id", "phone", "fcs"}},

             {2, {"id", "phone", "fcs"}},

             {3, {"title", "description", "complexity", "season_number"}},

             {4, {"address", "building_parameters", "additional_info"}},

             {5, {"id", "age", "marital_status", "phone", "fcs", "flat_number", "flat_owner"}},

             {6, {"id", "fcs", "water_debt_size", "gas_debt_size", "electricity_debt_size"}},

             {7, {"address", "work_title"}},

             {8, {"executor_id", "total_seasons"}},

             {9, {"id", "tenant_id", "water_debt_size", "gas_debt_size", "electricity_debt_size"}}
    };

    std::map<int, QStringList> joinedColumns = {
            {0, {}},

            {1, {}},

            {2, {"phone", "fcs"}},

            {3, {"season_number"}},

            {4, {}},

            {5, {"phone", "fcs"}},

            {6, {"fcs"}},

            {7, {"id", "urgency", "description", "address", "work_title"}},

            {8, {"executor_id", "total_seasons"}},

            {9, {"id", "tenant_id", "water_debt_size", "gas_debt_size", "electricity_debt_size"}}
    };


    DataBaseHandler db;

    QWidget *mainWidget;

    QComboBox *comboBox;
    QTableWidget *table;
    QVBoxLayout *mainLayout;

    QPushButton *addingButton;
    QPushButton *deletingButton;

    int tableIndex;
    QString savedItemText;

    QStringList getPrimaryKeysCols();

    QStringList getBoolCols();

    void makeTableDerived(QStringList &cols);

private slots:

    void makeTable(int index);

    void onItemDoubleClick(QTableWidgetItem *item);

    void updateDataBase(QTableWidgetItem *item);

    void deleteRows();

    void addRow();
};


#endif //DATABASE_1_MAINWINDOW_H
