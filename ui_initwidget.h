/********************************************************************************
** Form generated from reading UI file 'initwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INITWIDGET_H
#define UI_INITWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InitWidget
{
public:
    QAction *action333;
    QGridLayout *gridLayout_2;
    QWidget *titleWidget;
    QGridLayout *gridLayout;
    QPushButton *btnMax;
    QLabel *textLogo;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnMin;
    QPushButton *btnClose;
    QLabel *textTitle;
    QMenuBar *menuBar;
    QHBoxLayout *horizontalLayout;
    QLabel *leftSide;
    QLabel *rightSide;
    QWidget *statusWidget;
    QGridLayout *gridLayout_3;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QLabel *textModuleStatus;
    QLabel *label;
    QLabel *textInfo;
    QLabel *textTime;

    void setupUi(QWidget *InitWidget)
    {
        if (InitWidget->objectName().isEmpty())
            InitWidget->setObjectName(QString::fromUtf8("InitWidget"));
        InitWidget->resize(1450, 902);
        InitWidget->setMinimumSize(QSize(1450, 600));
        InitWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
        InitWidget->setStyleSheet(QString::fromUtf8(""));
        action333 = new QAction(InitWidget);
        action333->setObjectName(QString::fromUtf8("action333"));
        gridLayout_2 = new QGridLayout(InitWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        titleWidget = new QWidget(InitWidget);
        titleWidget->setObjectName(QString::fromUtf8("titleWidget"));
        titleWidget->setMinimumSize(QSize(0, 40));
        titleWidget->setMaximumSize(QSize(16777215, 40));
        titleWidget->setStyleSheet(QString::fromUtf8(""));
        gridLayout = new QGridLayout(titleWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(-1, 5, -1, -1);
        btnMax = new QPushButton(titleWidget);
        btnMax->setObjectName(QString::fromUtf8("btnMax"));
        btnMax->setMinimumSize(QSize(0, 32));
        btnMax->setMaximumSize(QSize(32, 50));
        btnMax->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(btnMax, 0, 4, 1, 1);

        textLogo = new QLabel(titleWidget);
        textLogo->setObjectName(QString::fromUtf8("textLogo"));
        textLogo->setMinimumSize(QSize(40, 30));
        textLogo->setMaximumSize(QSize(40, 30));

        gridLayout->addWidget(textLogo, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 2, 1, 1);

        btnMin = new QPushButton(titleWidget);
        btnMin->setObjectName(QString::fromUtf8("btnMin"));
        btnMin->setMinimumSize(QSize(0, 32));
        btnMin->setMaximumSize(QSize(32, 32));
        btnMin->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(btnMin, 0, 3, 1, 1);

        btnClose = new QPushButton(titleWidget);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        btnClose->setMinimumSize(QSize(0, 32));
        btnClose->setMaximumSize(QSize(32, 32));
        btnClose->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(btnClose, 0, 5, 1, 1);

        textTitle = new QLabel(titleWidget);
        textTitle->setObjectName(QString::fromUtf8("textTitle"));

        gridLayout->addWidget(textTitle, 0, 1, 1, 1);


        gridLayout_2->addWidget(titleWidget, 0, 0, 1, 1);

        menuBar = new QMenuBar(InitWidget);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setMaximumSize(QSize(16777215, 40));
        menuBar->setNativeMenuBar(false);

        gridLayout_2->addWidget(menuBar, 1, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        leftSide = new QLabel(InitWidget);
        leftSide->setObjectName(QString::fromUtf8("leftSide"));
        leftSide->setMinimumSize(QSize(2, 0));
        leftSide->setMaximumSize(QSize(2, 16777215));
        leftSide->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));

        horizontalLayout->addWidget(leftSide);

        rightSide = new QLabel(InitWidget);
        rightSide->setObjectName(QString::fromUtf8("rightSide"));
        rightSide->setMinimumSize(QSize(2, 0));
        rightSide->setMaximumSize(QSize(2, 16777215));
        rightSide->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));

        horizontalLayout->addWidget(rightSide);


        gridLayout_2->addLayout(horizontalLayout, 2, 0, 1, 1);

        statusWidget = new QWidget(InitWidget);
        statusWidget->setObjectName(QString::fromUtf8("statusWidget"));
        statusWidget->setMinimumSize(QSize(0, 30));
        statusWidget->setMaximumSize(QSize(16777215, 30));
        statusWidget->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);\n"
"font: 9pt \"Consolas\";"));
        gridLayout_3 = new QGridLayout(statusWidget);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_5, 1, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 1, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_3, 1, 5, 1, 1);

        textModuleStatus = new QLabel(statusWidget);
        textModuleStatus->setObjectName(QString::fromUtf8("textModuleStatus"));

        gridLayout_3->addWidget(textModuleStatus, 1, 6, 1, 1);

        label = new QLabel(statusWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_3->addWidget(label, 1, 1, 1, 1);

        textInfo = new QLabel(statusWidget);
        textInfo->setObjectName(QString::fromUtf8("textInfo"));

        gridLayout_3->addWidget(textInfo, 1, 4, 1, 1);

        textTime = new QLabel(statusWidget);
        textTime->setObjectName(QString::fromUtf8("textTime"));

        gridLayout_3->addWidget(textTime, 1, 2, 1, 1);


        gridLayout_2->addWidget(statusWidget, 3, 0, 1, 1);


        retranslateUi(InitWidget);

        QMetaObject::connectSlotsByName(InitWidget);
    } // setupUi

    void retranslateUi(QWidget *InitWidget)
    {
        InitWidget->setWindowTitle(QCoreApplication::translate("InitWidget", "InitWidget", nullptr));
        action333->setText(QCoreApplication::translate("InitWidget", "333", nullptr));
        btnMax->setText(QString());
        textLogo->setText(QString());
        btnMin->setText(QString());
        btnClose->setText(QString());
        textTitle->setText(QCoreApplication::translate("InitWidget", "Solder Joint Inspection System", nullptr));
        leftSide->setText(QString());
        rightSide->setText(QString());
        textModuleStatus->setText(QCoreApplication::translate("InitWidget", "Initializing...", nullptr));
        label->setText(QCoreApplication::translate("InitWidget", "Current time\357\274\232", nullptr));
        textInfo->setText(QCoreApplication::translate("InitWidget", "Copyright \302\251 2020\342\200\2232022 Changsha Mingzhun Information Technology Co., Ltd.", nullptr));
        textTime->setText(QCoreApplication::translate("InitWidget", "2023/03/29 15\357\274\23252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InitWidget: public Ui_InitWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INITWIDGET_H
