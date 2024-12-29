#ifndef DATABASE_1_MAINWINDOW_H
#define DATABASE_1_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>

#include "DataBaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(const QString &dataBaseName, QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    QWidget *mainWidget;

    QComboBox *comboBox;
    QTableWidget *table;
    QVBoxLayout *mainLayout;

    int tableIndex;

    DataBaseManager db;

    std::map<int, QStringList> columnNames = {{0, {"Id", "Описание"}},
                                              {1, {"Id", "Телефон", "ФИО"}},
                                              {4, {"Id", "Параметры строения", "Доп. Информация"}}};

public slots:
    void showTable(int currentIndex);
};


#endif //DATABASE_1_MAINWINDOW_H
