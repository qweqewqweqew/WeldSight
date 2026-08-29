/********************************************************************************
** Form generated from reading UI file 'saveform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAVEFORM_H
#define UI_SAVEFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SaveForm
{
public:
    QGridLayout *gridLayout_7;
    QGroupBox *gBox_1;
    QGridLayout *gridLayout;
    QCheckBox *cBox_1_1;
    QCheckBox *cBox_1_2;
    QCheckBox *cBox_1_3;
    QCheckBox *cBox_1_4;
    QGroupBox *gBox_2;
    QGridLayout *gridLayout_2;
    QCheckBox *cBox_2_1;
    QCheckBox *cBox_2_2;
    QCheckBox *cBox_2_3;
    QCheckBox *cBox_2_4;
    QGroupBox *gBox_path;
    QVBoxLayout *verticalLayout;
    QLabel *label_pathHint;
    QLabel *label_currentPath;
    QHBoxLayout *horizontalLayout;
    QPushButton *Bt_changePath;
    QPushButton *Bt_look;
    QPushButton *Bt_confirm;

    void setupUi(QWidget *SaveForm)
    {
        if (SaveForm->objectName().isEmpty())
            SaveForm->setObjectName(QString::fromUtf8("SaveForm"));
        SaveForm->resize(314, 520);
        SaveForm->setStyleSheet(QString::fromUtf8("\n"
"				QGroupBox{\n"
"				border: 1px solid black;\n"
"				}\n"
"			"));
        gridLayout_7 = new QGridLayout(SaveForm);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        gBox_1 = new QGroupBox(SaveForm);
        gBox_1->setObjectName(QString::fromUtf8("gBox_1"));
        gBox_1->setStyleSheet(QString::fromUtf8(""));
        gridLayout = new QGridLayout(gBox_1);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        cBox_1_1 = new QCheckBox(gBox_1);
        cBox_1_1->setObjectName(QString::fromUtf8("cBox_1_1"));

        gridLayout->addWidget(cBox_1_1, 0, 0, 1, 1);

        cBox_1_2 = new QCheckBox(gBox_1);
        cBox_1_2->setObjectName(QString::fromUtf8("cBox_1_2"));

        gridLayout->addWidget(cBox_1_2, 1, 0, 1, 1);

        cBox_1_3 = new QCheckBox(gBox_1);
        cBox_1_3->setObjectName(QString::fromUtf8("cBox_1_3"));

        gridLayout->addWidget(cBox_1_3, 2, 0, 1, 1);

        cBox_1_4 = new QCheckBox(gBox_1);
        cBox_1_4->setObjectName(QString::fromUtf8("cBox_1_4"));

        gridLayout->addWidget(cBox_1_4, 3, 0, 1, 1);


        gridLayout_7->addWidget(gBox_1, 0, 0, 1, 1);

        gBox_2 = new QGroupBox(SaveForm);
        gBox_2->setObjectName(QString::fromUtf8("gBox_2"));
        gBox_2->setStyleSheet(QString::fromUtf8(""));
        gridLayout_2 = new QGridLayout(gBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        cBox_2_1 = new QCheckBox(gBox_2);
        cBox_2_1->setObjectName(QString::fromUtf8("cBox_2_1"));

        gridLayout_2->addWidget(cBox_2_1, 0, 0, 1, 1);

        cBox_2_2 = new QCheckBox(gBox_2);
        cBox_2_2->setObjectName(QString::fromUtf8("cBox_2_2"));

        gridLayout_2->addWidget(cBox_2_2, 1, 0, 1, 1);

        cBox_2_3 = new QCheckBox(gBox_2);
        cBox_2_3->setObjectName(QString::fromUtf8("cBox_2_3"));

        gridLayout_2->addWidget(cBox_2_3, 2, 0, 1, 1);

        cBox_2_4 = new QCheckBox(gBox_2);
        cBox_2_4->setObjectName(QString::fromUtf8("cBox_2_4"));

        gridLayout_2->addWidget(cBox_2_4, 3, 0, 1, 1);


        gridLayout_7->addWidget(gBox_2, 0, 1, 1, 1);

        gBox_path = new QGroupBox(SaveForm);
        gBox_path->setObjectName(QString::fromUtf8("gBox_path"));
        verticalLayout = new QVBoxLayout(gBox_path);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_pathHint = new QLabel(gBox_path);
        label_pathHint->setObjectName(QString::fromUtf8("label_pathHint"));

        verticalLayout->addWidget(label_pathHint);

        label_currentPath = new QLabel(gBox_path);
        label_currentPath->setObjectName(QString::fromUtf8("label_currentPath"));
        label_currentPath->setStyleSheet(QString::fromUtf8("\n"
"										QLabel {\n"
"										background-color: #f0f0f0;\n"
"										border: 1px solid #ccc;\n"
"										padding: 4px;\n"
"										border-radius: 3px;\n"
"										}\n"
"									"));
        label_currentPath->setWordWrap(true);

        verticalLayout->addWidget(label_currentPath);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        Bt_changePath = new QPushButton(gBox_path);
        Bt_changePath->setObjectName(QString::fromUtf8("Bt_changePath"));

        horizontalLayout->addWidget(Bt_changePath);

        Bt_look = new QPushButton(gBox_path);
        Bt_look->setObjectName(QString::fromUtf8("Bt_look"));

        horizontalLayout->addWidget(Bt_look);

        Bt_confirm = new QPushButton(gBox_path);
        Bt_confirm->setObjectName(QString::fromUtf8("Bt_confirm"));

        horizontalLayout->addWidget(Bt_confirm);


        verticalLayout->addLayout(horizontalLayout);


        gridLayout_7->addWidget(gBox_path, 2, 0, 1, 2);

        gridLayout_7->setRowStretch(0, 3);
        gridLayout_7->setRowStretch(1, 1);
        gridLayout_7->setRowStretch(2, 2);

        retranslateUi(SaveForm);

        QMetaObject::connectSlotsByName(SaveForm);
    } // setupUi

    void retranslateUi(QWidget *SaveForm)
    {
        SaveForm->setWindowTitle(QCoreApplication::translate("SaveForm", "Form", nullptr));
        gBox_1->setTitle(QCoreApplication::translate("SaveForm", "ccd1", nullptr));
        cBox_1_1->setText(QCoreApplication::translate("SaveForm", "\345\216\237\345\233\276", nullptr));
        cBox_1_2->setText(QCoreApplication::translate("SaveForm", "\345\220\210\346\210\220\345\233\276", nullptr));
        cBox_1_3->setText(QCoreApplication::translate("SaveForm", "OK", nullptr));
        cBox_1_4->setText(QCoreApplication::translate("SaveForm", "NG", nullptr));
        gBox_2->setTitle(QCoreApplication::translate("SaveForm", "ccd2", nullptr));
        cBox_2_1->setText(QCoreApplication::translate("SaveForm", "\345\216\237\345\233\276", nullptr));
        cBox_2_2->setText(QCoreApplication::translate("SaveForm", "\345\220\210\346\210\220\345\233\276", nullptr));
        cBox_2_3->setText(QCoreApplication::translate("SaveForm", "OK", nullptr));
        cBox_2_4->setText(QCoreApplication::translate("SaveForm", "NG", nullptr));
        gBox_path->setTitle(QCoreApplication::translate("SaveForm", "\345\233\276\345\203\217\344\277\235\345\255\230\350\267\257\345\276\204", nullptr));
        label_pathHint->setText(QCoreApplication::translate("SaveForm", "\345\275\223\345\211\215\344\277\235\345\255\230\350\267\257\345\276\204:", nullptr));
        label_currentPath->setText(QCoreApplication::translate("SaveForm", "\351\273\230\350\256\244\350\267\257\345\276\204", nullptr));
        Bt_changePath->setText(QCoreApplication::translate("SaveForm", "\346\233\264\346\224\271\350\267\257\345\276\204", nullptr));
        Bt_look->setText(QCoreApplication::translate("SaveForm", "\346\237\245\347\234\213\346\226\207\344\273\266\345\244\271", nullptr));
        Bt_confirm->setText(QCoreApplication::translate("SaveForm", "\347\241\256\350\256\244", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SaveForm: public Ui_SaveForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAVEFORM_H
