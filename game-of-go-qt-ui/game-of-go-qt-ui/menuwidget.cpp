#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "playwidget.h"
#include "historywidget.h"

#include <QMainWindow>

MenuWidget::MenuWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MenuWidget)
{
    ui->setupUi(this);
}

MenuWidget::~MenuWidget()
{
    delete ui;
}

void MenuWidget::on_btn_play_clicked()
{
    QMainWindow *parent = static_cast<QMainWindow *>(this->parentWidget());
    parent->setCentralWidget(new PlayWidget(parent));
}


void MenuWidget::on_btn_history_clicked()
{
    QMainWindow *parent = static_cast<QMainWindow *>(this->parentWidget());
    parent->setCentralWidget(new HistoryWidget(parent));
}

