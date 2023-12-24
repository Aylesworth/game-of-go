#include "gameboardwidget.h"
#include "ui_gameboardwidget.h"
#include <cmath>

#include <QMouseEvent>
#include <QDebug>

GameBoardWidget::GameBoardWidget(int boardSize, int color, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameBoardWidget)
    , boardSize(boardSize)
    , playerColor(color)
{
    ui->setupUi(this);
    setFixedWidth(fullWidth);
    setFixedHeight(fullWidth);

    cellWidth = (fullWidth - 2 * margin) / (boardSize - 1);
    lineWidth = 0.05 * cellWidth;
    stoneRadius = 0.45 * cellWidth;
    shadowDisabled = color == 0;

    if (color > 0) {
        shadow = new StoneWidget(color, stoneRadius, 0, 0.5, this);
        shadow->hide();
    }
}

GameBoardWidget::~GameBoardWidget()
{
    delete ui;
}

void GameBoardWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    // Create a QPainter object
    QPainter painter(this);

    painter.setBrush(backgroundColor);
    painter.drawRect(0, 0, fullWidth, fullWidth);

    QPen pen;
    pen.setWidth(lineWidth);
    pen.setColor(lineColor);
    painter.setPen(pen);
    for (int i = 0; i < boardSize; i++) {
        int offset = margin + i * cellWidth;
        if (boardSize > 11) offset += i * (boardSize - 10) * 0.04;
        int eps = 2 * lineWidth * 0;
        painter.drawLine(offset, margin, offset, fullWidth - margin - eps);
        painter.drawLine(margin, offset, fullWidth - margin - eps, offset);
    }
}

void GameBoardWidget::mouseMoveEvent(QMouseEvent *event) {
    if (playerColor == 0 || shadowDisabled) return;
    moveStoneShadow(pointToCoords(QPoint(event->pos().x(), event->pos().y())));
}

void GameBoardWidget::mousePressEvent(QMouseEvent *event) {
    if (playerColor == 0) return;
    QString coords = pointToCoords(QPoint(event->pos().x(), event->pos().y()));
    if (stoneMap[coords] != nullptr) return;
    emit click(coords);
}

void GameBoardWidget::leaveEvent(QEvent *event) {
    setStoneShadowVisible(false);
}

void GameBoardWidget::drawStone(int color, QString coords, bool withMarker) {
    if (stoneMap[coords] != nullptr) {
        stoneMap[coords]->hide();
        stoneMap.erase(coords);
    }
    StoneWidget *stone = new StoneWidget(color, stoneRadius, withMarker, 1, this);
    QPoint point = coordsToPoint(coords);
    stone->setGeometry(point.x() - stoneRadius, point.y() - stoneRadius, 2 * stoneRadius, 2 * stoneRadius);
    stone->show();
    stoneMap[coords] = stone;
    update();
}

void GameBoardWidget::removeStones(QStringList coordsList) {
    for (QString coords: coordsList) {
        if (stoneMap[coords] != nullptr) {
            stoneMap[coords]->hide();
            stoneMap.erase(coords);
        }
    }
    update();
}

void GameBoardWidget::moveStoneShadow(QString coords) {
    if (stoneMap[coords] != nullptr) return;
    QPoint point = coordsToPoint(coords);
    shadow->setGeometry(point.x() - stoneRadius, point.y() - stoneRadius, 2 * stoneRadius, 2 * stoneRadius);
    shadow->show();
    update();
}

void GameBoardWidget::setStoneShadowVisible(bool visible) {
    if (visible) shadow->show();
    else shadow->hide();
    update();
}

void GameBoardWidget::setStoneShadowDisabled(bool disabled) {
    shadowDisabled = disabled;
    if (disabled) {
        shadow->hide();
    }
}

void GameBoardWidget::drawTerritory(int color, QStringList coordsList) {
    int width = cellWidth / 5;
    for (QString coords: coordsList) {
        TerritoryWidget *territory = new TerritoryWidget(color, width, this);
        QPoint point = coordsToPoint(coords);
        territory->setGeometry(point.x() - width / 2, point.y() - width / 2, width, width);
        territory->show();
        territoryMap[coords] = territory;
    }
    update();
}

QString GameBoardWidget::pointToCoords(QPoint point) {
    int x = point.x();
    int y = point.y();
    if (x < margin) x = margin;
    if (x > fullWidth - margin) x = fullWidth - margin;
    if (y < margin) y = margin;
    if (y > fullWidth - margin) y = fullWidth - margin;

    int col = (int) round(static_cast<double>(x - margin) / cellWidth);
    int row = boardSize - 1 - (int) round(static_cast<double>(y - margin) / cellWidth);

    char colChar = (char) (col + 'A');
    if (colChar >= 'I') colChar++;

    return QString(colChar) + QString::number(row + 1);
}

QPoint GameBoardWidget::coordsToPoint(QString coords) {
    char colChar = coords[0].toLatin1();
    if (colChar >= 'J') colChar--;

    int col = colChar - 'A';
    int row = coords.mid(1).toInt() - 1;

    double x = margin + col * cellWidth;
    double y = fullWidth - (margin + row * cellWidth);

    return QPoint(x, y);
}
