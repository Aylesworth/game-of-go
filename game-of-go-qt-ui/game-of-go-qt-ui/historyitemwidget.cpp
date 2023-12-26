#include "historyitemwidget.h"
#include "ui_historyitemwidget.h"

#include <QDateTime>

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
}

HistoryItemWidget::~HistoryItemWidget()
{
    delete ui;
}
