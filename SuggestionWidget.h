#ifndef SUGGESTIONWIDGET_H
#define SUGGESTIONWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStringList>

class SuggestionWidget : public QWidget {
Q_OBJECT

public:
    SuggestionWidget(QWidget *parent = nullptr);
    void setSuggestions(const QStringList &suggestions);
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void suggestionSelected(const QString &suggestion);


private:
    QListWidget *listWidget;


};

#endif // SUGGESTIONWIDGET_H