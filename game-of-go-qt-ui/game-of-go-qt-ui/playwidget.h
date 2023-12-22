#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>

namespace Ui {
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget *parent = nullptr);
    ~PlayWidget();
    int getSelectedBoardSize();

private slots:
    void on_btn_challenge_clicked();
    void onBoardSizeChanged(const QString &currentSize);

private:
    Ui::PlayWidget *ui;
    int selectedBoardSize;
};

#endif // PLAYWIDGET_H
