#include "historyitemwidget.h"
#include "ui_historyitemwidget.h"
#include "mainwindow.h"
#include "replaywidget.h"
#include "stonewidget.h"
#include "socket.h"

#include <QDateTime>
#include <QDebug>

HistoryItemWidget::HistoryItemWidget(
        QString id,
        int boardSize,
        int color,
        QString opponent,
        float blackScore,
        float whiteScore,
        int64_t time,
        QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryItemWidget)
    , socket(Socket::getInstance())
    , id(id)
    , boardSize(boardSize)
    , color(color)
    , blackScore(blackScore)
    , whiteScore(whiteScore)
{
    setFixedSize(720, 120);
    ui->setupUi(this);

    bool victory = color == 1 ? blackScore > whiteScore : whiteScore > blackScore;

    ui->lbl_id->setText("#" + id);
    ui->lbl_boardsize->setText(QString("%1x%1").arg(boardSize));
    ui->lbl_opponent->setText("vs. " + opponent);
    ui->lbl_result->setText(victory ? "VICTORY" : "DEFEAT");
    ui->lbl_result->setStyleSheet(QString("color: %1;").arg(victory ? "DARKBLUE" : "DARKRED"));
    ui->lbl_scores->setText(QString("Black score: %1\nWhite score: %2").arg(blackScore).arg(whiteScore));
    ui->lbl_time->setText(QDateTime::fromSecsSinceEpoch(time).toString("hh:mm dd/MM/yyyy"));

    StoneWidget *stone = new StoneWidget(color, 25, false, 1, this);
    stone->setGeometry(20, 40, 60, 60);
    stone->show();

    connect(socket, &Socket::messageReceived, this, &HistoryItemWidget::onMessageReceived);
}

HistoryItemWidget::~HistoryItemWidget()
{
    delete ui;
}

void HistoryItemWidget::on_btn_record_clicked()
{
    socket->sendMessage("REPLAY", id + "\n");
}

void HistoryItemWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "REPLAY") {
        QStringList params = payload.split("\n");
        QString id = params[0];
        if (id != this->id) return;

        QStringList log = params[1].split(" ", Qt::SkipEmptyParts);
        QStringList blackTerritory = params[2].split(" ", Qt::SkipEmptyParts);
        QStringList whiteTerritory = params[3].split(" ", Qt::SkipEmptyParts);

        MainWindow *w = MainWindow::getInstance();
        w->next(new ReplayWidget(
            boardSize,
            color,
            blackScore,
            whiteScore,
            log,
            blackTerritory,
            whiteTerritory
        ));
        return;
    }
}

