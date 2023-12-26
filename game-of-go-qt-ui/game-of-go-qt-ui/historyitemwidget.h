#ifndef HISTORYITEMWIDGET_H
#define HISTORYITEMWIDGET_H

#include <QWidget>

namespace Ui {
class HistoryItemWidget;
}

class HistoryItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryItemWidget(
        QString id,
        int boardSize,
        int color,
        QString opponent,
        float blackScore,
        float whiteScore,
        int64_t time,
        QWidget *parent = nullptr);
    ~HistoryItemWidget();

private:
    Ui::HistoryItemWidget *ui;
};

#endif // HISTORYITEMWIDGET_H
