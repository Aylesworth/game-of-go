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
    void disableTimeControl();
    void disableByoyomi();
    void setKomi(double komi);
    QString getBlackName();
    QString getWhiteName();
    void setBlackName(QString name);
    void setWhiteName(QString name);
    int getBlackCaptures();
    int getWhiteCaptures();
    void setBlackCaptures(int captures);
    void setWhiteCaptures(int captures);
    int getBlackMainTime();
    int getWhiteMainTime();
    void setBlackMainTime(int mainTime);
    void setWhiteMainTime(int mainTime);
    int getBlackByoyomiTime();
    int getWhiteByoyomiTIme();
    void setBlackByoyomiTime(int byoyomiTime);
    void setWhiteByoyomiTime(int byoyomiTime);
    int getBlackByoyomiPeriods();
    int getWhiteByoyomiPeriods();
    void setBlackByoyomiPeriods(int byoyomiPeriods);
    void setWhiteByoyomiPeriods(int byoyomiPeriods);

private:
    Ui::ScoreboardWidget *ui;
    QString blackName, whiteName;
    int blackCaptures, whiteCaptures;
    int blackMainTime, whiteMainTime;
    int blackByoyomiTime, whiteByoyomiTime;
    int blackByoyomiPeriods, whiteByoyomiPeriods;
    QStandardItemModel *model;
};

#endif // SCOREBOARDWIDGET_H
