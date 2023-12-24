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

void StoneWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QColor stoneColor(color == 1 ? "black" : "white");
    painter.setBrush(stoneColor);
    painter.setPen(stoneColor);
    painter.setOpacity(opacity);
    painter.drawEllipse(0, 0, 2 * stoneRadius, 2 * stoneRadius);

    if (withMarker) {
        QColor markerColor(color == 1 ? "white" : "black");
        int markerRadius = 0.35 * stoneRadius;
        painter.setBrush(markerColor);
        painter.setPen(markerColor);
        painter.drawEllipse(stoneRadius - markerRadius, stoneRadius - markerRadius, 2 * markerRadius, 2 * markerRadius);
    }
}
