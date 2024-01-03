#include "playwidget.h"
#include "ui_playwidget.h"
#include "gameconfigwindow.h"
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
    boardSize = 13;
    komi = 6.5;
    timeSystem = 0;
    mainTimeSeconds = 60 * 60;
    byoyomiTimeSeconds = 30;
    byoyomiPeriods = 3;
    ranked = 1;

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &PlayWidget::onBoardSizeChanged);
    connect(MainWindow::getInstance(), SIGNAL(matchSetUp(QString)), this, SLOT(onMatchSetUp(QString)));
    connect(timer, &QTimer::timeout, this, &PlayWidget::onTimeout);
}

PlayWidget::~PlayWidget()
{
    delete ui;
}

void PlayWidget::onBoardSizeChanged(const QString &currentSize) {
    setBoardSize(currentSize.mid(0, currentSize.indexOf('x')).toInt());
}

void PlayWidget::on_btn_settings_clicked()
{
    GameConfigWindow *w = new GameConfigWindow(this);
    w->show();
}

void PlayWidget::on_btn_challenge_clicked()
{
    ChallengeWindow *w = new ChallengeWindow(this);
    w->show();
}

void PlayWidget::on_btn_cpu_clicked()
{
    socket->sendMessage("INVITE", QString("%1\n%2\n%3\n%4 %5 %6 %7\n%8\n")
                                      .arg("@CPU")
                                      .arg(boardSize)
                                      .arg(komi)
                                      .arg(timeSystem)
                                      .arg(timeSystem == 0 ? -1 : mainTimeSeconds)
                                      .arg(timeSystem == 0 ? -1 : byoyomiTimeSeconds)
                                      .arg(timeSystem == 0 ? -1 : byoyomiPeriods)
                                      .arg(ranked));
}

void PlayWidget::on_btn_quick_clicked()
{
    socket->sendMessage("MATCH", QString::number(boardSize) + "\n");
    waitingSeconds = 0;
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

int PlayWidget::getBoardSize() {
    return boardSize;
}

double PlayWidget::getKomi() {
    return komi;
}

int PlayWidget::getTimeSystem() {
    return timeSystem;
}

int PlayWidget::getMainTimeSeconds() {
    return mainTimeSeconds;
}

int PlayWidget::getByoyomiTimeSeconds() {
    return byoyomiTimeSeconds;
}

int PlayWidget::getByoyomiPeriods() {
    return byoyomiPeriods;
}

int PlayWidget::getRanked() {
    return ranked;
}

void PlayWidget::setBoardSize(int boardSize) {
    this->boardSize = boardSize;
    if (boardSize == 9 || boardSize == 13 || boardSize == 19) {
        ui->comboBox->setMaxCount(3);
        ui->comboBox->setCurrentText(QString("%1x%1").arg(boardSize));
    } else {
        ui->comboBox->setMaxCount(4);
        ui->comboBox->addItem(QString("%1x%1").arg(boardSize));
        ui->comboBox->setCurrentIndex(3);
    }
}

void PlayWidget::setKomi(double komi) {
    this->komi = komi;
}

void PlayWidget::setTimeSystem(int timeSystem) {
    this->timeSystem = timeSystem;
}

void PlayWidget::setMainTimeSeconds(int mainTimeSeconds) {
    this->mainTimeSeconds = mainTimeSeconds;
}

void PlayWidget::setByoyomiTimeSeconds(int byoyomiTimeSeconds) {
    this->byoyomiTimeSeconds = byoyomiTimeSeconds;
}

void PlayWidget::setByoyomiPeriods(int byoyomiPeriods) {
    this->byoyomiPeriods = byoyomiPeriods;
}

void PlayWidget::setRanked(int ranked) {
    this->ranked = ranked;
}

