/********************************************************************************
** Form generated from reading UI file 'ioactionform.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IOACTIONFORM_H
#define UI_IOACTIONFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ioActionForm
{
public:
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_4;
    QHBoxLayout *horizontalLayout_26;
    QLabel *label_28;
    QComboBox *cBox_1;
    QPushButton *Bt_refresh;
    QHBoxLayout *horizontalLayout_27;
    QLabel *label_29;
    QComboBox *cBox_2;
    QPushButton *Bt_open;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_5;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_30;
    QLabel *label_camera1_status;
    QFrame *frame_camera1;
    QHBoxLayout *horizontalLayout_31;
    QLabel *label_camera2_status;
    QFrame *frame_camera2;
    QHBoxLayout *horizontalLayout_32;
    QLabel *label_sync_status;
    QFrame *frame_sync;
    QTextEdit *textEdit;
    QHBoxLayout *horizontalLayout_33;
    QPushButton *Bt_clear;
    QPushButton *Bt_reset_sync;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_9;
    QVBoxLayout *verticalLayout;
    QLabel *label_44;
    QLabel *label_45;
    QLabel *label_46;
    QLabel *label_47;
    QLabel *label_48;
    QLabel *label_49;
    QLabel *label_50;
    QLabel *label_51;
    QHBoxLayout *horizontalLayout_29;
    QPushButton *Bt_get;
    QCheckBox *checkBox_auto_poll;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *checkBox_9;
    QCheckBox *checkBox_10;
    QCheckBox *checkBox_11;
    QCheckBox *checkBox_12;
    QCheckBox *checkBox_13;
    QCheckBox *checkBox_14;
    QCheckBox *checkBox_15;
    QCheckBox *checkBox_16;
    QPushButton *Bt_start;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *cBox_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QPushButton *Bt_save;

    void setupUi(QWidget *ioActionForm)
    {
        if (ioActionForm->objectName().isEmpty())
            ioActionForm->setObjectName(QString::fromUtf8("ioActionForm"));
        ioActionForm->resize(850, 700);
        gridLayout_3 = new QGridLayout(ioActionForm);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        groupBox = new QGroupBox(ioActionForm);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_4 = new QGridLayout(groupBox);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        horizontalLayout_26 = new QHBoxLayout();
        horizontalLayout_26->setObjectName(QString::fromUtf8("horizontalLayout_26"));
        label_28 = new QLabel(groupBox);
        label_28->setObjectName(QString::fromUtf8("label_28"));

        horizontalLayout_26->addWidget(label_28);

        cBox_1 = new QComboBox(groupBox);
        cBox_1->setObjectName(QString::fromUtf8("cBox_1"));
        cBox_1->setMinimumSize(QSize(120, 30));
        cBox_1->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_26->addWidget(cBox_1);

        Bt_refresh = new QPushButton(groupBox);
        Bt_refresh->setObjectName(QString::fromUtf8("Bt_refresh"));
        Bt_refresh->setMinimumSize(QSize(60, 30));

        horizontalLayout_26->addWidget(Bt_refresh);


        gridLayout_4->addLayout(horizontalLayout_26, 0, 0, 1, 1);

        horizontalLayout_27 = new QHBoxLayout();
        horizontalLayout_27->setObjectName(QString::fromUtf8("horizontalLayout_27"));
        label_29 = new QLabel(groupBox);
        label_29->setObjectName(QString::fromUtf8("label_29"));

        horizontalLayout_27->addWidget(label_29);

        cBox_2 = new QComboBox(groupBox);
        cBox_2->addItem(QString());
        cBox_2->setObjectName(QString::fromUtf8("cBox_2"));
        cBox_2->setMinimumSize(QSize(120, 30));
        cBox_2->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_27->addWidget(cBox_2);


        gridLayout_4->addLayout(horizontalLayout_27, 1, 0, 1, 1);

        Bt_open = new QPushButton(groupBox);
        Bt_open->setObjectName(QString::fromUtf8("Bt_open"));
        Bt_open->setMinimumSize(QSize(80, 30));

        gridLayout_4->addWidget(Bt_open, 2, 0, 1, 1);


        verticalLayout_4->addWidget(groupBox);

        groupBox_5 = new QGroupBox(ioActionForm);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout_5 = new QGridLayout(groupBox_5);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        horizontalLayout_30 = new QHBoxLayout();
        horizontalLayout_30->setObjectName(QString::fromUtf8("horizontalLayout_30"));
        label_camera1_status = new QLabel(groupBox_5);
        label_camera1_status->setObjectName(QString::fromUtf8("label_camera1_status"));

        horizontalLayout_30->addWidget(label_camera1_status);

        frame_camera1 = new QFrame(groupBox_5);
        frame_camera1->setObjectName(QString::fromUtf8("frame_camera1"));
        frame_camera1->setMinimumSize(QSize(20, 20));
        frame_camera1->setStyleSheet(QString::fromUtf8("background-color: gray;"));
        frame_camera1->setFrameShape(QFrame::Box);
        frame_camera1->setFrameShadow(QFrame::Raised);

        horizontalLayout_30->addWidget(frame_camera1);


        verticalLayout_6->addLayout(horizontalLayout_30);

        horizontalLayout_31 = new QHBoxLayout();
        horizontalLayout_31->setObjectName(QString::fromUtf8("horizontalLayout_31"));
        label_camera2_status = new QLabel(groupBox_5);
        label_camera2_status->setObjectName(QString::fromUtf8("label_camera2_status"));

        horizontalLayout_31->addWidget(label_camera2_status);

        frame_camera2 = new QFrame(groupBox_5);
        frame_camera2->setObjectName(QString::fromUtf8("frame_camera2"));
        frame_camera2->setMinimumSize(QSize(20, 20));
        frame_camera2->setStyleSheet(QString::fromUtf8("background-color: gray;"));
        frame_camera2->setFrameShape(QFrame::Box);
        frame_camera2->setFrameShadow(QFrame::Raised);

        horizontalLayout_31->addWidget(frame_camera2);


        verticalLayout_6->addLayout(horizontalLayout_31);

        horizontalLayout_32 = new QHBoxLayout();
        horizontalLayout_32->setObjectName(QString::fromUtf8("horizontalLayout_32"));
        label_sync_status = new QLabel(groupBox_5);
        label_sync_status->setObjectName(QString::fromUtf8("label_sync_status"));

        horizontalLayout_32->addWidget(label_sync_status);

        frame_sync = new QFrame(groupBox_5);
        frame_sync->setObjectName(QString::fromUtf8("frame_sync"));
        frame_sync->setMinimumSize(QSize(20, 20));
        frame_sync->setStyleSheet(QString::fromUtf8("background-color: gray;"));
        frame_sync->setFrameShape(QFrame::Box);
        frame_sync->setFrameShadow(QFrame::Raised);

        horizontalLayout_32->addWidget(frame_sync);


        verticalLayout_6->addLayout(horizontalLayout_32);


        gridLayout_5->addLayout(verticalLayout_6, 0, 0, 1, 1);


        verticalLayout_4->addWidget(groupBox_5);

        textEdit = new QTextEdit(ioActionForm);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setMinimumSize(QSize(0, 150));

        verticalLayout_4->addWidget(textEdit);

        horizontalLayout_33 = new QHBoxLayout();
        horizontalLayout_33->setObjectName(QString::fromUtf8("horizontalLayout_33"));
        Bt_clear = new QPushButton(ioActionForm);
        Bt_clear->setObjectName(QString::fromUtf8("Bt_clear"));
        Bt_clear->setMinimumSize(QSize(80, 30));

        horizontalLayout_33->addWidget(Bt_clear);

        Bt_reset_sync = new QPushButton(ioActionForm);
        Bt_reset_sync->setObjectName(QString::fromUtf8("Bt_reset_sync"));
        Bt_reset_sync->setMinimumSize(QSize(80, 30));

        horizontalLayout_33->addWidget(Bt_reset_sync);


        verticalLayout_4->addLayout(horizontalLayout_33);


        gridLayout_3->addLayout(verticalLayout_4, 0, 0, 1, 1);

        groupBox_4 = new QGroupBox(ioActionForm);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_2 = new QGridLayout(groupBox_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_2 = new QGroupBox(groupBox_4);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_9 = new QVBoxLayout(groupBox_2);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_44 = new QLabel(groupBox_2);
        label_44->setObjectName(QString::fromUtf8("label_44"));

        verticalLayout->addWidget(label_44);

        label_45 = new QLabel(groupBox_2);
        label_45->setObjectName(QString::fromUtf8("label_45"));

        verticalLayout->addWidget(label_45);

        label_46 = new QLabel(groupBox_2);
        label_46->setObjectName(QString::fromUtf8("label_46"));

        verticalLayout->addWidget(label_46);

        label_47 = new QLabel(groupBox_2);
        label_47->setObjectName(QString::fromUtf8("label_47"));

        verticalLayout->addWidget(label_47);

        label_48 = new QLabel(groupBox_2);
        label_48->setObjectName(QString::fromUtf8("label_48"));

        verticalLayout->addWidget(label_48);

        label_49 = new QLabel(groupBox_2);
        label_49->setObjectName(QString::fromUtf8("label_49"));

        verticalLayout->addWidget(label_49);

        label_50 = new QLabel(groupBox_2);
        label_50->setObjectName(QString::fromUtf8("label_50"));

        verticalLayout->addWidget(label_50);

        label_51 = new QLabel(groupBox_2);
        label_51->setObjectName(QString::fromUtf8("label_51"));

        verticalLayout->addWidget(label_51);


        verticalLayout_9->addLayout(verticalLayout);

        horizontalLayout_29 = new QHBoxLayout();
        horizontalLayout_29->setObjectName(QString::fromUtf8("horizontalLayout_29"));
        Bt_get = new QPushButton(groupBox_2);
        Bt_get->setObjectName(QString::fromUtf8("Bt_get"));
        Bt_get->setMinimumSize(QSize(80, 30));

        horizontalLayout_29->addWidget(Bt_get);

        checkBox_auto_poll = new QCheckBox(groupBox_2);
        checkBox_auto_poll->setObjectName(QString::fromUtf8("checkBox_auto_poll"));
        checkBox_auto_poll->setChecked(true);

        horizontalLayout_29->addWidget(checkBox_auto_poll);


        verticalLayout_9->addLayout(horizontalLayout_29);


        verticalLayout_3->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(groupBox_4);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout = new QGridLayout(groupBox_3);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        checkBox_9 = new QCheckBox(groupBox_3);
        checkBox_9->setObjectName(QString::fromUtf8("checkBox_9"));

        verticalLayout_2->addWidget(checkBox_9);

        checkBox_10 = new QCheckBox(groupBox_3);
        checkBox_10->setObjectName(QString::fromUtf8("checkBox_10"));

        verticalLayout_2->addWidget(checkBox_10);

        checkBox_11 = new QCheckBox(groupBox_3);
        checkBox_11->setObjectName(QString::fromUtf8("checkBox_11"));

        verticalLayout_2->addWidget(checkBox_11);

        checkBox_12 = new QCheckBox(groupBox_3);
        checkBox_12->setObjectName(QString::fromUtf8("checkBox_12"));

        verticalLayout_2->addWidget(checkBox_12);

        checkBox_13 = new QCheckBox(groupBox_3);
        checkBox_13->setObjectName(QString::fromUtf8("checkBox_13"));

        verticalLayout_2->addWidget(checkBox_13);

        checkBox_14 = new QCheckBox(groupBox_3);
        checkBox_14->setObjectName(QString::fromUtf8("checkBox_14"));

        verticalLayout_2->addWidget(checkBox_14);

        checkBox_15 = new QCheckBox(groupBox_3);
        checkBox_15->setObjectName(QString::fromUtf8("checkBox_15"));

        verticalLayout_2->addWidget(checkBox_15);

        checkBox_16 = new QCheckBox(groupBox_3);
        checkBox_16->setObjectName(QString::fromUtf8("checkBox_16"));

        verticalLayout_2->addWidget(checkBox_16);


        gridLayout->addLayout(verticalLayout_2, 1, 0, 1, 1);

        Bt_start = new QPushButton(groupBox_3);
        Bt_start->setObjectName(QString::fromUtf8("Bt_start"));
        Bt_start->setMinimumSize(QSize(80, 30));

        gridLayout->addWidget(Bt_start, 2, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox_3);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(80, 30));
        label->setMaximumSize(QSize(80, 30));

        horizontalLayout->addWidget(label);

        cBox_3 = new QComboBox(groupBox_3);
        cBox_3->setObjectName(QString::fromUtf8("cBox_3"));
        cBox_3->setMinimumSize(QSize(80, 30));
        cBox_3->setMaximumSize(QSize(80, 30));

        horizontalLayout->addWidget(cBox_3);


        verticalLayout_5->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(80, 30));
        label_2->setMaximumSize(QSize(80, 30));

        horizontalLayout_2->addWidget(label_2);

        lineEdit = new QLineEdit(groupBox_3);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setMinimumSize(QSize(80, 30));
        lineEdit->setMaximumSize(QSize(80, 30));

        horizontalLayout_2->addWidget(lineEdit);


        verticalLayout_5->addLayout(horizontalLayout_2);


        horizontalLayout_3->addLayout(verticalLayout_5);

        Bt_save = new QPushButton(groupBox_3);
        Bt_save->setObjectName(QString::fromUtf8("Bt_save"));
        Bt_save->setMinimumSize(QSize(80, 80));
        Bt_save->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_3->addWidget(Bt_save);


        gridLayout->addLayout(horizontalLayout_3, 0, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox_3);


        gridLayout_2->addLayout(verticalLayout_3, 0, 0, 1, 1);


        gridLayout_3->addWidget(groupBox_4, 0, 1, 1, 1);


        retranslateUi(ioActionForm);

        QMetaObject::connectSlotsByName(ioActionForm);
    } // setupUi

    void retranslateUi(QWidget *ioActionForm)
    {
        ioActionForm->setWindowTitle(QCoreApplication::translate("ioActionForm", "IO Channel (PLC)", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ioActionForm", "PLC\350\256\276\347\275\256", nullptr));
        label_28->setText(QCoreApplication::translate("ioActionForm", "\345\267\245\346\216\247\346\234\272COM\345\217\243:", nullptr));
        Bt_refresh->setText(QCoreApplication::translate("ioActionForm", "\345\210\267\346\226\260", nullptr));
        label_29->setText(QCoreApplication::translate("ioActionForm", "\350\276\223\345\207\272\347\261\273\345\236\213:", nullptr));
        cBox_2->setItemText(0, QCoreApplication::translate("ioActionForm", "NPN", nullptr));

        Bt_open->setText(QCoreApplication::translate("ioActionForm", "\345\205\263\351\227\255", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ioActionForm", "\347\233\270\346\234\272\345\220\214\346\255\245\347\212\266\346\200\201", nullptr));
        label_camera1_status->setText(QCoreApplication::translate("ioActionForm", "\347\233\270\346\234\2721: \347\255\211\345\276\205", nullptr));
        label_camera2_status->setText(QCoreApplication::translate("ioActionForm", "\347\233\270\346\234\2722: \347\255\211\345\276\205", nullptr));
        label_sync_status->setText(QCoreApplication::translate("ioActionForm", "\345\220\214\346\255\245\347\212\266\346\200\201: \347\255\211\345\276\205", nullptr));
        Bt_clear->setText(QCoreApplication::translate("ioActionForm", "\346\270\205\347\251\272\346\227\245\345\277\227", nullptr));
        Bt_reset_sync->setText(QCoreApplication::translate("ioActionForm", "\351\207\215\347\275\256\345\220\214\346\255\245", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("ioActionForm", "IO\347\212\266\346\200\201", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ioActionForm", "\350\276\223\345\205\245\346\243\200\346\265\213 (DI)", nullptr));
        label_44->setText(QCoreApplication::translate("ioActionForm", "DI1 (\347\233\270\346\234\2721\351\207\207\351\233\206\345\256\214\346\210\220) - \344\275\216\347\224\265\345\271\263\350\247\246\345\217\221", nullptr));
        label_45->setText(QCoreApplication::translate("ioActionForm", "DI2 (\347\233\270\346\234\2722\351\207\207\351\233\206\345\256\214\346\210\220) - \344\275\216\347\224\265\345\271\263\350\247\246\345\217\221", nullptr));
        label_46->setText(QCoreApplication::translate("ioActionForm", "DI3 (\346\270\205\351\231\244\346\225\260\346\215\256\345\221\275\344\273\244) - \344\275\216\347\224\265\345\271\263\350\247\246\345\217\221", nullptr));
        label_47->setText(QCoreApplication::translate("ioActionForm", "DI4", nullptr));
        label_48->setText(QCoreApplication::translate("ioActionForm", "DI5", nullptr));
        label_49->setText(QCoreApplication::translate("ioActionForm", "DI6", nullptr));
        label_50->setText(QCoreApplication::translate("ioActionForm", "DI7", nullptr));
        label_51->setText(QCoreApplication::translate("ioActionForm", "DI8", nullptr));
        Bt_get->setText(QCoreApplication::translate("ioActionForm", "\350\216\267\345\217\226IO\347\224\265\345\271\263", nullptr));
        checkBox_auto_poll->setText(QCoreApplication::translate("ioActionForm", "\350\207\252\345\212\250\350\275\256\350\257\242", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ioActionForm", "\350\276\223\345\207\272\344\275\277\350\203\275 (DO)", nullptr));
        checkBox_9->setText(QCoreApplication::translate("ioActionForm", "DO1 (CCD1NG)", nullptr));
        checkBox_10->setText(QCoreApplication::translate("ioActionForm", "DO2 (CCD2NG)", nullptr));
        checkBox_11->setText(QCoreApplication::translate("ioActionForm", "DO3 (CCD1OK)", nullptr));
        checkBox_12->setText(QCoreApplication::translate("ioActionForm", "DO4 (CCD2OK)", nullptr));
        checkBox_13->setText(QCoreApplication::translate("ioActionForm", "DO5", nullptr));
        checkBox_14->setText(QCoreApplication::translate("ioActionForm", "DO6", nullptr));
        checkBox_15->setText(QCoreApplication::translate("ioActionForm", "DO7", nullptr));
        checkBox_16->setText(QCoreApplication::translate("ioActionForm", "DO8 (CCD\350\277\220\350\241\214)", nullptr));
        Bt_start->setText(QCoreApplication::translate("ioActionForm", "\345\274\200\345\247\213\346\211\247\350\241\214", nullptr));
        label->setText(QCoreApplication::translate("ioActionForm", "\350\276\223\345\207\272\347\253\257\345\217\243:", nullptr));
        label_2->setText(QCoreApplication::translate("ioActionForm", "\346\214\201\347\273\255\346\227\266\351\227\264(ms):", nullptr));
        Bt_save->setText(QCoreApplication::translate("ioActionForm", "\344\277\235\345\255\230", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ioActionForm: public Ui_ioActionForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IOACTIONFORM_H
