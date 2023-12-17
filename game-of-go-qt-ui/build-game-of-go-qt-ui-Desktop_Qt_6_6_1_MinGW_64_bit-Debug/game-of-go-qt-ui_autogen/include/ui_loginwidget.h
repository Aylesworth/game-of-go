/********************************************************************************
** Form generated from reading UI file 'loginwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINWIDGET_H
#define UI_LOGINWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginWidget
{
public:
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *lbl_username;
    QLineEdit *txt_username;
    QLabel *lbl_password;
    QLineEdit *txt_password;
    QLabel *label_3;
    QPushButton *pushButton;

    void setupUi(QWidget *LoginWidget)
    {
        if (LoginWidget->objectName().isEmpty())
            LoginWidget->setObjectName("LoginWidget");
        LoginWidget->resize(400, 300);
        formLayoutWidget = new QWidget(LoginWidget);
        formLayoutWidget->setObjectName("formLayoutWidget");
        formLayoutWidget->setGeometry(QRect(90, 100, 211, 81));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setVerticalSpacing(20);
        formLayout->setContentsMargins(0, 0, 0, 0);
        lbl_username = new QLabel(formLayoutWidget);
        lbl_username->setObjectName("lbl_username");

        formLayout->setWidget(0, QFormLayout::LabelRole, lbl_username);

        txt_username = new QLineEdit(formLayoutWidget);
        txt_username->setObjectName("txt_username");

        formLayout->setWidget(0, QFormLayout::FieldRole, txt_username);

        lbl_password = new QLabel(formLayoutWidget);
        lbl_password->setObjectName("lbl_password");

        formLayout->setWidget(1, QFormLayout::LabelRole, lbl_password);

        txt_password = new QLineEdit(formLayoutWidget);
        txt_password->setObjectName("txt_password");
        txt_password->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(1, QFormLayout::FieldRole, txt_password);

        label_3 = new QLabel(LoginWidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(170, 50, 61, 16));
        QFont font;
        font.setPointSize(18);
        label_3->setFont(font);
        label_3->setAlignment(Qt::AlignCenter);
        pushButton = new QPushButton(LoginWidget);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(160, 210, 80, 18));

        retranslateUi(LoginWidget);

        QMetaObject::connectSlotsByName(LoginWidget);
    } // setupUi

    void retranslateUi(QWidget *LoginWidget)
    {
        LoginWidget->setWindowTitle(QCoreApplication::translate("LoginWidget", "Form", nullptr));
        lbl_username->setText(QCoreApplication::translate("LoginWidget", "Username", nullptr));
        lbl_password->setText(QCoreApplication::translate("LoginWidget", "Password", nullptr));
        label_3->setText(QCoreApplication::translate("LoginWidget", "LOGIN", nullptr));
        pushButton->setText(QCoreApplication::translate("LoginWidget", "Login", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginWidget: public Ui_LoginWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWIDGET_H
