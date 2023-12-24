#include "territorywidget.h"
#include "ui_territorywidget.h"

#include <QPainter>

TerritoryWidget::TerritoryWidget(int color, int width, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TerritoryWidget)
    , color(color)
    , width(width)
{
    ui->setupUi(this);
}

TerritoryWidget::~TerritoryWidget()
{
    delete ui;
}

void TerritoryWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QColor fillColor(color == 1 ? "black" : "white");
    painter.setBrush(fillColor);
    painter.setPen(fillColor);
    painter.drawRect(0, 0, width, width);
}
