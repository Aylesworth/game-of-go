#ifndef TERRITORYWIDGET_H
#define TERRITORYWIDGET_H

#include <QWidget>

namespace Ui {
class TerritoryWidget;
}

class TerritoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerritoryWidget(int color, int width, QWidget *parent = nullptr);
    ~TerritoryWidget();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::TerritoryWidget *ui;
    int color;
    int width;
};

#endif // TERRITORYWIDGET_H
