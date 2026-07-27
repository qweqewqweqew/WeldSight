#ifndef SAVEFORM_H
#define SAVEFORM_H

#include <QWidget>
#include <Common/common.h>

namespace Ui {
class SaveForm;
}

class SaveForm : public QWidget
{
    Q_OBJECT

public:
    explicit SaveForm(QWidget *parent = nullptr);
    ~SaveForm();

    void openFolder();
    void initCheckBoxes();
    void updateCurrentPathDisplay();  //更新路径显示
    void updateSaveDaysDisplay();     // 新增：更新保存天数显示

    // 新增：加载保存设置
    void loadSaveSettings();

    //获取相机的保存设置
    CameraSw getCameraSaveSettings(int cameraIndex) const;

    //设置相机的保存设置
    void setCameraSaveSettings(int cameraIndex, const CameraSw& settings);

signals:
    void sigSaveStateChanged();
    void sigSavePathChanged(const QString &newPath);  //保存路径更改信号
    void sigSaveDaysChanged(int days);                // 新增：保存天数更改信号

private slots:
    void on_Bt_confirm_clicked();
    void on_Bt_look_clicked();
    void onCheckBoxStateChanged(int);
    void on_Bt_changePath_clicked();
    void on_saveDaysSpinBox_valueChanged(int days);   // 新增：保存天数变化槽函数

public:
    SaveSw m_saveSw;

private:
    Ui::SaveForm *ui;

    // 添加成员指针数组声明
    static int CameraSw::*cameraSwMembers[4];

    //检查路径是否可写
    bool isPathWritable(const QString &path);
};

#endif // SAVEFORM_H
