#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>

namespace Ui {
class MenuWidget;
}

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);
    ~MenuWidget();

private slots:
    void on_btn_play_clicked();

    void on_btn_history_clicked();

private:
    Ui::MenuWidget *ui;
};

#endif // MENUWIDGET_H
