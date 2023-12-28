#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "playwidget.h"
#include "historywidget.h"
#include "rankingswidget.h"
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
    MainWindow::getInstance()->next(new PlayWidget());
}


void MenuWidget::on_btn_history_clicked()
{
    MainWindow::getInstance()->next(new HistoryWidget());
}


void MenuWidget::on_btn_rankings_clicked()
{
    MainWindow::getInstance()->next(new RankingsWidget());
}

