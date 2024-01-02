#ifndef GAMECONFIGWINDOW_H
#define GAMECONFIGWINDOW_H

#include "playwidget.h"
#include <QMainWindow>
#include <QAbstractButton>

namespace Ui {
class GameConfigWindow;
}

class GameConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameConfigWindow(QWidget *parent = nullptr);
    ~GameConfigWindow();

private slots:
    void onBoardSizeChanged(QAbstractButton *button);
    void onTimeSystemChanged(int index);
    void on_btn_ok_clicked();

private:
    Ui::GameConfigWindow *ui;
    PlayWidget *playWidget;
};

#endif // GAMECONFIGWINDOW_H
