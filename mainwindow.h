#ifndef DATABASE_1_MAINWINDOW_H
#define DATABASE_1_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QTableWidget>
#include <pqxx/pqxx>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const std::string& dataBaseParams, QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    pqxx::connection c;
    pqxx::work w;

    std::map<int, std::pair<std::string, QStringList>> queryInfo;

    QWidget *mainWidget;

    QComboBox *comboBox;
    QTableWidget *table;
    QVBoxLayout *mainLayout;

    void getPrimaryKeys();

private slots:
    void makeTable(int index);

    //void
};


#endif //DATABASE_1_MAINWINDOW_H
