#include <QScrollArea>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

#include <iostream>

#include "mainwindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(const QString &dataBaseName, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow), db(dataBaseName) {
    ui->setupUi(this);

    this->setMinimumSize(1000, 700);
    this->setWindowTitle("Data Base GUI");

    mainWidget = new QWidget(this);
    this->setCentralWidget(mainWidget);

    comboBox = new QComboBox;

    QStringList comboBoxItems = {"Квалификации", "Контактные данные", "Исполнители", "Виды работ", "Домовладения",
                                 "Жильцы", "Задолженности жильцов", "Запросы домовладений",
                                 "Число работ, которые доступны для 1 исполнителя за год", "Жильцы-должники"};
    comboBox->insertItems(0, comboBoxItems);
    comboBox->setCurrentIndex(-1);
    comboBox->setPlaceholderText("Выберите таблицу из списка");
    comboBox->setFixedSize(400, 60);
    connect(comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::showTable);

    mainLayout = new QVBoxLayout(mainWidget);
    //mainLayout->addLayout(upperLayout, 1);
    mainLayout->addWidget(comboBox, 1, Qt::AlignHCenter);

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

void MainWindow::showTable(int currentIndex) {
    dataVector data;
    switch (currentIndex) {
        case 0:
            data = db.loadData<Qualification>();
            break;
        case 1:
            data = db.loadData<ContactDetails>();
            break;
        case 4:
            data = db.loadData<HomeOwnership>();
            break;
        default:
            //table->hide();
            table->setRowCount(0);
            table->setColumnCount(0);
            return;
    }

    table->setColumnCount(data[0]->getColumnCount());
    table->setHorizontalHeaderLabels(columnNames[currentIndex]);
    table->setRowCount(data.size());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    if (!data.empty()) {
        auto row = 0;
        for (auto& item: data) {
            for (auto col = 0; col < table->columnCount(); col++) {
                auto insertingItem = new QTableWidgetItem(item->getValue(col).toString());
                insertingItem->setTextAlignment(Qt::AlignCenter);
                table->setItem(row, col, insertingItem);
            }
            row++;
        }
    }
}

/*void MainWindow::makeTable(int index) {
    //table->disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::updateDataBase);
    bool wasBlocked = table->blockSignals(true);
    deletingButton->show();
    addingButton->show();
    //table->setRowCount(0);

    tableIndex = index;

    table->setColumnCount(queryInfo[tableIndex].second.size());
    table->setHorizontalHeaderLabels(queryInfo[tableIndex].second);

    auto r = db.selectPrintQuery(queryInfo[tableIndex].first);
    table->setRowCount(r.size());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for (int rowNum = 0; rowNum < table->rowCount(); rowNum++) {
        pqxx::row row = r[rowNum];

        *//*auto *//*lockedCols = getPrimaryKeysCols();
        //Если первичные ключи не найдены - мы считаем данную
        //таблицу таблицей производного запроса. Такие таблицы нельзя изменять,
        //они зависят от основных таблиц БД
        if (joinedColumns[tableIndex].size() == queryInfo[tableIndex].second.size())
            makeTableDerived(lockedCols);
        else {
            for (auto &i: joinedColumns[tableIndex])
                lockedCols.append(i);
        }

        auto boolCols = getBoolCols();

        for (int colNum = 0; colNum < table->columnCount(); colNum++) {
            pqxx::field field = row[colNum];
            if (field.is_null()) {
                QString str = "";
                auto item = new QTableWidgetItem(str);
                item->setTextAlignment(Qt::AlignCenter);
                table->setItem(rowNum, colNum, item);
                continue;
            }

            //Получаем строку из pqxx-запроса
            auto preStr = field.as<std::string>();
            //Напрямую из pqxx::field в QString записать значение нельзя, поэтому и создаём
            //переменную preStr
            QString str = QString::fromStdString(preStr);
            auto item = new QTableWidgetItem(str);

            //Проверяем, является ли столбец имеющим булевы значения,
            //чтобы заменить "t" и "f" на более понятное представление
            if (boolCols.contains(columnMatching[tableIndex][colNum])) {
                if (item->text() == "t")
                    item->setText("True");
                else
                    item->setText("False");
            }

            //Проверяем, не является ли столбец соответствующим первичному ключу
            if (lockedCols.contains(columnMatching[tableIndex][colNum])) {
                //Если является - то его делаем read-only
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                //table->mousePressEvent(nullptr);
            }
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(rowNum, colNum, item);
        }
    }

    //connect(table, &QTableWidget::itemChanged, this, &MainWindow::updateDataBase);
    table->blockSignals(wasBlocked);
}*/