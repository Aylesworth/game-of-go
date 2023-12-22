#include "playwidget.h"
#include "ui_playwidget.h"

PlayWidget::PlayWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayWidget)
{
    ui->setupUi(this);
}

PlayWidget::~PlayWidget()
{
    delete ui;
}
