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
    explicit StoneWidget(int color, int stoneRadius, bool withMarker = false, qreal opacity = 1, QWidget *parent = nullptr);
    ~StoneWidget();

private:
    Ui::StoneWidget *ui;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int color;
    int stoneRadius;
    bool withMarker;
    qreal opacity;
    int eps = 1;
};

#endif // STONEWIDGET_H
