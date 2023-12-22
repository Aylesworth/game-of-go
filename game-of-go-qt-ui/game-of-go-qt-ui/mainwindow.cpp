#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
#include "menuwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(new MenuWidget(this));
}

MainWindow::~MainWindow()
{
    delete ui;
}
