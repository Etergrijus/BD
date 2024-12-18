#include "SuggestionWidget.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>

SuggestionWidget::SuggestionWidget(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint); // Сделать всплывающим

    listWidget = new QListWidget(this);
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    setLayout(layout);

    connect(listWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item){
        emit suggestionSelected(item->text());
        this->hide();
    });

}

void SuggestionWidget::setSuggestions(const QStringList &suggestions) {
    listWidget->clear();
    listWidget->addItems(suggestions);

    if (!suggestions.isEmpty()) {
        int maxHeight = 200; // Максимальная высота списка
        int itemCount = suggestions.size();
        int itemHeight = 20; // Высота элемента списка
        int height = std::min(maxHeight, itemCount * itemHeight + 2); //  +2 для border
        listWidget->setFixedHeight(height);

        int width = 200;
        this->setFixedWidth(width);
        this->adjustSize();
    }

}

void SuggestionWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
    }

    QWidget::keyPressEvent(event);
}

void SuggestionWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !this->rect().contains(event->pos())) {
        this->hide();
    }
    QWidget::mousePressEvent(event);
}