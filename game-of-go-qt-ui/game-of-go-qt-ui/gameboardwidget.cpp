#include "gameboardwidget.h"
#include "ui_gameboardwidget.h"
#include <cmath>

GameBoardWidget::GameBoardWidget(int boardSize, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameBoardWidget)
    , boardSize(boardSize)
{
    ui->setupUi(this);
    setFixedWidth(fullWidth);
    setFixedHeight(fullWidth);

    cellWidth = (fullWidth - 2 * margin) / (boardSize - 1);
    lineWidth = 0.05 * cellWidth;
    stoneRadius = 0.45 * cellWidth;
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

void GameBoardWidget::drawStone(int color, QString coords, bool withMarker) {
    StoneWidget *stone = new StoneWidget(color, stoneRadius, withMarker, 1, this);
    QPoint point = coordsToPoint(coords);
    stone->setGeometry(point.x() - stoneRadius, point.y() - stoneRadius - 4 * eps, 2 * stoneRadius, 2 * stoneRadius);
    map[coords] = stone;
}

void GameBoardWidget::removeStone(QString coords) {
    if (map.count(coords) > 0) {
        map[coords]->hide();
        map.erase(coords);
    }
}

QString GameBoardWidget::pointToCoords(QPoint point) {
    int col = (int) round((point.x() - margin) / cellWidth);
    int row = boardSize - 1 - (int) round(point.y() - margin / cellWidth);

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
