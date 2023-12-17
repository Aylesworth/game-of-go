#include "stonewidget.h"
#include "ui_stonewidget.h"

StoneWidget::StoneWidget(int color, int stoneRadius, bool withMarker, qreal opacity, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StoneWidget)
    , stoneRadius(stoneRadius)
    , color(color)
    , withMarker(withMarker)
    , opacity(opacity)
{
    ui->setupUi(this);
}

StoneWidget::~StoneWidget()
{
    delete ui;
}
