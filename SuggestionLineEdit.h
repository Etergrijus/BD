#ifndef DATABASE_1_SUGGESTIONLINEEDIT_H
#define DATABASE_1_SUGGESTIONLINEEDIT_H


#include <QLineEdit>
#include <QCompleter>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QWidget>

#include "DataBaseHandler.h"

class SuggestionLineEdit : public QLineEdit {
Q_OBJECT

public:
    SuggestionLineEdit(QString tableName, QString colName, QWidget* parent = nullptr);

private slots:
    void updateSuggestions(const QString& text);

    void onCompleterActivated(const QString& text);

protected:
    void keyPressEvent(QKeyEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

signals:
    void finished(QString text);

private:
    DataBaseHandler db;
    QCompleter* completer;
    QString tableName;
    QString columnName;

    void saveData();
};


#endif //DATABASE_1_SUGGESTIONLINEEDIT_H
