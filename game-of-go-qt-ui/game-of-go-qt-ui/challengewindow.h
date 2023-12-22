#ifndef CHALLENGEWINDOW_H
#define CHALLENGEWINDOW_H

#include "socket.h"
#include <QMainWindow>

namespace Ui {
class ChallengeWindow;
}

class ChallengeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChallengeWindow(QWidget *parent = nullptr);
    ~ChallengeWindow();

private:
    Ui::ChallengeWindow *ui;
    Socket *socket;

private slots:
    void onMessageReceived(QString msgtype, QString payload);
    void onPlayerChanged(const QString &currentPlayer);
    void on_btn_challenge_clicked();
};

#endif // CHALLENGEWINDOW_H
