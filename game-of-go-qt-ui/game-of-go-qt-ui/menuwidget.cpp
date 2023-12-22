#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "playwidget.h"

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
    QMainWindow *parent = static_cast<QMainWindow *>(this->parent());
    parent->setCentralWidget(new PlayWidget(parent));
}

