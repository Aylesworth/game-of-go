/********************************************************************************
** Form generated from reading UI file 'gameboardwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMEBOARDWIDGET_H
#define UI_GAMEBOARDWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameBoardWidget
{
public:

    void setupUi(QWidget *GameBoardWidget)
    {
        if (GameBoardWidget->objectName().isEmpty())
            GameBoardWidget->setObjectName("GameBoardWidget");
        GameBoardWidget->resize(400, 400);

        retranslateUi(GameBoardWidget);

        QMetaObject::connectSlotsByName(GameBoardWidget);
    } // setupUi

    void retranslateUi(QWidget *GameBoardWidget)
    {
        GameBoardWidget->setWindowTitle(QCoreApplication::translate("GameBoardWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameBoardWidget: public Ui_GameBoardWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEBOARDWIDGET_H
