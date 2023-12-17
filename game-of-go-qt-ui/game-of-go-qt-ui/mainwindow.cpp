#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gameboardwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(new GameBoardWidget(13, this));
}

MainWindow::~MainWindow()
{
    delete ui;
}
