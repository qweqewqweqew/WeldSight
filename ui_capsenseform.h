/********************************************************************************
** Form generated from reading UI file 'capsenseform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAPSENSEFORM_H
#define UI_CAPSENSEFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "imagezoomview.h"

QT_BEGIN_NAMESPACE

class Ui_CapSenseForm
{
public:
    QVBoxLayout *verticalLayout;
    ImageZoomView *img_view_1;
    QTableWidget *tableWidget;

    void setupUi(QWidget *CapSenseForm)
    {
        if (CapSenseForm->objectName().isEmpty())
            CapSenseForm->setObjectName(QString::fromUtf8("CapSenseForm"));
        CapSenseForm->resize(551, 725);
        verticalLayout = new QVBoxLayout(CapSenseForm);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        img_view_1 = new ImageZoomView(CapSenseForm);
        img_view_1->setObjectName(QString::fromUtf8("img_view_1"));
        img_view_1->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));

        verticalLayout->addWidget(img_view_1);

        tableWidget = new QTableWidget(CapSenseForm);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setMaximumSize(QSize(16777215, 16777215));

        verticalLayout->addWidget(tableWidget);

        verticalLayout->setStretch(0, 5);
        verticalLayout->setStretch(1, 3);

        retranslateUi(CapSenseForm);

        QMetaObject::connectSlotsByName(CapSenseForm);
    } // setupUi

    void retranslateUi(QWidget *CapSenseForm)
    {
        CapSenseForm->setWindowTitle(QCoreApplication::translate("CapSenseForm", "Form", nullptr));
        img_view_1->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CapSenseForm: public Ui_CapSenseForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPSENSEFORM_H
