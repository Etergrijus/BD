#include <QScrollArea>
#include <QTableWidgetItem>
#include <QHeaderView>

#include "mainwindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(const std::string& dataBaseParams, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow),
        c(dataBaseParams), w(c) {
    ui->setupUi(this);

    this->setMinimumSize(1000, 700);
    this->setWindowTitle("Data Base GUI");

    mainWidget = new QWidget(this);
    this->setCentralWidget(mainWidget);

    queryInfo = {{0, {"SELECT * FROM qualifications;", {"ID", "Description"}}},

                 {1, {"SELECT * FROM contact_details", {"ID", "Phone", "FCs"}}},

                 {2, {"SELECT e.id, phone, fcs FROM executors e\n"
                      "JOIN\n"
                      "contact_details cd ON cd.id = e.contact_details_id;", {"ID", "Phone", "FCs"}}},

                 {3, {"SELECT w.title, w.description, w.complexity, ws.season_number\n"
                      "FROM works w\n"
                      "JOIN works_seasonality ws on w.title = ws.work_title;",
                      {"title", "Description", "Complexity", "Season number"}}},

                 {4, {"SELECT * FROM home_ownerships;", {"Address", "Building Parameters",
                                                                                     "Additional Info"}}},
                 {5, {"SELECT t.id, t.age, t.marital_status, cd.phone,\n"
                      "cd.fcs, t.flat_number, t.flat_owner FROM tenants t\n"
                      "JOIN tenants_list tl on t.id = tl.tenant_id\n"
                      "JOIN contact_details cd on cd.id = t.contact_details_id;",
                      {"ID", "Age", "Marital Status", "Phone", "FCs", "Flat Number", "Flat Owner"}}},

                 {6, {"SELECT td.id,\n"
                      "t.id,\n"
                      "td.water_debt_size,\n"
                      "td.gas_debt_size,\n"
                      "td.electricity_debt_size\n"
                      "FROM total_debts td\n"
                      "JOIN tenants t on t.id = td.tenant_id;", {"ID", "Tenant ID", "Water Debt Size",
                                                                 "Gas Debt Size", "Electricity Debt Size"}}},

                 {7, {"SELECT r.id, r.urgency, r.description, ho.address, r.work_title\n"
                      "FROM requests r\n"
                      "JOIN home_ownerships ho ON r.home_ownership_address = ho.address\n"
                      "JOIN works w on w.title = r.work_title;", {"ID", "Urgency", "Description",
                                                                  "Address", "Work Title"}}},
                 {8, {"SELECT e.executor_id,\n"
                      "COUNT(s.season_number) AS total_seasons\n"
                      "FROM executors_list e\n"
                      "JOIN\n"
                      "works w ON e.work_title = w.title\n"
                      "JOIN\n"
                      "works_seasonality s ON w.title = s.work_title\n"
                      "GROUP BY e.executor_id ORDER BY e.executor_id;", {"Executor ID", "Count Of Seasons"}}},
                 {9, {"SELECT *\n"
                       "FROM total_debts\n"
                       "WHERE total_debts.water_debt_size != 0\n"
                       "OR total_debts.gas_debt_size != 0\n"
                       "OR total_debts.electricity_debt_size != 0\n"
                       "ORDER BY water_debt_size DESC;", {"ID", "Tenant ID", "Water Debt Size",
                                                          "Gas Debt Size", "Electricity Debt Size"}}}
    };

    comboBox = new QComboBox;

    QStringList comboBoxItems = {"Квалификации", "Контактные данные", "Исполнители", "Виды работ", "Домовладения",
                                 "Жильцы", "Задолженности жильцов", "Запросы домовладений",
                                 "Число работ, которые доступны для 1 исполнителя за год", "Жильцы-должники"};
    comboBox->insertItems(0, comboBoxItems);
    comboBox->setCurrentIndex(-1);
    comboBox->setPlaceholderText("Выберите таблицу из списка");
    comboBox->setFixedSize(400, 60);

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(comboBox, 0);
    mainLayout->setAlignment(comboBox,Qt::AlignHCenter | Qt::AlignTop);

    connect(comboBox, &QComboBox::currentIndexChanged, this,
            [this] () {MainWindow::makeTable(comboBox->currentIndex());});


    table = new QTableWidget;
    table->setStyleSheet("QTableWidget { border: 2px solid black; }"
                         "QTableWidget::item { border: 1px solid black; }"
                         "QHeaderView::section { background-color: lightgray; }");
    table->setWordWrap(true);

    auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(680, 400);
    scrollArea->setWidget(table);
    mainLayout->addWidget(scrollArea, 3);
    this->setLayout(mainLayout);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::getPrimaryKeys() {
    std::string queryPrimaryKeys = "SELECT kcu.ordinal_position\n"
                                   "FROM information_schema.table_constraints AS tc\n"
                                   "         JOIN information_schema.key_column_usage AS kcu\n"
                                   "              ON kcu.constraint_name = tc.constraint_name\n"
                                   "WHERE tc.table_name =" + table.

}

void MainWindow::makeTable(int index) {
    table->setColumnCount(queryInfo[index].second.size());
    table->setHorizontalHeaderLabels(queryInfo[index].second);

    auto r = w.exec(queryInfo[index].first);
    table->setRowCount(r.size());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for (int rowNum = 0; rowNum < table->rowCount(); rowNum++) {
        pqxx::row row = r[rowNum];


        for (int colNum = 0; colNum < table->columnCount(); colNum++) {
            pqxx::field field = row[colNum];
            if (field.is_null()) {
                QString str = "";
                auto item = new QTableWidgetItem(str);
                item->setTextAlignment(Qt::AlignCenter);
                table->setItem(rowNum, colNum, item);
                continue;
            }

            auto preStr = field.as<std::string>();
            QString str = QString::fromStdString(preStr);
            auto item = new QTableWidgetItem(str);
            if (colNum == 0)
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(rowNum, colNum, item);
        }
    }
}
