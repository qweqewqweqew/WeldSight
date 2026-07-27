#include "saveform.h"
#include "ui_saveform.h"

#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QFileDialog>
#include <QSpinBox>

#include <Common/SysEnv.h>

// 添加成员指针数组定义
int CameraSw::*SaveForm::cameraSwMembers[4] = {
    &CameraSw::m_nSource,
    &CameraSw::m_nComposite,
    &CameraSw::m_nOK,
    &CameraSw::m_nNG
};

SaveForm::SaveForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SaveForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(tr("Save Image Settings"));

    // 新增：从配置文件加载保存设置
    loadSaveSettings();

    initCheckBoxes();
    updateCurrentPathDisplay();
    updateSaveDaysDisplay();  // 新增：初始化保存天数显示

}

SaveForm::~SaveForm()
{
    delete ui;
}

// 新增：添加加载保存设置的函数
void SaveForm::loadSaveSettings()
{
    // 从系统配置中加载所有相机的保存设置
    m_saveSw = SysEnv::instance().getAllCameraSaveSettings();
    emit sigSaveStateChanged();
}

void SaveForm::on_Bt_confirm_clicked()
{
    emit sigSaveStateChanged();
    this->hide();
}

void SaveForm::on_Bt_look_clicked()
{
    openFolder();
}

void SaveForm::openFolder() {
    QString imgPath = SysEnv::instance().getImageSavePath();
    imgPath = QDir::toNativeSeparators(imgPath);

    if (QDir(imgPath).exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(imgPath));
    }
    else {
        QMessageBox::warning(this, "Error", "Folder not found.");
    }
}

void SaveForm::initCheckBoxes() {
    for (int group = 1; group <= 6; group++) {
        for (int param = 1; param <= 4; param++) {
            QString objName = QString("cBox_%1_%2").arg(group).arg(param);
            QCheckBox* cb = findChild<QCheckBox*>(objName);
            if (cb) {
                cb->setProperty("groupIndex", group);
                cb->setProperty("paramIndex", param);
                connect(cb, &QCheckBox::stateChanged, this, &SaveForm::onCheckBoxStateChanged);

                // 初始化复选框状态
                CameraSw settings = getCameraSaveSettings(group);
                bool checked = false;
                switch(param) {
                case 1: checked = settings.m_nSource; break;
                case 2: checked = settings.m_nComposite; break;
                case 3: checked = settings.m_nOK; break;
                case 4: checked = settings.m_nNG; break;
                }
                cb->setChecked(checked);
            }
        }
    }

    // 新增：初始化保存天数SpinBox
    QSpinBox* saveDaysSpinBox = findChild<QSpinBox*>("saveDaysSpinBox");
    if (saveDaysSpinBox) {
        int currentDays = SysEnv::instance().getImageSaveDays();
        saveDaysSpinBox->setValue(currentDays);
        saveDaysSpinBox->setRange(1, 3650); // 1天到10年
        saveDaysSpinBox->setSuffix(tr(" 天"));
        connect(saveDaysSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &SaveForm::on_saveDaysSpinBox_valueChanged);
    }
}

void SaveForm::updateCurrentPathDisplay()
{
    QString currentPath = SysEnv::instance().getImageSavePath();
    ui->label_currentPath->setText(currentPath);
}

// 新增：更新保存天数显示
void SaveForm::updateSaveDaysDisplay()
{
    QSpinBox* saveDaysSpinBox = findChild<QSpinBox*>("saveDaysSpinBox");
    if (saveDaysSpinBox) {
        int currentDays = SysEnv::instance().getImageSaveDays();
        saveDaysSpinBox->setValue(currentDays);
    }
}

CameraSw SaveForm::getCameraSaveSettings(int cameraIndex) const
{
    switch(cameraIndex) {
    case 1: return m_saveSw.m_cSave1;
    case 2: return m_saveSw.m_cSave2;
    case 3: return m_saveSw.m_cSave3;
    case 4: return m_saveSw.m_cSave4;
    case 5: return m_saveSw.m_cSave5;
    case 6: return m_saveSw.m_cSave6;
    default: return CameraSw();
    }
}

// 修改：设置相机保存设置函数
void SaveForm::setCameraSaveSettings(int cameraIndex, const CameraSw &settings)
{
    switch(cameraIndex) {
    case 1: m_saveSw.m_cSave1 = settings; break;
    case 2: m_saveSw.m_cSave2 = settings; break;
    case 3: m_saveSw.m_cSave3 = settings; break;
    case 4: m_saveSw.m_cSave4 = settings; break;
    case 5: m_saveSw.m_cSave5 = settings; break;
    case 6: m_saveSw.m_cSave6 = settings; break;
    default: break;
    }

    // 更新UI
    for (int param = 1; param <= 4; param++) {
        QString objName = QString("cBox_%1_%2").arg(cameraIndex).arg(param);
        QCheckBox* cb = findChild<QCheckBox*>(objName);
        if (cb) {
            int value = 0;
            switch(param) {
            case 1: value = settings.m_nSource; break;
            case 2: value = settings.m_nComposite; break;
            case 3: value = settings.m_nOK; break;
            case 4: value = settings.m_nNG; break;
            }
            cb->setChecked(value == 1);
        }
    }

    // 新增：立即保存到配置文件
    SysEnv::instance().setAllCameraSaveSettings(m_saveSw);
}

// 修改：复选框状态改变时保存设置
void SaveForm::onCheckBoxStateChanged(int) {
    QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
    if (!cb) return;

    // 获取属性值
    int group = cb->property("groupIndex").toInt();
    int param = cb->property("paramIndex").toInt();

    // 获取对应的结构体和参数成员
    CameraSw& cameraSw = m_saveSw.getCameraSw(group);
    int value = cb->isChecked() ? 1 : 0;

    // 使用成员指针赋值
    switch(param) {
    case 1: cameraSw.m_nSource = value; break;
    case 2: cameraSw.m_nComposite = value; break;
    case 3: cameraSw.m_nOK = value; break;
    case 4: cameraSw.m_nNG = value; break;
    }

    // 更新内部状态
    setCameraSaveSettings(group, cameraSw);

    // 新增：立即保存到配置文件
    SysEnv::instance().setAllCameraSaveSettings(m_saveSw);
}

void SaveForm::on_Bt_changePath_clicked()
{
    // 获取当前保存路径
    QString currentPath = SysEnv::instance().getImageSavePath();

    // 如果当前是默认路径，提供一个合适的初始目录
    QString initialDir = currentPath;
    if (initialDir.isEmpty() || initialDir == QCoreApplication::applicationDirPath() + "/capture_root") {
        initialDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (initialDir.isEmpty()) {
            initialDir = QDir::homePath();
        }
    }
    // 使用文件对话框选择目录
    QString newPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select the image to save the directory"),
        initialDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!newPath.isEmpty()) {
        // Verify that the path is writable
        if (!isPathWritable(newPath)) {
            QMessageBox::warning(this,tr("Error"), tr("The selected directory does not have write permissions. Please choose another directory."));
            return;
        }

        // Check whether the new path is identical to the current one
        if (newPath == currentPath) {
            QMessageBox::information(this, tr("Notice"), tr("The save path has not changed."));
            return;
        }

        // Update the path in the system environment
        SysEnv::instance().setImageSavePath(newPath);

        // Refresh the UI display
        updateCurrentPathDisplay();

        // Emit a signal to notify that the path has changed
        emit sigSavePathChanged(newPath);

        QMessageBox::information(this, tr("Success"),QString(tr("The save path has been updated to:\n%1")).arg(newPath));
    }
}

// 新增：保存天数变化槽函数
void SaveForm::on_saveDaysSpinBox_valueChanged(int days)
{
    // 更新系统环境中的保存天数
    SysEnv::instance().setImageSaveDays(days);

    // 发出信号通知保存天数已更改
    emit sigSaveDaysChanged(days);
}

bool SaveForm::isPathWritable(const QString &path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        // 如果路径不存在，尝试创建
        QDir dir;
        return dir.mkpath(path);
    }

    if (!fileInfo.isDir() || !fileInfo.isWritable()) {
        return false;
    }

    // 尝试创建测试文件
    QString testFilePath = path + "/test_write_permission.tmp";
    QFile testFile(testFilePath);
    if (testFile.open(QIODevice::WriteOnly)) {
        testFile.close();
        testFile.remove();
        return true;
    }

    return false;
}
