/********************************************************************************
** Form generated from reading UI file 'dataform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DATAFORM_H
#define UI_DATAFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DataForm
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *Data_gBox_1;
    QGridLayout *gridLayout_4;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *horizontalLayout;
    QLabel *label_start;
    QDateEdit *dateStart;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_end;
    QDateEdit *dateEnd;
    QPushButton *Bt_Query;
    QSpacerItem *horizontalSpacer_5;
    QGroupBox *Data_gBox_2;
    QGridLayout *gridLayout;
    QTableWidget *tableWidget;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_5;
    QLabel *data_1;
    QLabel *data_2;
    QLabel *data_3;
    QLabel *data_4;
    QLabel *data_5;
    QLabel *data_number;
    QLabel *data_ok_count;
    QLabel *data_weight;
    QLabel *data_ng_count;
    QLabel *data_ng_value;
    QGroupBox *Data_gBox_3;
    QGridLayout *gridLayout_3;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *Bt_NextPage;
    QPushButton *Bt_Delete;
    QCheckBox *cb_SelectAllResults;
    QSpacerItem *horizontalSpacer;
    QPushButton *Bt_Search;
    QPushButton *Bt_Export;
    QLabel *labPage;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *Bt_PrePage;

    void setupUi(QWidget *DataForm)
    {
        if (DataForm->objectName().isEmpty())
            DataForm->setObjectName(QString::fromUtf8("DataForm"));
        DataForm->resize(998, 690);
        gridLayout_2 = new QGridLayout(DataForm);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(9, -1, 9, -1);
        Data_gBox_1 = new QGroupBox(DataForm);
        Data_gBox_1->setObjectName(QString::fromUtf8("Data_gBox_1"));
        Data_gBox_1->setMinimumSize(QSize(0, 80));
        Data_gBox_1->setMaximumSize(QSize(16777215, 80));
        gridLayout_4 = new QGridLayout(Data_gBox_1);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(-1, 20, -1, -1);
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_4, 0, 4, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_start = new QLabel(Data_gBox_1);
        label_start->setObjectName(QString::fromUtf8("label_start"));
        label_start->setMinimumSize(QSize(60, 30));
        label_start->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_start);

        dateStart = new QDateEdit(Data_gBox_1);
        dateStart->setObjectName(QString::fromUtf8("dateStart"));
        dateStart->setMinimumSize(QSize(150, 30));
        dateStart->setCalendarPopup(true);

        horizontalLayout->addWidget(dateStart);


        gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_end = new QLabel(Data_gBox_1);
        label_end->setObjectName(QString::fromUtf8("label_end"));
        label_end->setMinimumSize(QSize(60, 30));
        label_end->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_end);

        dateEnd = new QDateEdit(Data_gBox_1);
        dateEnd->setObjectName(QString::fromUtf8("dateEnd"));
        dateEnd->setMinimumSize(QSize(150, 30));
        dateEnd->setCalendarPopup(true);

        horizontalLayout_2->addWidget(dateEnd);


        gridLayout_4->addLayout(horizontalLayout_2, 0, 1, 1, 1);

        Bt_Query = new QPushButton(Data_gBox_1);
        Bt_Query->setObjectName(QString::fromUtf8("Bt_Query"));
        Bt_Query->setMinimumSize(QSize(80, 30));

        gridLayout_4->addWidget(Bt_Query, 0, 3, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_5, 0, 2, 1, 1);


        gridLayout_2->addWidget(Data_gBox_1, 0, 0, 1, 1);

        Data_gBox_2 = new QGroupBox(DataForm);
        Data_gBox_2->setObjectName(QString::fromUtf8("Data_gBox_2"));
        Data_gBox_2->setMinimumSize(QSize(980, 450));
        gridLayout = new QGridLayout(Data_gBox_2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(-1, 30, -1, -1);
        tableWidget = new QTableWidget(Data_gBox_2);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setStyleSheet(QString::fromUtf8(""));

        gridLayout->addWidget(tableWidget, 0, 0, 1, 1);

        groupBox = new QGroupBox(Data_gBox_2);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setMinimumSize(QSize(0, 50));
        groupBox->setStyleSheet(QString::fromUtf8("\n"
"										#groupBox{\n"
"										border: 1px solid #CCCCCC;\n"
"										}\n"
"									"));
        gridLayout_5 = new QGridLayout(groupBox);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        data_1 = new QLabel(groupBox);
        data_1->setObjectName(QString::fromUtf8("data_1"));
        data_1->setMinimumSize(QSize(0, 0));
        data_1->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_1->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_1, 0, 0, 1, 1);

        data_2 = new QLabel(groupBox);
        data_2->setObjectName(QString::fromUtf8("data_2"));
        data_2->setMinimumSize(QSize(0, 0));

        gridLayout_5->addWidget(data_2, 0, 1, 1, 1);

        data_3 = new QLabel(groupBox);
        data_3->setObjectName(QString::fromUtf8("data_3"));
        data_3->setMinimumSize(QSize(0, 0));

        gridLayout_5->addWidget(data_3, 0, 2, 1, 1);

        data_4 = new QLabel(groupBox);
        data_4->setObjectName(QString::fromUtf8("data_4"));
        data_4->setMinimumSize(QSize(0, 0));

        gridLayout_5->addWidget(data_4, 0, 3, 1, 1);

        data_5 = new QLabel(groupBox);
        data_5->setObjectName(QString::fromUtf8("data_5"));
        data_5->setMinimumSize(QSize(0, 0));

        gridLayout_5->addWidget(data_5, 0, 4, 1, 1);

        data_number = new QLabel(groupBox);
        data_number->setObjectName(QString::fromUtf8("data_number"));
        data_number->setMinimumSize(QSize(0, 0));
        data_number->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_number->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_number, 0, 5, 1, 1);

        data_ok_count = new QLabel(groupBox);
        data_ok_count->setObjectName(QString::fromUtf8("data_ok_count"));
        data_ok_count->setMinimumSize(QSize(0, 0));
        data_ok_count->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_ok_count->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_ok_count, 0, 6, 1, 1);

        data_weight = new QLabel(groupBox);
        data_weight->setObjectName(QString::fromUtf8("data_weight"));
        data_weight->setMinimumSize(QSize(0, 0));
        data_weight->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_weight->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_weight, 0, 7, 1, 1);

        data_ng_count = new QLabel(groupBox);
        data_ng_count->setObjectName(QString::fromUtf8("data_ng_count"));
        data_ng_count->setMinimumSize(QSize(0, 0));
        data_ng_count->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_ng_count->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_ng_count, 0, 8, 1, 1);

        data_ng_value = new QLabel(groupBox);
        data_ng_value->setObjectName(QString::fromUtf8("data_ng_value"));
        data_ng_value->setMinimumSize(QSize(0, 0));
        data_ng_value->setStyleSheet(QString::fromUtf8("\n"
"													font: 9pt \"Consolas\";\n"
"													border: 0px;\n"
"													color: rgb(0, 0, 0);\n"
"													background-color: rgb(209, 209, 209);\n"
"													border-radius:5px;\n"
"												"));
        data_ng_value->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(data_ng_value, 0, 9, 1, 1);


        gridLayout->addWidget(groupBox, 1, 0, 1, 1);


        gridLayout_2->addWidget(Data_gBox_2, 1, 0, 1, 1);

        Data_gBox_3 = new QGroupBox(DataForm);
        Data_gBox_3->setObjectName(QString::fromUtf8("Data_gBox_3"));
        Data_gBox_3->setMinimumSize(QSize(980, 80));
        Data_gBox_3->setMaximumSize(QSize(16777215, 80));
        gridLayout_3 = new QGridLayout(Data_gBox_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_3, 0, 6, 1, 1);

        Bt_NextPage = new QPushButton(Data_gBox_3);
        Bt_NextPage->setObjectName(QString::fromUtf8("Bt_NextPage"));
        Bt_NextPage->setMinimumSize(QSize(80, 30));

        gridLayout_3->addWidget(Bt_NextPage, 0, 5, 1, 1);

        Bt_Delete = new QPushButton(Data_gBox_3);
        Bt_Delete->setObjectName(QString::fromUtf8("Bt_Delete"));
        Bt_Delete->setMinimumSize(QSize(80, 30));

        gridLayout_3->addWidget(Bt_Delete, 0, 0, 1, 1);

        cb_SelectAllResults = new QCheckBox(Data_gBox_3);
        cb_SelectAllResults->setObjectName(QString::fromUtf8("cb_SelectAllResults"));
        cb_SelectAllResults->setMinimumSize(QSize(120, 30));

        gridLayout_3->addWidget(cb_SelectAllResults, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer, 0, 3, 1, 1);

        Bt_Search = new QPushButton(Data_gBox_3);
        Bt_Search->setObjectName(QString::fromUtf8("Bt_Search"));
        Bt_Search->setMinimumSize(QSize(80, 30));

        gridLayout_3->addWidget(Bt_Search, 0, 2, 1, 1);

        Bt_Export = new QPushButton(Data_gBox_3);
        Bt_Export->setObjectName(QString::fromUtf8("Bt_Export"));
        Bt_Export->setMinimumSize(QSize(80, 30));

        gridLayout_3->addWidget(Bt_Export, 0, 9, 1, 1);

        labPage = new QLabel(Data_gBox_3);
        labPage->setObjectName(QString::fromUtf8("labPage"));

        gridLayout_3->addWidget(labPage, 0, 7, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 8, 1, 1);

        Bt_PrePage = new QPushButton(Data_gBox_3);
        Bt_PrePage->setObjectName(QString::fromUtf8("Bt_PrePage"));
        Bt_PrePage->setMinimumSize(QSize(80, 30));

        gridLayout_3->addWidget(Bt_PrePage, 0, 4, 1, 1);


        gridLayout_2->addWidget(Data_gBox_3, 2, 0, 1, 1);


        retranslateUi(DataForm);

        QMetaObject::connectSlotsByName(DataForm);
    } // setupUi

    void retranslateUi(QWidget *DataForm)
    {
        DataForm->setWindowTitle(QCoreApplication::translate("DataForm", "\347\204\212\351\224\241\346\243\200\346\265\213\346\225\260\346\215\256", nullptr));
        Data_gBox_1->setTitle(QString());
        label_start->setText(QCoreApplication::translate("DataForm", "\345\274\200\345\247\213\346\227\266\351\227\264", nullptr));
        label_end->setText(QCoreApplication::translate("DataForm", "\347\273\223\346\235\237\346\227\266\351\227\264", nullptr));
        Bt_Query->setText(QCoreApplication::translate("DataForm", "\346\237\245\350\257\242", nullptr));
        Data_gBox_2->setTitle(QString());
        groupBox->setTitle(QString());
        data_1->setText(QCoreApplication::translate("DataForm", "\346\200\273\350\256\260\345\275\225\346\225\260", nullptr));
        data_2->setText(QString());
        data_3->setText(QString());
        data_4->setText(QString());
        data_5->setText(QString());
        data_number->setText(QCoreApplication::translate("DataForm", "0", nullptr));
        data_ok_count->setText(QCoreApplication::translate("DataForm", "\345\220\210\346\240\274\346\225\260", nullptr));
        data_weight->setText(QCoreApplication::translate("DataForm", "0", nullptr));
        data_ng_count->setText(QCoreApplication::translate("DataForm", "\344\270\215\345\220\210\346\240\274\346\225\260", nullptr));
        data_ng_value->setText(QCoreApplication::translate("DataForm", "0", nullptr));
        Data_gBox_3->setTitle(QString());
        Bt_NextPage->setText(QCoreApplication::translate("DataForm", "\344\270\213\344\270\200\351\241\265", nullptr));
        Bt_Delete->setText(QCoreApplication::translate("DataForm", "\345\210\240\351\231\244", nullptr));
#if QT_CONFIG(tooltip)
        cb_SelectAllResults->setToolTip(QCoreApplication::translate("DataForm", "\345\213\276\351\200\211\345\275\223\345\211\215\346\237\245\350\257\242\346\235\241\344\273\266\344\270\213\347\232\204\345\205\250\351\203\250\350\256\260\345\275\225\357\274\214\344\273\205\347\224\250\344\272\216\345\210\240\351\231\244", nullptr));
#endif // QT_CONFIG(tooltip)
        cb_SelectAllResults->setText(QCoreApplication::translate("DataForm", "\345\205\250\351\200\211\346\211\200\346\234\211\350\256\260\345\275\225", nullptr));
        Bt_Search->setText(QCoreApplication::translate("DataForm", "\350\257\246\346\203\205", nullptr));
        Bt_Export->setText(QCoreApplication::translate("DataForm", "\345\257\274\345\207\272", nullptr));
        labPage->setText(QCoreApplication::translate("DataForm", "1/1", nullptr));
        Bt_PrePage->setText(QCoreApplication::translate("DataForm", "\344\270\212\344\270\200\351\241\265", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DataForm: public Ui_DataForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DATAFORM_H
