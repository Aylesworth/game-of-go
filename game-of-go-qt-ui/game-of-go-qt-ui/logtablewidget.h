#ifndef LOGTABLEWIDGET_H
#define LOGTABLEWIDGET_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class LogTableWidget;
}

class LogTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogTableWidget(QWidget *parent = nullptr);
    ~LogTableWidget();
    void addRow(QString player, QString move);
    void removeLastRow();

private:
    Ui::LogTableWidget *ui;
    QStandardItemModel *model;
};

#endif // LOGTABLEWIDGET_H
