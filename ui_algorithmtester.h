/********************************************************************************
** Form generated from reading UI file 'algorithmtester.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALGORITHMTESTER_H
#define UI_ALGORITHMTESTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>
#include "imagezoomview.h"

QT_BEGIN_NAMESPACE

class Ui_AlgorithmTester
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_Buttons;
    QPushButton *btnLoadFour;
    QPushButton *btnLoadOne;
    QPushButton *btnNpBatch;
    QPushButton *btnLoopTest;
    QComboBox *cBox_1;
    QCheckBox *checkBox;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_Input;
    ImageZoomView *labelInput0;
    ImageZoomView *labelInput1;
    ImageZoomView *labelInput2;
    ImageZoomView *labelInput3;
    QHBoxLayout *horizontalLayout;
    ImageZoomView *labelFusion;
    ImageZoomView *labelResult;
    QTableWidget *tableWidget;

    void setupUi(QWidget *AlgorithmTester)
    {
        if (AlgorithmTester->objectName().isEmpty())
            AlgorithmTester->setObjectName(QString::fromUtf8("AlgorithmTester"));
        AlgorithmTester->resize(1000, 896);
        gridLayout = new QGridLayout(AlgorithmTester);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout_Buttons = new QHBoxLayout();
        horizontalLayout_Buttons->setObjectName(QString::fromUtf8("horizontalLayout_Buttons"));
        btnLoadFour = new QPushButton(AlgorithmTester);
        btnLoadFour->setObjectName(QString::fromUtf8("btnLoadFour"));
        btnLoadFour->setMinimumSize(QSize(0, 30));

        horizontalLayout_Buttons->addWidget(btnLoadFour);

        btnLoadOne = new QPushButton(AlgorithmTester);
        btnLoadOne->setObjectName(QString::fromUtf8("btnLoadOne"));
        btnLoadOne->setMinimumSize(QSize(0, 30));

        horizontalLayout_Buttons->addWidget(btnLoadOne);

        btnNpBatch = new QPushButton(AlgorithmTester);
        btnNpBatch->setObjectName(QString::fromUtf8("btnNpBatch"));
        btnNpBatch->setMinimumSize(QSize(0, 30));

        horizontalLayout_Buttons->addWidget(btnNpBatch);

        btnLoopTest = new QPushButton(AlgorithmTester);
        btnLoopTest->setObjectName(QString::fromUtf8("btnLoopTest"));
        btnLoopTest->setMinimumSize(QSize(0, 30));

        horizontalLayout_Buttons->addWidget(btnLoopTest);

        cBox_1 = new QComboBox(AlgorithmTester);
        cBox_1->setObjectName(QString::fromUtf8("cBox_1"));
        cBox_1->setMinimumSize(QSize(150, 30));

        horizontalLayout_Buttons->addWidget(cBox_1);

        checkBox = new QCheckBox(AlgorithmTester);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        horizontalLayout_Buttons->addWidget(checkBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Buttons->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout_Buttons, 0, 0, 1, 1);

        horizontalLayout_Input = new QHBoxLayout();
        horizontalLayout_Input->setObjectName(QString::fromUtf8("horizontalLayout_Input"));
        labelInput0 = new ImageZoomView(AlgorithmTester);
        labelInput0->setObjectName(QString::fromUtf8("labelInput0"));
        labelInput0->setMinimumSize(QSize(200, 150));
        labelInput0->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelInput0->setFrameShape(QFrame::Box);
        labelInput0->setAlignment(Qt::AlignCenter);

        horizontalLayout_Input->addWidget(labelInput0);

        labelInput1 = new ImageZoomView(AlgorithmTester);
        labelInput1->setObjectName(QString::fromUtf8("labelInput1"));
        labelInput1->setMinimumSize(QSize(200, 150));
        labelInput1->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelInput1->setFrameShape(QFrame::Box);
        labelInput1->setAlignment(Qt::AlignCenter);

        horizontalLayout_Input->addWidget(labelInput1);

        labelInput2 = new ImageZoomView(AlgorithmTester);
        labelInput2->setObjectName(QString::fromUtf8("labelInput2"));
        labelInput2->setMinimumSize(QSize(200, 150));
        labelInput2->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelInput2->setFrameShape(QFrame::Box);
        labelInput2->setAlignment(Qt::AlignCenter);

        horizontalLayout_Input->addWidget(labelInput2);

        labelInput3 = new ImageZoomView(AlgorithmTester);
        labelInput3->setObjectName(QString::fromUtf8("labelInput3"));
        labelInput3->setMinimumSize(QSize(200, 150));
        labelInput3->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelInput3->setFrameShape(QFrame::Box);
        labelInput3->setAlignment(Qt::AlignCenter);

        horizontalLayout_Input->addWidget(labelInput3);


        gridLayout->addLayout(horizontalLayout_Input, 1, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        labelFusion = new ImageZoomView(AlgorithmTester);
        labelFusion->setObjectName(QString::fromUtf8("labelFusion"));
        labelFusion->setMinimumSize(QSize(400, 350));
        labelFusion->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelFusion->setFrameShape(QFrame::Box);
        labelFusion->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(labelFusion);

        labelResult = new ImageZoomView(AlgorithmTester);
        labelResult->setObjectName(QString::fromUtf8("labelResult"));
        labelResult->setMinimumSize(QSize(400, 350));
        labelResult->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);\n"
"border: 1px solid rgb(255, 255, 255);\n"
"color: rgb(255, 255, 255);"));
        labelResult->setFrameShape(QFrame::Box);
        labelResult->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(labelResult);


        gridLayout->addLayout(horizontalLayout, 2, 0, 1, 1);

        tableWidget = new QTableWidget(AlgorithmTester);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setColumnCount(0);
        tableWidget->horizontalHeader()->setStretchLastSection(true);

        gridLayout->addWidget(tableWidget, 4, 0, 1, 1);


        retranslateUi(AlgorithmTester);

        QMetaObject::connectSlotsByName(AlgorithmTester);
    } // setupUi

    void retranslateUi(QWidget *AlgorithmTester)
    {
        AlgorithmTester->setWindowTitle(QCoreApplication::translate("AlgorithmTester", "\347\204\212\347\202\271\346\243\200\346\265\213\347\256\227\346\263\225\346\265\213\350\257\225", nullptr));
        btnLoadFour->setText(QCoreApplication::translate("AlgorithmTester", "\345\212\240\350\275\2754\345\274\240\345\233\276", nullptr));
        btnLoadOne->setText(QCoreApplication::translate("AlgorithmTester", "\345\212\240\350\275\275\350\236\215\345\220\210\345\233\276", nullptr));
        btnNpBatch->setText(QCoreApplication::translate("AlgorithmTester", "NP\350\201\224\345\220\210\346\265\213\350\257\225", nullptr));
        btnLoopTest->setText(QCoreApplication::translate("AlgorithmTester", "\345\276\252\347\216\257\346\265\213\350\257\225", nullptr));
        checkBox->setText(QCoreApplication::translate("AlgorithmTester", "\347\233\270\346\234\272\346\240\207\345\256\232", nullptr));
        labelInput0->setText(QCoreApplication::translate("AlgorithmTester", "\345\233\2761", nullptr));
        labelInput1->setText(QCoreApplication::translate("AlgorithmTester", "\345\233\2762", nullptr));
        labelInput2->setText(QCoreApplication::translate("AlgorithmTester", "\345\233\2763", nullptr));
        labelInput3->setText(QCoreApplication::translate("AlgorithmTester", "\345\233\2764", nullptr));
        labelFusion->setText(QCoreApplication::translate("AlgorithmTester", "\350\236\215\345\220\210\345\233\276", nullptr));
        labelResult->setText(QCoreApplication::translate("AlgorithmTester", "\347\273\223\346\236\234\345\233\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AlgorithmTester: public Ui_AlgorithmTester {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALGORITHMTESTER_H
