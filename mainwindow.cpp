#include <QScrollArea>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

#include <iostream>

#include "mainwindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(const std::string &dataBaseParams, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow), db(dataBaseParams) {
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

    deletingButton = new QPushButton(this);
    deletingButton->setFixedSize(60, 60);
    deletingButton->setIcon(QIcon(R"(D:\CLionProjects\Database_1\Icons\Delete.png)"));
    deletingButton->setIconSize(deletingButton->size());
    deletingButton->setStyleSheet("QPushButton {border : 4px solid black}");
    connect(deletingButton, &QPushButton::clicked, this, &MainWindow::deleteRows);

    auto upperLayout = new QHBoxLayout;
    upperLayout->addStretch();
    upperLayout->addWidget(comboBox);
    upperLayout->addWidget(deletingButton);

    upperLayout->addStretch();

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addLayout(upperLayout, 1);

    connect(comboBox, &QComboBox::currentIndexChanged, this,
            [this]() { MainWindow::makeTable(comboBox->currentIndex()); });


    table = new QTableWidget;
    table->setStyleSheet("QTableWidget { border: 2px solid black; }"
                         "QTableWidget::item { border: 1px solid black; }"
                         "QHeaderView::section { background-color: lightgray; }");
    table->setWordWrap(true);
    table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //connect(table, &QTableWidget::cellChanged, this, &MainWindow::onCellChanged);
    connect(table, &QTableWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClick);
    connect(table, &QTableWidget::itemChanged, this, &MainWindow::updateDataBase);
    connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onCellDoubleClicked);

/*    suggestionWidget = new SuggestionWidget(this);
    suggestionWidget->hide();
    connect(suggestionWidget, &SuggestionWidget::suggestionSelected, this, &MainWindow::handleSuggestionSelected);*/

    auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumSize(680, 400);
    scrollArea->setWidget(table);
    mainLayout->addWidget(scrollArea, 3);


    addingButton = new QPushButton(this);
    addingButton->setFixedSize(60, 60);
    addingButton->setIcon(QIcon(R"(D:\CLionProjects\Database_1\Icons\Add.png)"));
    addingButton->setIconSize(deletingButton->size());
    addingButton->setStyleSheet("QPushButton {border : 4px solid black}");
    connect(addingButton, &QPushButton::clicked, this, &MainWindow::addRow);
    mainLayout->addWidget(addingButton, 1, Qt::AlignHCenter);

    this->setLayout(mainLayout);

/*    suggestionTimer = new QTimer(this);
    suggestionTimer->setSingleShot(true);
    connect(suggestionTimer, &QTimer::timeout, [this]() {
        querySuggestions(table->currentItem()->row(), table->currentItem()->column(),
                         table->item(table->currentItem()->row(), table->currentItem()->column())->text());
    });*/
}

MainWindow::~MainWindow() {
    delete ui;
}

QStringList MainWindow::getPrimaryKeysCols() {
    QStringList primaryKeysCols;

    std::string queryPrimaryKeys = "SELECT kcu.column_name\n"
                                   "FROM information_schema.table_constraints AS tc\n"
                                   "         JOIN information_schema.key_column_usage AS kcu\n"
                                   "              ON kcu.constraint_name = tc.constraint_name\n"
                                   "WHERE tc.table_name ='" + codesOfTables[tableIndex]
                                   + "' AND tc.constraint_type = 'PRIMARY KEY'\n"
                                     "ORDER BY kcu.column_name;";

    auto r = db.selectPrintQuery(queryPrimaryKeys);
    r.for_each([&primaryKeysCols](const std::string &colName) {
        primaryKeysCols.append(QString::fromStdString(colName));
    });

    return primaryKeysCols;
}

QStringList MainWindow::getBoolCols() {
    QStringList booleanColumns;
    // Выполняем SQL-запрос для получения информации о столбцах
    std::string boolColsQuery = "SELECT column_name FROM information_schema.columns\n"
                                "WHERE table_name = '" + codesOfTables[tableIndex] + "' AND data_type = 'boolean'";

    auto r = db.selectPrintQuery(boolColsQuery);
    r.for_each([&booleanColumns](const std::string &colName) {
        booleanColumns.append(QString::fromStdString(colName));
    });

    return booleanColumns;
}

void MainWindow::makeTableDerived(QStringList &cols) {
    for (auto &i: joinedColumns[tableIndex])
        cols.append(i);
    deletingButton->hide();
    addingButton->hide();
}

void MainWindow::makeTable(int index) {
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

        /*auto */lockedCols = getPrimaryKeysCols();
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
}

void MainWindow::onItemDoubleClick(QTableWidgetItem *item) {
    savedItemText = item->text();
}

void MainWindow::updateDataBase(QTableWidgetItem *item) {
    int row = item->row();
    int col = item->column();
    QString newValue = item->text();

    QString id = table->item(row, 0)->text();
    QString idColName = columnMatching[tableIndex][0];
    QString columnName = columnMatching[tableIndex][col];

    bool isNumeric;
    bool isIdColNumeric;
    int intNewValue = newValue.toInt(&isNumeric);
    intNewValue = id.toInt(&isIdColNumeric);

    QString query;
    try {
        if (newValue == "True" || newValue == "true")
            query = QString("UPDATE %1 SET %2 = TRUE WHERE %3 = %4").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, idColName, id);
        else if (newValue == "False" || newValue == "false")
            query = QString("UPDATE %1 SET %2 = FALSE WHERE %3 = %4").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, idColName, id);
        else if (isNumeric)
            query = QString("UPDATE %1 SET %2 = %3 WHERE %4 = %5").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, newValue, idColName, id);
        else
            query = QString("UPDATE %1 SET %2 = '%3' WHERE %4 = %5").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, newValue, idColName, id);

        if (!isIdColNumeric) {
            query.insert(std::distance(query.begin(), query.end() - id.size()), "'");
            query.insert(std::distance(query.begin(), query.end()), "'");
        }

        db.selectPrintQuery(query.toStdString());
    } catch (const std::exception &e) {
        item->setText(savedItemText);
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные: \n" +
                                              QString::fromStdString(e.what()));
    }
}

void MainWindow::deleteRows() {
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();

    while (!selectedRows.empty()) {
        QString id = table->item(selectedRows[0].row(), 0)->text();
        QString idColName = columnMatching[tableIndex][0];

        bool isIdColNumeric;
        int intNewValue = id.toInt(&isIdColNumeric);

        QString query;
        try {
            query = QString("DELETE FROM %1 WHERE %2 = %3").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), idColName, id);
            if (!isIdColNumeric) {
                query.insert(std::distance(query.begin(), query.end() - id.size()), "'");
                query.insert(std::distance(query.begin(), query.end()), "'");
            }

            db.selectPrintQuery(query.toStdString());
            table->removeRow(selectedRows[0].row());
            selectedRows = table->selectionModel()->selectedRows();
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить данные: \n" +
                                                  QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::addRow() {
    bool wasBlocked = table->blockSignals(true);

    auto nRows = table->rowCount();
    table->setRowCount(nRows + 1);
    for (auto i = 0; i < table->columnCount(); i++) {
        auto item = new QTableWidgetItem("");
        item->setTextAlignment(Qt::AlignCenter);
        table->setItem(nRows, i, item);
    }

    auto primaryKeys = getPrimaryKeysCols();
    for (auto i = 0; i < table->columnCount(); i++)
        if (primaryKeys.contains(columnMatching[tableIndex][i]))
            table->openPersistentEditor(table->item(nRows, i));


    table->blockSignals(wasBlocked);


    if (!joinedColumns[tableIndex].empty()) {
        auto query = "SELECT\n"
                     "    kcu.column_name,\n"
                     "    ccu.table_name AS referenced_table_name,\n"
                     "FROM\n"
                     "    information_schema.table_constraints AS tc\n"
                     "JOIN\n"
                     "    information_schema.key_column_usage AS kcu\n"
                     "    ON tc.constraint_name = kcu.constraint_name\n"
                     "    AND tc.table_schema = kcu.table_schema\n"
                     "JOIN\n"
                     "    information_schema.referential_constraints AS rc\n"
                     "    ON tc.constraint_name = rc.constraint_name\n"
                     "    AND tc.table_schema = rc.constraint_schema\n"
                     "JOIN\n"
                     "    information_schema.constraint_column_usage AS ccu\n"
                     "    ON rc.unique_constraint_name = ccu.constraint_name\n"
                     "WHERE\n"
                     "    tc.constraint_type = 'FOREIGN KEY'\n"
                     "    AND tc.table_name = '" + codesOfTables[tableIndex] + "'";
    }
}



void MainWindow::onCellDoubleClicked(int row, int col) {
    if (lockedCols.contains(columnMatching[tableIndex][col]))
        return;

    SuggestionLineEdit* lineEdit = new SuggestionLineEdit(QString::fromStdString(codesOfTables[tableIndex]),
                                                          columnMatching[tableIndex][col], table);
    lineEdit->setText(table->item(row, col)->text());
    table->setCellWidget(row, col, lineEdit);
    lineEdit->setFocus();
    connect(lineEdit, &SuggestionLineEdit::finished, this, &MainWindow::closeEditor);
}

void MainWindow::closeEditor(const QString& text) {
    table->item(table->currentRow(), table->currentColumn())->setText(text);
    table->setCellWidget(table->currentRow(), table->currentColumn(), nullptr);
}