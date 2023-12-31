#include "playwidget.h"
#include "ui_playwidget.h"
#include "challengewindow.h"
#include "mainwindow.h"
#include "socket.h"

#include <QMessageBox>
#include <QDebug>

PlayWidget::PlayWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayWidget)
    , socket(Socket::getInstance())
    , timer(new QTimer(this))
    , waitingSeconds(0)
    , waitingBox(nullptr)
{
    ui->setupUi(this);
    ui->comboBox->addItems({"9x9", "13x13", "19x19"});
    ui->comboBox->setCurrentIndex(1);
    selectedBoardSize = 13;
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &PlayWidget::onBoardSizeChanged);
    connect(MainWindow::getInstance(), SIGNAL(matchSetUp(QString)), this, SLOT(onMatchSetUp(QString)));
    connect(timer, &QTimer::timeout, this, &PlayWidget::onTimeout);
}

PlayWidget::~PlayWidget()
{
    delete ui;
}

void PlayWidget::onBoardSizeChanged(const QString &currentSize) {
    selectedBoardSize = currentSize.mid(0, currentSize.indexOf('x')).toInt();
}

int PlayWidget::getSelectedBoardSize() {
    return selectedBoardSize;
}

void PlayWidget::on_btn_challenge_clicked()
{
    ChallengeWindow *w = new ChallengeWindow(this);
    w->show();
}

void PlayWidget::on_btn_cpu_clicked()
{
    socket->sendMessage("INVITE", "@CPU\n" + QString::number(selectedBoardSize) + "\n");
}

void PlayWidget::on_btn_quick_clicked()
{
    socket->sendMessage("MATCH", QString::number(selectedBoardSize) + "\n");
    waitingBox = new QMessageBox(this);
    waitingBox->setIcon(QMessageBox::Information);
    waitingBox->setWindowTitle("Waiting");
    waitingBox->setText("Waiting for other players...\nElapsed time: 00:00");
    waitingBox->setStandardButtons(QMessageBox::Cancel);
    timer->start(1000);
    if (waitingBox->exec() == QMessageBox::Cancel) {
        socket->sendMessage("MATCCL");
    }
}

void PlayWidget::onTimeout() {
    waitingSeconds++;
    int minutes = waitingSeconds / 60;
    int seconds = waitingSeconds % 60;
    waitingBox->setText(QString::asprintf("Waiting for other players...\nElapsed time: %02d:%02d", minutes, seconds));
}

void PlayWidget::onMatchSetUp(QString opponent) {
    if (waitingBox != nullptr) {
        timer->stop();
        waitingSeconds = 0;
        waitingBox->done(QMessageBox::Close);
        waitingBox = nullptr;
        QMessageBox::information(this, "Message", "Found opponent: " + opponent);
    }
}

void PlayWidget::on_btn_leave_clicked()
{
    MainWindow::getInstance()->previous();
}

