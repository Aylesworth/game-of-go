/********************************************************************************
** Form generated from reading UI file 'stonewidget.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STONEWIDGET_H
#define UI_STONEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StoneWidget
{
public:

    void setupUi(QWidget *StoneWidget)
    {
        if (StoneWidget->objectName().isEmpty())
            StoneWidget->setObjectName("StoneWidget");
        StoneWidget->resize(400, 300);

        retranslateUi(StoneWidget);

        QMetaObject::connectSlotsByName(StoneWidget);
    } // setupUi

    void retranslateUi(QWidget *StoneWidget)
    {
        StoneWidget->setWindowTitle(QCoreApplication::translate("StoneWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StoneWidget: public Ui_StoneWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STONEWIDGET_H
