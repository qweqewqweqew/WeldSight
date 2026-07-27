/********************************************************************************
** Form generated from reading UI file 'algconfigform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALGCONFIGFORM_H
#define UI_ALGCONFIGFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AlgConfigForm
{
public:
    QGridLayout *gridLayout_7;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label;
    QDoubleSpinBox *minWSpinBox;
    QLabel *label_2;
    QDoubleSpinBox *minHSpinBox;
    QLabel *label_3;
    QDoubleSpinBox *maxWSpinBox;
    QLabel *label_4;
    QDoubleSpinBox *maxHSpinBox;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_5;
    QDoubleSpinBox *minAreaSpinBox;
    QLabel *label_6;
    QDoubleSpinBox *maxAreaSpinBox;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_3;
    QLabel *label_7;
    QDoubleSpinBox *maxTBDisSpinBox;
    QLabel *label_8;
    QDoubleSpinBox *minTBDisSpinBox;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QLabel *label_9;
    QDoubleSpinBox *maxPYSpinBox;
    QLabel *label_10;
    QDoubleSpinBox *minPYSpinBox;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_8;
    QLabel *label_15;
    QDoubleSpinBox *ltAllAreaSpinBox;
    QLabel *label_16;
    QDoubleSpinBox *ltMaxAreaSpinBox;
    QLabel *label_17;
    QDoubleSpinBox *hdAllAreaSpinBox;
    QLabel *label_18;
    QDoubleSpinBox *hdMaxAreaSpinBox;
    QLabel *label_19;
    QDoubleSpinBox *moonRadiusSpinBox;
    QLabel *label_20;
    QDoubleSpinBox *moonDiffAreaSpinBox;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_6;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_11;
    QDoubleSpinBox *pix2mmScale_F_x;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_13;
    QDoubleSpinBox *pix2mmScale_F_y;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout_5;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_12;
    QDoubleSpinBox *pix2mmScale_Z_x;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_14;
    QDoubleSpinBox *pix2mmScale_Z_y;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QPushButton *applyButton;
    QPushButton *saveButton;
    QPushButton *resetButton;

    void setupUi(QWidget *AlgConfigForm)
    {
        if (AlgConfigForm->objectName().isEmpty())
            AlgConfigForm->setObjectName(QString::fromUtf8("AlgConfigForm"));
        AlgConfigForm->resize(444, 594);
        gridLayout_7 = new QGridLayout(AlgConfigForm);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        groupBox = new QGroupBox(AlgConfigForm);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        minWSpinBox = new QDoubleSpinBox(groupBox);
        minWSpinBox->setObjectName(QString::fromUtf8("minWSpinBox"));

        gridLayout->addWidget(minWSpinBox, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 2, 1, 1);

        minHSpinBox = new QDoubleSpinBox(groupBox);
        minHSpinBox->setObjectName(QString::fromUtf8("minHSpinBox"));

        gridLayout->addWidget(minHSpinBox, 0, 3, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        maxWSpinBox = new QDoubleSpinBox(groupBox);
        maxWSpinBox->setObjectName(QString::fromUtf8("maxWSpinBox"));

        gridLayout->addWidget(maxWSpinBox, 1, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 1, 2, 1, 1);

        maxHSpinBox = new QDoubleSpinBox(groupBox);
        maxHSpinBox->setObjectName(QString::fromUtf8("maxHSpinBox"));

        gridLayout->addWidget(maxHSpinBox, 1, 3, 1, 1);


        gridLayout_7->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(AlgConfigForm);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 0, 0, 1, 1);

        minAreaSpinBox = new QDoubleSpinBox(groupBox_2);
        minAreaSpinBox->setObjectName(QString::fromUtf8("minAreaSpinBox"));

        gridLayout_2->addWidget(minAreaSpinBox, 0, 1, 1, 1);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_2->addWidget(label_6, 0, 2, 1, 1);

        maxAreaSpinBox = new QDoubleSpinBox(groupBox_2);
        maxAreaSpinBox->setObjectName(QString::fromUtf8("maxAreaSpinBox"));

        gridLayout_2->addWidget(maxAreaSpinBox, 0, 3, 1, 1);


        gridLayout_7->addWidget(groupBox_2, 1, 0, 1, 1);

        groupBox_3 = new QGroupBox(AlgConfigForm);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_3 = new QGridLayout(groupBox_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_3->addWidget(label_7, 0, 0, 1, 1);

        maxTBDisSpinBox = new QDoubleSpinBox(groupBox_3);
        maxTBDisSpinBox->setObjectName(QString::fromUtf8("maxTBDisSpinBox"));

        gridLayout_3->addWidget(maxTBDisSpinBox, 0, 1, 1, 1);

        label_8 = new QLabel(groupBox_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_3->addWidget(label_8, 0, 2, 1, 1);

        minTBDisSpinBox = new QDoubleSpinBox(groupBox_3);
        minTBDisSpinBox->setObjectName(QString::fromUtf8("minTBDisSpinBox"));

        gridLayout_3->addWidget(minTBDisSpinBox, 0, 3, 1, 1);


        gridLayout_7->addWidget(groupBox_3, 2, 0, 1, 1);

        groupBox_4 = new QGroupBox(AlgConfigForm);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label_9 = new QLabel(groupBox_4);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_4->addWidget(label_9, 0, 0, 1, 1);

        maxPYSpinBox = new QDoubleSpinBox(groupBox_4);
        maxPYSpinBox->setObjectName(QString::fromUtf8("maxPYSpinBox"));

        gridLayout_4->addWidget(maxPYSpinBox, 0, 1, 1, 1);

        label_10 = new QLabel(groupBox_4);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_4->addWidget(label_10, 0, 2, 1, 1);

        minPYSpinBox = new QDoubleSpinBox(groupBox_4);
        minPYSpinBox->setObjectName(QString::fromUtf8("minPYSpinBox"));

        gridLayout_4->addWidget(minPYSpinBox, 0, 3, 1, 1);


        gridLayout_7->addWidget(groupBox_4, 3, 0, 1, 1);

        groupBox_7 = new QGroupBox(AlgConfigForm);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        gridLayout_8 = new QGridLayout(groupBox_7);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        label_15 = new QLabel(groupBox_7);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_8->addWidget(label_15, 0, 0, 1, 1);

        ltAllAreaSpinBox = new QDoubleSpinBox(groupBox_7);
        ltAllAreaSpinBox->setObjectName(QString::fromUtf8("ltAllAreaSpinBox"));

        gridLayout_8->addWidget(ltAllAreaSpinBox, 0, 1, 1, 1);

        label_16 = new QLabel(groupBox_7);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout_8->addWidget(label_16, 0, 2, 1, 1);

        ltMaxAreaSpinBox = new QDoubleSpinBox(groupBox_7);
        ltMaxAreaSpinBox->setObjectName(QString::fromUtf8("ltMaxAreaSpinBox"));

        gridLayout_8->addWidget(ltMaxAreaSpinBox, 0, 3, 1, 1);

        label_17 = new QLabel(groupBox_7);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_8->addWidget(label_17, 1, 0, 1, 1);

        hdAllAreaSpinBox = new QDoubleSpinBox(groupBox_7);
        hdAllAreaSpinBox->setObjectName(QString::fromUtf8("hdAllAreaSpinBox"));

        gridLayout_8->addWidget(hdAllAreaSpinBox, 1, 1, 1, 1);

        label_18 = new QLabel(groupBox_7);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_8->addWidget(label_18, 1, 2, 1, 1);

        hdMaxAreaSpinBox = new QDoubleSpinBox(groupBox_7);
        hdMaxAreaSpinBox->setObjectName(QString::fromUtf8("hdMaxAreaSpinBox"));

        gridLayout_8->addWidget(hdMaxAreaSpinBox, 1, 3, 1, 1);

        label_19 = new QLabel(groupBox_7);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout_8->addWidget(label_19, 2, 0, 1, 1);

        moonRadiusSpinBox = new QDoubleSpinBox(groupBox_7);
        moonRadiusSpinBox->setObjectName(QString::fromUtf8("moonRadiusSpinBox"));

        gridLayout_8->addWidget(moonRadiusSpinBox, 2, 1, 1, 1);

        label_20 = new QLabel(groupBox_7);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_8->addWidget(label_20, 2, 2, 1, 1);

        moonDiffAreaSpinBox = new QDoubleSpinBox(groupBox_7);
        moonDiffAreaSpinBox->setObjectName(QString::fromUtf8("moonDiffAreaSpinBox"));

        gridLayout_8->addWidget(moonDiffAreaSpinBox, 2, 3, 1, 1);


        gridLayout_7->addWidget(groupBox_7, 4, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        groupBox_5 = new QGroupBox(AlgConfigForm);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout_6 = new QGridLayout(groupBox_5);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_11 = new QLabel(groupBox_5);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_3->addWidget(label_11);

        pix2mmScale_F_x = new QDoubleSpinBox(groupBox_5);
        pix2mmScale_F_x->setObjectName(QString::fromUtf8("pix2mmScale_F_x"));

        horizontalLayout_3->addWidget(pix2mmScale_F_x);


        gridLayout_6->addLayout(horizontalLayout_3, 0, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_4->addWidget(label_13);

        pix2mmScale_F_y = new QDoubleSpinBox(groupBox_5);
        pix2mmScale_F_y->setObjectName(QString::fromUtf8("pix2mmScale_F_y"));

        horizontalLayout_4->addWidget(pix2mmScale_F_y);


        gridLayout_6->addLayout(horizontalLayout_4, 1, 0, 1, 1);


        horizontalLayout_2->addWidget(groupBox_5);

        groupBox_6 = new QGroupBox(AlgConfigForm);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        gridLayout_5 = new QGridLayout(groupBox_6);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_12 = new QLabel(groupBox_6);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_5->addWidget(label_12);

        pix2mmScale_Z_x = new QDoubleSpinBox(groupBox_6);
        pix2mmScale_Z_x->setObjectName(QString::fromUtf8("pix2mmScale_Z_x"));

        horizontalLayout_5->addWidget(pix2mmScale_Z_x);


        gridLayout_5->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_14 = new QLabel(groupBox_6);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_6->addWidget(label_14);

        pix2mmScale_Z_y = new QDoubleSpinBox(groupBox_6);
        pix2mmScale_Z_y->setObjectName(QString::fromUtf8("pix2mmScale_Z_y"));

        horizontalLayout_6->addWidget(pix2mmScale_Z_y);


        gridLayout_5->addLayout(horizontalLayout_6, 1, 0, 1, 1);


        horizontalLayout_2->addWidget(groupBox_6);


        gridLayout_7->addLayout(horizontalLayout_2, 5, 0, 1, 1);

        widget = new QWidget(AlgConfigForm);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        applyButton = new QPushButton(widget);
        applyButton->setObjectName(QString::fromUtf8("applyButton"));
        applyButton->setMinimumSize(QSize(0, 30));

        horizontalLayout->addWidget(applyButton);

        saveButton = new QPushButton(widget);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        saveButton->setMinimumSize(QSize(0, 30));

        horizontalLayout->addWidget(saveButton);

        resetButton = new QPushButton(widget);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));
        resetButton->setMinimumSize(QSize(0, 30));

        horizontalLayout->addWidget(resetButton);


        gridLayout_7->addWidget(widget, 6, 0, 1, 1);


        retranslateUi(AlgConfigForm);

        QMetaObject::connectSlotsByName(AlgConfigForm);
    } // setupUi

    void retranslateUi(QWidget *AlgConfigForm)
    {
        AlgConfigForm->setWindowTitle(QCoreApplication::translate("AlgConfigForm", "\347\256\227\346\263\225\345\217\202\346\225\260\351\205\215\347\275\256", nullptr));
        groupBox->setTitle(QCoreApplication::translate("AlgConfigForm", "\345\260\272\345\257\270\351\230\210\345\200\274", nullptr));
        label->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\260\217\345\256\275\345\272\246(mm):", nullptr));
        label_2->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\260\217\351\253\230\345\272\246(mm):", nullptr));
        label_3->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\244\247\345\256\275\345\272\246(mm):", nullptr));
        label_4->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\244\247\351\253\230\345\272\246(mm):", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("AlgConfigForm", "\351\235\242\347\247\257\351\230\210\345\200\274", nullptr));
        label_5->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\260\217\351\235\242\347\247\257(mm\302\262):", nullptr));
        label_6->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\244\247\351\235\242\347\247\257(mm\302\262):", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("AlgConfigForm", "\351\223\234\346\235\277\350\267\235\347\246\273\351\230\210\345\200\274", nullptr));
        label_7->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\244\247\350\267\235\347\246\273(mm):", nullptr));
        label_8->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\260\217\350\267\235\347\246\273(mm):", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("AlgConfigForm", "\347\204\212\351\224\241\345\201\217\347\246\273\347\250\213\345\272\246", nullptr));
        label_9->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\244\247\345\201\217\347\247\273:", nullptr));
        label_10->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\200\345\260\217\345\201\217\347\247\273:", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("AlgConfigForm", "\347\221\225\347\226\265\351\235\242\347\247\257\346\216\247\345\210\266", nullptr));
        label_15->setText(QCoreApplication::translate("AlgConfigForm", "\351\234\262\351\223\234\346\200\273\351\235\242\347\247\257\351\230\210\345\200\274(mm\302\262):", nullptr));
        label_16->setText(QCoreApplication::translate("AlgConfigForm", "\351\234\262\351\223\234\346\234\200\345\244\247\351\235\242\347\247\257\351\230\210\345\200\274(mm\302\262):", nullptr));
        label_17->setText(QCoreApplication::translate("AlgConfigForm", "\351\273\221\347\202\271\346\200\273\351\235\242\347\247\257\351\230\210\345\200\274(mm\302\262):", nullptr));
        label_18->setText(QCoreApplication::translate("AlgConfigForm", "\351\273\221\347\202\271\346\234\200\345\244\247\351\235\242\347\247\257\351\230\210\345\200\274(mm\302\262):", nullptr));
        label_19->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\210\347\211\231\345\215\212\345\276\204(px):", nullptr));
        label_20->setText(QCoreApplication::translate("AlgConfigForm", "\346\234\210\347\211\231NG\351\235\242\347\247\257(mm\302\262):", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("AlgConfigForm", "\347\233\270\346\234\2721\346\240\207\345\256\232", nullptr));
        label_11->setText(QCoreApplication::translate("AlgConfigForm", "\345\203\217\347\264\240\345\210\260\346\257\253\347\261\263\346\257\224\344\276\213X:", nullptr));
        label_13->setText(QCoreApplication::translate("AlgConfigForm", "\345\203\217\347\264\240\345\210\260\346\257\253\347\261\263\346\257\224\344\276\213Y:", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("AlgConfigForm", "\347\233\270\346\234\2722\346\240\207\345\256\232", nullptr));
        label_12->setText(QCoreApplication::translate("AlgConfigForm", "\345\203\217\347\264\240\345\210\260\346\257\253\347\261\263\346\257\224\344\276\213X:", nullptr));
        label_14->setText(QCoreApplication::translate("AlgConfigForm", "\345\203\217\347\264\240\345\210\260\346\257\253\347\261\263\346\257\224\344\276\213Y:", nullptr));
        applyButton->setText(QCoreApplication::translate("AlgConfigForm", "\345\272\224\347\224\250", nullptr));
        saveButton->setText(QCoreApplication::translate("AlgConfigForm", "\344\277\235\345\255\230", nullptr));
        resetButton->setText(QCoreApplication::translate("AlgConfigForm", "\351\207\215\347\275\256", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AlgConfigForm: public Ui_AlgConfigForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALGCONFIGFORM_H
