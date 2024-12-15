#include <QApplication>
#include <QWidget>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    auto *mainWindow = new MainWindow("dbname=postgres user=postgres password=postgres port=5432");
    mainWindow->show();

    return app.exec();
}