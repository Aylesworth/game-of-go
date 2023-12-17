#ifndef GAMEBOARDWIDGET_H
#define GAMEBOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QColor>
#include "stonewidget.h"

namespace Ui {
class GameBoardWidget;
}

class GameBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoardWidget(int boardSize, QWidget *parent = nullptr);
    ~GameBoardWidget();
    void drawStone(int color, QString coords, bool withMarker);
    void removeStone(QString coords);
    void moveStoneShadow(int color, QString coords);
    void hideStoneShadow(int color);

private:
    Ui::GameBoardWidget *ui;

protected:
    void paintEvent(QPaintEvent *event) override {
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
            int eps = 2 * lineWidth;
            painter.drawLine(offset, margin, offset, fullWidth - margin - eps);
            painter.drawLine(margin, offset, fullWidth - margin - eps, offset);
        }
    }

private:
    int boardSize;
    int fullWidth = 720;
    int margin = 40;
    int cellWidth;
    int lineWidth;
    int stoneRadius;
    int eps = 1;
    QColor backgroundColor = QColor(215, 186, 137);
    QColor lineColor = QColor("brown");
    std::map<QString, StoneWidget *> map;

    QString pointToCoords(QPoint point);
    QPoint coordsToPoint(QString coords);
};

#endif // GAMEBOARDWIDGET_H
