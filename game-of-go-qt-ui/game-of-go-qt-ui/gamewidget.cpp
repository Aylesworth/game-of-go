#include "gamewidget.h"
#include "ui_gamewidget.h"
#include "gameboardwidget.h"

#include <QVBoxLayout>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
{
    ui->setupUi(this);
    // QVBoxLayout *layout = new QVBoxLayout();
    // layout->setAlignment(Qt::AlignHCenter);
    // layout->addWidget(new GameBoardWidget(13, this));

    ui->leftVBox->addWidget(new GameBoardWidget(13, this));
}

GameWidget::~GameWidget()
{
    delete ui;
}
