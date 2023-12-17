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

    // StoneWidget *stone = new StoneWidget(2, stoneRadius, 1, 1, this);
    // stone->setGeometry(margin-stoneRadius, margin-stoneRadius, 2 * stoneRadius, 2 * stoneRadius);

    drawStone(1, "A13", 1);
    drawStone(2, "H8", 0);
    // removeStone("A13");
    // removeStone("H8");
}

GameBoardWidget::~GameBoardWidget()
{
    delete ui;
}

void GameBoardWidget::drawStone(int color, QString coords, bool withMarker) {
    StoneWidget *stone = new StoneWidget(color, stoneRadius, withMarker, 1, this);
    QPoint point = coordsToPoint(coords);
    stone->setGeometry(point.x() - stoneRadius, point.y() - stoneRadius - 4 * eps, 2 * stoneRadius, 2 * stoneRadius);
    map[coords] = stone;
    qDebug() << "Put " << coords;
}

void GameBoardWidget::removeStone(QString coords) {
    qDebug() << "Remove " << coords;
    if (map.count(coords) > 0) {
        qDebug() << map[coords]->x();
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
