#include "gameconfigwindow.h"
#include "ui_gameconfigwindow.h"
#include "mainwindow.h"

#include <QButtonGroup>
#include <QAbstractButton>
#include <QDebug>

GameConfigWindow::GameConfigWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameConfigWindow)
{
    playWidget = static_cast<PlayWidget *>(parent);
    ui->setupUi(this);
    ui->formLayout->setSpacing(8);

    QButtonGroup *sizeGroup = new QButtonGroup(this);
    sizeGroup->addButton(ui->choice_9);
    sizeGroup->addButton(ui->choice_13);
    sizeGroup->addButton(ui->choice_19);
    sizeGroup->addButton(ui->choice_custom);
    ui->inp_size->setDisabled(true);
    switch(playWidget->getBoardSize()) {
    case 9:
        ui->choice_9->setChecked(true);
        break;
    case 13:
        ui->choice_13->setChecked(true);
        break;
    case 19:
        ui->choice_19->setChecked(true);
        break;
    default:
        ui->choice_custom->setChecked(true);
        ui->inp_size->setDisabled(false);
        ui->inp_size->setValue(playWidget->getBoardSize());
        break;
    }

    ui->time_control->addItems({"None", "Byo-yomi"});
    ui->time_control->setCurrentIndex(playWidget->getTimeSystem());

    ui->inp_main_time->setValue(playWidget->getMainTimeSeconds() / 60);
    ui->inp_byo_time->setValue(playWidget->getByoyomiTimeSeconds());
    ui->inp_byo_periods->setValue(playWidget->getByoyomiPeriods());
    if (ui->time_control->currentIndex() == 0) {
        ui->inp_main_time->setDisabled(true);
        ui->inp_byo_time->setDisabled(true);
        ui->inp_byo_periods->setDisabled(true);
    }

    QButtonGroup *rankedGroup = new QButtonGroup(this);
    rankedGroup->addButton(ui->choice_yes);
    rankedGroup->addButton(ui->choice_no);
    if (playWidget->getRanked() == 1) {
        ui->choice_yes->setChecked(true);
    } else {
        ui->choice_no->setChecked(true);
    }

    connect(sizeGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(onBoardSizeChanged(QAbstractButton *)));
    connect(ui->time_control, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimeSystemChanged(int)));
    connect(MainWindow::getInstance(), SIGNAL(matchSetUp()), this, SLOT(hide()));
}

GameConfigWindow::~GameConfigWindow()
{
    delete ui;
}

void GameConfigWindow::onBoardSizeChanged(QAbstractButton *button) {
    ui->inp_size->setEnabled(button->text().startsWith("Custom"));
}

void GameConfigWindow::onTimeSystemChanged(int index) {
    bool isNone = index == 0;
    ui->inp_main_time->setDisabled(isNone);
    ui->inp_byo_time->setDisabled(isNone);
    ui->inp_byo_periods->setDisabled(isNone);
}


void GameConfigWindow::on_btn_ok_clicked()
{
    if (ui->choice_custom->isChecked()) {
        playWidget->setBoardSize(ui->inp_size->value());
    } else {
        if (ui->choice_9->isChecked()) playWidget->setBoardSize(9);
        else if (ui->choice_13->isChecked()) playWidget->setBoardSize(13);
        else if (ui->choice_19->isChecked()) playWidget->setBoardSize(19);
    }

    playWidget->setKomi(ui->inp_komi->value());
    playWidget->setTimeSystem(ui->time_control->currentIndex());
    playWidget->setMainTimeSeconds(ui->inp_main_time->value() * 60);
    playWidget->setByoyomiTimeSeconds(ui->inp_byo_time->value());
    playWidget->setByoyomiPeriods(ui->inp_byo_periods->value());
    playWidget->setRanked(ui->choice_yes->isChecked() ? 1 : 0);

    this->hide();
}

