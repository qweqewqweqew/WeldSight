/********************************************************************************
** Form generated from reading UI file 'capsensetriform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAPSENSETRIFORM_H
#define UI_CAPSENSETRIFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CapSenseTriForm
{
public:
    QWidget *display_sidebar;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QGroupBox *gBox_config;
    QGroupBox *gBox_ok;
    QGroupBox *gBox_ng;
    QGroupBox *gBox_total;
    QSpacerItem *verticalSpacer;
    QCheckBox *cb_autoStart;
    QHBoxLayout *horizontalLayout;
    QPushButton *Bt_start;
    QPushButton *Bt_clear;

    void setupUi(QWidget *CapSenseTriForm)
    {
        if (CapSenseTriForm->objectName().isEmpty())
            CapSenseTriForm->setObjectName(QString::fromUtf8("CapSenseTriForm"));
        CapSenseTriForm->resize(1525, 797);
        CapSenseTriForm->setStyleSheet(QString::fromUtf8(""));
        display_sidebar = new QWidget(CapSenseTriForm);
        display_sidebar->setObjectName(QString::fromUtf8("display_sidebar"));
        display_sidebar->setGeometry(QRect(570, 130, 188, 264));
        gridLayout = new QGridLayout(display_sidebar);
        gridLayout->setSpacing(2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(2, 2, 2, 2);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gBox_config = new QGroupBox(display_sidebar);
        gBox_config->setObjectName(QString::fromUtf8("gBox_config"));
        gBox_config->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(gBox_config);

        gBox_ok = new QGroupBox(display_sidebar);
        gBox_ok->setObjectName(QString::fromUtf8("gBox_ok"));
        gBox_ok->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(gBox_ok);

        gBox_ng = new QGroupBox(display_sidebar);
        gBox_ng->setObjectName(QString::fromUtf8("gBox_ng"));
        gBox_ng->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(gBox_ng);

        gBox_total = new QGroupBox(display_sidebar);
        gBox_total->setObjectName(QString::fromUtf8("gBox_total"));
        gBox_total->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(gBox_total);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        cb_autoStart = new QCheckBox(display_sidebar);
        cb_autoStart->setObjectName(QString::fromUtf8("cb_autoStart"));
        cb_autoStart->setChecked(false);

        verticalLayout->addWidget(cb_autoStart);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        Bt_start = new QPushButton(display_sidebar);
        Bt_start->setObjectName(QString::fromUtf8("Bt_start"));
        Bt_start->setMinimumSize(QSize(0, 40));

        horizontalLayout->addWidget(Bt_start);

        Bt_clear = new QPushButton(display_sidebar);
        Bt_clear->setObjectName(QString::fromUtf8("Bt_clear"));
        Bt_clear->setMinimumSize(QSize(0, 40));

        horizontalLayout->addWidget(Bt_clear);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(CapSenseTriForm);

        QMetaObject::connectSlotsByName(CapSenseTriForm);
    } // setupUi

    void retranslateUi(QWidget *CapSenseTriForm)
    {
        CapSenseTriForm->setWindowTitle(QCoreApplication::translate("CapSenseTriForm", "Form", nullptr));
        gBox_config->setTitle(QCoreApplication::translate("CapSenseTriForm", "Configuration Parameters", nullptr));
        gBox_ok->setTitle(QCoreApplication::translate("CapSenseTriForm", "Yield Rate / Pass Rate", nullptr));
        gBox_ng->setTitle(QCoreApplication::translate("CapSenseTriForm", "Defect Count / Reject Count", nullptr));
        gBox_total->setTitle(QCoreApplication::translate("CapSenseTriForm", "Total Inspected / Total Tested", nullptr));
        cb_autoStart->setText(QCoreApplication::translate("CapSenseTriForm", "Auto Start", nullptr));
#if QT_CONFIG(tooltip)
        cb_autoStart->setToolTip(QCoreApplication::translate("CapSenseTriForm", "Automatically start detection when system is ready", nullptr));
#endif // QT_CONFIG(tooltip)
        Bt_start->setText(QCoreApplication::translate("CapSenseTriForm", "start", nullptr));
        Bt_clear->setText(QCoreApplication::translate("CapSenseTriForm", "clear statistics", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CapSenseTriForm: public Ui_CapSenseTriForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPSENSETRIFORM_H
