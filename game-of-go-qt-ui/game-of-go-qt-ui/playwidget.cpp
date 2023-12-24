#include "playwidget.h"
#include "ui_playwidget.h"
#include "challengewindow.h"

PlayWidget::PlayWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayWidget)
{
    ui->setupUi(this);
    ui->comboBox->addItems({"9x9", "13x13", "19x19"});
    ui->comboBox->setCurrentIndex(1);
    selectedBoardSize = 13;
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &PlayWidget::onBoardSizeChanged);
}

PlayWidget::~PlayWidget()
{
    delete ui;
}

void PlayWidget::on_btn_challenge_clicked()
{
    ChallengeWindow *w = new ChallengeWindow(this);
    w->show();
}

void PlayWidget::onBoardSizeChanged(const QString &currentSize) {
    selectedBoardSize = currentSize.mid(0, currentSize.indexOf('x')).toInt();
}

int PlayWidget::getSelectedBoardSize() {
    return selectedBoardSize;
}