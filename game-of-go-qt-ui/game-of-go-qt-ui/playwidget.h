#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>

namespace Ui {
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget *parent = nullptr);
    ~PlayWidget();

private:
    Ui::PlayWidget *ui;
};

#endif // PLAYWIDGET_H
