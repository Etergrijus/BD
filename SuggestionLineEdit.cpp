#include <QMessageBox>
#include <QStringListModel>
#include <QKeyEvent>
#include <utility>

#include "SuggestionLineEdit.h"

SuggestionLineEdit::SuggestionLineEdit(QString tableName, QString colName, QWidget *parent)
    : QLineEdit(parent), db("dbname=postgres user=postgres password=postgres port=5432"),
    tableName(std::move(tableName)), columnName(std::move(colName)) {
        completer = new QCompleter(this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        this->setCompleter(completer);
        connect(this, &QLineEdit::textChanged, this, &SuggestionLineEdit::updateSuggestions);
        connect(completer, QOverload<const QString&>::of(&QCompleter::activated),
                this, &SuggestionLineEdit::onCompleterActivated);
}

void SuggestionLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Сохранить данные в БД и закрыть LineEdit
        saveData();
        return; // Не передаем дальше, чтобы не вызывался стандартный обработчик
    }
    QLineEdit::keyPressEvent(event);
}

void SuggestionLineEdit::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !this->rect().contains(event->pos())) {
        saveData();
        return;
    }
    QLineEdit::mousePressEvent(event);
}

void SuggestionLineEdit::onCompleterActivated(const QString &text) {
    this->setText(text);
    saveData();
}

void SuggestionLineEdit::updateSuggestions(const QString &text) {
    bool isNumeric;
    int intNewValue = text.toInt(&isNumeric);
    QString query;
    if (isNumeric)
        query = QString("SELECT DISTINCT %1 FROM %2 WHERE CAST(%1 AS text) ILIKE '" + text + "%';").arg(columnName,
                                                                                                        tableName);
    else
        query = QString("SELECT DISTINCT %1 FROM %2 WHERE %1 ILIKE '" + text + "%';").arg(columnName,
                                                                                          tableName);
    QStringList suggestions;
    try {
        auto r = db.selectPrintQuery(query.toStdString());
        r.for_each([&suggestions](const std::string &suggestion) {
            suggestions.append(QString::fromStdString(suggestion));
        });
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Ошибка",
                              "Ошибка при запросе подсказок:\n" + QString::fromStdString(e.what()));
    }

    completer->setModel(new QStringListModel(suggestions, completer));
    if (!suggestions.isEmpty()) {
        completer->complete();
    }
}

void SuggestionLineEdit::saveData() {
    QString newValue = this->text();
    emit finished(newValue);
}