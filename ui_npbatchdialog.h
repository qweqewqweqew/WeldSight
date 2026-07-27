/********************************************************************************
** Form generated from reading UI file 'npbatchdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NPBATCHDIALOG_H
#define UI_NPBATCHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NpBatchDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupN;
    QVBoxLayout *verticalLayout_2;
    QPushButton *btnLoadN;
    QListWidget *listN;
    QGroupBox *groupP;
    QVBoxLayout *verticalLayout_3;
    QPushButton *btnLoadP;
    QListWidget *listP;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnStart;
    QPushButton *btnCancel;

    void setupUi(QDialog *NpBatchDialog)
    {
        if (NpBatchDialog->objectName().isEmpty())
            NpBatchDialog->setObjectName(QString::fromUtf8("NpBatchDialog"));
        NpBatchDialog->resize(600, 400);
        verticalLayout = new QVBoxLayout(NpBatchDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupN = new QGroupBox(NpBatchDialog);
        groupN->setObjectName(QString::fromUtf8("groupN"));
        verticalLayout_2 = new QVBoxLayout(groupN);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        btnLoadN = new QPushButton(groupN);
        btnLoadN->setObjectName(QString::fromUtf8("btnLoadN"));

        verticalLayout_2->addWidget(btnLoadN);

        listN = new QListWidget(groupN);
        listN->setObjectName(QString::fromUtf8("listN"));

        verticalLayout_2->addWidget(listN);


        horizontalLayout->addWidget(groupN);

        groupP = new QGroupBox(NpBatchDialog);
        groupP->setObjectName(QString::fromUtf8("groupP"));
        verticalLayout_3 = new QVBoxLayout(groupP);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        btnLoadP = new QPushButton(groupP);
        btnLoadP->setObjectName(QString::fromUtf8("btnLoadP"));

        verticalLayout_3->addWidget(btnLoadP);

        listP = new QListWidget(groupP);
        listP->setObjectName(QString::fromUtf8("listP"));

        verticalLayout_3->addWidget(listP);


        horizontalLayout->addWidget(groupP);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        btnStart = new QPushButton(NpBatchDialog);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        btnStart->setMinimumSize(QSize(100, 30));

        horizontalLayout_2->addWidget(btnStart);

        btnCancel = new QPushButton(NpBatchDialog);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        btnCancel->setMinimumSize(QSize(100, 30));

        horizontalLayout_2->addWidget(btnCancel);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(NpBatchDialog);

        QMetaObject::connectSlotsByName(NpBatchDialog);
    } // setupUi

    void retranslateUi(QDialog *NpBatchDialog)
    {
        NpBatchDialog->setWindowTitle(QCoreApplication::translate("NpBatchDialog", "NP\350\201\224\345\220\210\346\265\213\350\257\225", nullptr));
        groupN->setTitle(QCoreApplication::translate("NpBatchDialog", "N\347\253\257\357\274\210CCD1\357\274\211", nullptr));
        btnLoadN->setText(QCoreApplication::translate("NpBatchDialog", "\351\200\211\346\213\251N\347\253\257\350\236\215\345\220\210\345\233\276", nullptr));
        groupP->setTitle(QCoreApplication::translate("NpBatchDialog", "P\347\253\257\357\274\210CCD2\357\274\211", nullptr));
        btnLoadP->setText(QCoreApplication::translate("NpBatchDialog", "\351\200\211\346\213\251P\347\253\257\350\236\215\345\220\210\345\233\276", nullptr));
        btnStart->setText(QCoreApplication::translate("NpBatchDialog", "\345\274\200\345\247\213\346\265\213\350\257\225", nullptr));
        btnCancel->setText(QCoreApplication::translate("NpBatchDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class NpBatchDialog: public Ui_NpBatchDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NPBATCHDIALOG_H
