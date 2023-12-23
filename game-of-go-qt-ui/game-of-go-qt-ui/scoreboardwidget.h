#ifndef SCOREBOARDWIDGET_H
#define SCOREBOARDWIDGET_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class ScoreboardWidget;
}

class ScoreboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScoreboardWidget(QWidget *parent = nullptr);
    ~ScoreboardWidget();
    int getBlackScore();
    int getWhiteScore();
    void setBlackScore(int score);
    void setWhiteScore(int score);

private:
    Ui::ScoreboardWidget *ui;
    int blackScore, whiteScore;
    QStandardItemModel *model;
};

#endif // SCOREBOARDWIDGET_H
