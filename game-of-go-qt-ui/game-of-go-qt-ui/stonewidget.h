#ifndef STONEWIDGET_H
#define STONEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>

namespace Ui {
class StoneWidget;
}

class StoneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StoneWidget(int color, int stoneRadius, bool withMarker = 1, qreal opacity = 1, QWidget *parent = nullptr);
    ~StoneWidget();

private:
    Ui::StoneWidget *ui;

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        QColor stoneColor(color == 1 ? "black" : "white");
        painter.setBrush(stoneColor);
        painter.setPen(stoneColor);
        painter.setOpacity(opacity);
        painter.drawEllipse(eps, eps, 2 * stoneRadius - 2 * eps, 2 * stoneRadius - 2 * eps);

        if (withMarker) {
            QColor markerColor(color == 1 ? "white" : "black");
            int markerRadius = 0.35 * stoneRadius;
            painter.setBrush(markerColor);
            painter.setPen(markerColor);
            painter.drawEllipse(stoneRadius - markerRadius, stoneRadius - markerRadius, 2 * markerRadius, 2 * markerRadius);
        }
    }

private:
    int color;
    int stoneRadius;
    bool withMarker;
    qreal opacity;
    int eps = 1;
};

#endif // STONEWIDGET_H
