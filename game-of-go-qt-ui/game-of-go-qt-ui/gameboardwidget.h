#ifndef GAMEBOARDWIDGET_H
#define GAMEBOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QColor>
#include "stonewidget.h"
#include "territorywidget.h"

namespace Ui {
class GameBoardWidget;
}

class GameBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoardWidget(int boardSize, int color = 0, QWidget *parent = nullptr);
    ~GameBoardWidget();
    void drawStone(int color, QString coords, bool withMarker = false);
    void removeStones(QStringList coordsList);
    void moveStoneShadow(QString coords);
    void setStoneShadowVisible(bool visible);
    void setStoneShadowDisabled(bool disabled);
    void drawTerritory(int color, QStringList coordsList);

private:
    Ui::GameBoardWidget *ui;

signals:
    void click(QString coords);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int boardSize;
    int fullWidth = 720;
    int margin = 40;
    int cellWidth;
    int lineWidth;
    int stoneRadius;
    int eps = 1;
    int playerColor;
    QColor backgroundColor = QColor(215, 186, 137);
    QColor lineColor = QColor("brown");
    std::map<QString, StoneWidget *> stoneMap;
    std::map<QString, TerritoryWidget *> territoryMap;
    StoneWidget *shadow;
    bool shadowDisabled;

    QString pointToCoords(QPoint point);
    QPoint coordsToPoint(QString coords);
};

#endif // GAMEBOARDWIDGET_H
