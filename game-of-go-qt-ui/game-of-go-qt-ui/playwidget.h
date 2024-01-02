#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include "socket.h"
#include <QWidget>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget *parent = nullptr);
    ~PlayWidget();

private slots:
    void onBoardSizeChanged(const QString &currentSize);
    void onMatchSetUp(QString opponent);
    void onTimeout();
    void on_btn_settings_clicked();
    void on_btn_challenge_clicked();
    void on_btn_cpu_clicked();
    void on_btn_quick_clicked();
    void on_btn_leave_clicked();

private:
    Ui::PlayWidget *ui;
    Socket *socket;
    QTimer *timer;
    QMessageBox *waitingBox;
    int waitingSeconds;
    int boardSize;
    double komi;
    int timeSystem;
    int mainTimeSeconds;
    int byoyomiTimeSeconds;
    int byoyomiPeriods;
    int ranked;

public:
    int getBoardSize();
    double getKomi();
    int getTimeSystem();
    int getMainTimeSeconds();
    int getByoyomiTimeSeconds();
    int getByoyomiPeriods();
    int getRanked();
    void setBoardSize(int boardSize);
    void setKomi(double komi);
    void setTimeSystem(int timeSystem);
    void setMainTimeSeconds(int mainTimeSeconds);
    void setByoyomiTimeSeconds(int byoyomiTimeSeconds);
    void setByoyomiPeriods(int byoyomiPeriods);
    void setRanked(int ranked);
};

#endif // PLAYWIDGET_H
