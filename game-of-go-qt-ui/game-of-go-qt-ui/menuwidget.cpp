#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "playwidget.h"
#include "historywidget.h"
#include "mainwindow.h"

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
    MainWindow *w = MainWindow::getInstance();
    w->next(new PlayWidget());
}


void MenuWidget::on_btn_history_clicked()
{
    MainWindow *w = MainWindow::getInstance();
    w->next(new HistoryWidget());
}

