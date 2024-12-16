#include <QScrollArea>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>

#include <iostream>

#include "mainwindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(const std::string& dataBaseParams, QWidget *parent) :
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
    connect(table, &QTableWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClick);
    connect(table, &QTableWidget::itemChanged, this, &MainWindow::updateDataBase);


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

QStringList MainWindow::getPrimaryKeys() {
    QStringList primaryKeysCols;

    std::string queryPrimaryKeys = "SELECT kcu.column_name\n"
                                   "FROM information_schema.table_constraints AS tc\n"
                                   "         JOIN information_schema.key_column_usage AS kcu\n"
                                   "              ON kcu.constraint_name = tc.constraint_name\n"
                                   "WHERE tc.table_name ='" + codesOfTables[tableIndex]
                                   + "' AND tc.constraint_type = 'PRIMARY KEY'\n"
                                     "ORDER BY kcu.column_name;";

    auto r = db.selectPrintQuery(queryPrimaryKeys);
    r.for_each([&primaryKeysCols] (const std::string& colName) {
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
    r.for_each([&booleanColumns] (const std::string& colName) {
        booleanColumns.append(QString::fromStdString(colName));
    });

    return booleanColumns;
}

void MainWindow::makeTable(int index) {
    tableIndex = index;

    table->setColumnCount(queryInfo[tableIndex].second.size());
    table->setHorizontalHeaderLabels(queryInfo[tableIndex].second);

    //auto r = w.exec(queryInfo[tableIndex].first);
    auto r = db.selectPrintQuery(queryInfo[tableIndex].first);
    table->setRowCount(r.size());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for (int rowNum = 0; rowNum < table->rowCount(); rowNum++) {
        pqxx::row row = r[rowNum];

        auto lockedCols = getPrimaryKeys();
        //Если первичные ключи не найдены - мы считаем данную
        //таблицу таблицей производного запроса. Такие таблицы нельзя изменять,
        //они зависят от основных таблиц БД
/*        if (lockedCols.empty()) {
            for (auto i = 0; i < row.size(); i++)
                lockedCols.push_back(i);
        }*/

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
            if (lockedCols.contains(columnMatching[tableIndex][colNum]))
                //Если является - то его делаем read-only
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(rowNum, colNum, item);
        }
    }
}

void MainWindow::onItemDoubleClick(QTableWidgetItem *item) {
    savedItemText = item->text();
}

void MainWindow::updateDataBase(QTableWidgetItem *item) {
    /*pqxx::work txn(c);

    int row = item->row();
    int col = item->column();
    QString newValue = item->text();

    QString id = table->item(row, 0)->text();
    QString columnName = queryInfo[tableIndex].second[col];

    bool isNumeric;
    int intNewValue = newValue.toInt(&isNumeric);

    QString query;
    try {
        if (newValue == "True" || newValue == "true")
            query = QString("UPDATE %1 SET %2 = TRUE WHERE id = %3").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, id);
        else if (newValue == "False" || newValue ==  "false")
            query = QString("UPDATE %1 SET %2 = FALSE WHERE id = %3").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, id);
        else if (isNumeric)
            query = QString("UPDATE %1 SET %2 = %3 WHERE id = %4").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, newValue, id);
        else
            query = QString("UPDATE %1 SET %2 = %3 WHERE id = %4").arg(
                    QString::fromStdString(codesOfTables[tableIndex]), columnName, newValue, id);

        txn.exec(query.toStdString());
        txn.commit();
    } catch (const std::exception &e) {
        // Обработка исключений
        //std::cerr << "Ошибка при обновлении данных: " << e.what() << std::endl;
        txn.abort();
        item->setText(savedItemText);
        QMessageBox::warning(this, "Ошибка", "Не удалось обновить данные: \n" + QString::fromStdString(e.what()));
    }*/

}



/*
int row = item->row();
int column = item->column();
QString newValue = item->text();

// Предположим, что у нас есть уникальный идентификатор в первой колонке
QString id = tableWidget->item(row, 0)->text();
QString columnName = getColumnName(column); // Получаем имя столбца

QString query;

private slots:
void onItemChanged(QTableWidgetItem *item) {
    int row = item->row();
    int column = item->column();
    QString newValue = item->text();

    // Предположим, что у нас есть уникальный идентификатор в первой колонке
    QString id = tableWidget->item(row, 0)->text();
    QString columnName = getColumnName(column); // Получаем имя столбца

    QString query;

    // Проверяем, является ли новое значение числом
    bool isNumeric;
    int intValue = newValue.toInt(&isNumeric); // Попробуем преобразовать в целое число

    try {
        if (newValue == "true" || newValue == "1") {
            query = QString("UPDATE tenants SET %1 = TRUE WHERE id = %2").arg(columnName, id);
        } else if (newValue == "false" || newValue == "0") {
            query = QString("UPDATE tenants SET %1 = FALSE WHERE id = %2").arg(columnName, id);
        } else if (isNumeric) {
            query = QString("UPDATE tenants SET %1 = %2 WHERE id = %3").arg(columnName, newValue, id);
        } else {
            query = QString("UPDATE tenants SET %1 = '%2' WHERE id = %3").arg(columnName, newValue, id);
        }

        dbManager.updateData(query.toStdString());
    } catch (const std::exception &e) {
        // Обработка исключений
        std::cerr << "Ошибка при обновлении данных: " << e.what() << std::endl;
        QMessageBox::warning(this, "Ошибка", "Не удалось обновить данные: " + QString::fromStdString(e.what()));
    }

    // Обновляем отображение значений в таблице
    updateTableDisplay();
}

private:
void updateTableDisplay() {
    // Получаем информацию о типах столбцов
    QStringList booleanColumns = getBooleanColumns();

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        for (int column = 0; column < tableWidget->columnCount(); ++column) {
            QTableWidgetItem *item = tableWidget->item(row, column);
            if (item) {
                QString columnName = getColumnName(column);
                QString value = item->text();

                // Проверяем, является ли текущий столбец булевым
                if (booleanColumns.contains(columnName)) {
                    if (value == "t") {
                        item->setText("Да"); // Заменяем 't' на "Да"
                    } else if (value == "f") {
                        item->setText("Нет"); // Заменяем 'f' на "Нет"
                    }
                }
            }
        }
    }
}*/

