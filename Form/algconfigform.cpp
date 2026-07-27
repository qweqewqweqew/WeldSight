#include "algconfigform.h"
#include "ui_algconfigform.h"
#include "algorithmparammanager.h"
#include <QMessageBox>

AlgConfigForm::AlgConfigForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AlgConfigForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(tr("Algorithm Parameters"));

    // 设置范围
    ui->minWSpinBox->setRange(0.1, 100.0);
    ui->minHSpinBox->setRange(0.1, 100.0);
    ui->maxWSpinBox->setRange(0.1, 100.0);
    ui->maxHSpinBox->setRange(0.1, 100.0);
    ui->minAreaSpinBox->setRange(0.1, 1000.0);
    ui->maxAreaSpinBox->setRange(0.1, 1000.0);
    ui->maxTBDisSpinBox->setRange(0.01, 100.0);
    ui->minTBDisSpinBox->setRange(0.001, 100.0);
    ui->maxPYSpinBox->setRange(0.001, 100.0);
    ui->minPYSpinBox->setRange(0.001, 100.0);  // 新增
    ui->pix2mmScale_Z_x->setRange(0.0001, 1.0);
    ui->pix2mmScale_Z_y->setRange(0.0001, 1.0);
    ui->pix2mmScale_F_x->setRange(0.0001, 1.0);
    ui->pix2mmScale_F_y->setRange(0.0001, 1.0);

    // 瑕疵面积控制的范围设置
    ui->ltAllAreaSpinBox->setRange(0.001, 10.0);
    ui->ltMaxAreaSpinBox->setRange(0.001, 10.0);
    ui->hdAllAreaSpinBox->setRange(0.1, 100.0);
    ui->hdMaxAreaSpinBox->setRange(0.1, 100.0);
    ui->moonRadiusSpinBox->setRange(1.0, 1000.0);
    ui->moonDiffAreaSpinBox->setRange(0.001, 100.0);

    // 设置小数位数
    ui->minWSpinBox->setDecimals(2);
    ui->minHSpinBox->setDecimals(2);
    ui->maxWSpinBox->setDecimals(2);
    ui->maxHSpinBox->setDecimals(2);
    ui->minAreaSpinBox->setDecimals(2);
    ui->maxAreaSpinBox->setDecimals(2);
    ui->maxTBDisSpinBox->setDecimals(3);
    ui->minTBDisSpinBox->setDecimals(3);
    ui->maxPYSpinBox->setDecimals(3);
    ui->minPYSpinBox->setDecimals(3);
    ui->pix2mmScale_Z_x->setDecimals(6);
    ui->pix2mmScale_Z_y->setDecimals(6);
    ui->pix2mmScale_F_x->setDecimals(6);
    ui->pix2mmScale_F_y->setDecimals(6);

    // 瑕疵面积控制的小数位数
    ui->ltAllAreaSpinBox->setDecimals(3);
    ui->ltMaxAreaSpinBox->setDecimals(3);
    ui->hdAllAreaSpinBox->setDecimals(2);
    ui->hdMaxAreaSpinBox->setDecimals(2);
    ui->moonRadiusSpinBox->setDecimals(0);
    ui->moonDiffAreaSpinBox->setDecimals(3);

    // 读取配置
    readConfig();

}

AlgConfigForm::~AlgConfigForm()
{
    delete ui;
}

void AlgConfigForm::readConfig()
{
    AlgorithmParamManager::instance()->readConfig();
    InThresholdParam param = AlgorithmParamManager::instance()->getAlgParam();
    updateUiFromParam(param);
}

void AlgConfigForm::saveConfig()
{
    AlgorithmParamManager::instance()->saveConfig();
}

InThresholdParam AlgConfigForm::getAlgParam() const
{
    return AlgorithmParamManager::instance()->getAlgParam();
}

void AlgConfigForm::updateUiFromParam(const InThresholdParam &param)
{
    // 更新UI控件显示参数值
    ui->minWSpinBox->setValue(param.min_w);
    ui->minHSpinBox->setValue(param.min_h);
    ui->maxWSpinBox->setValue(param.max_w);
    ui->maxHSpinBox->setValue(param.max_h);

    ui->minAreaSpinBox->setValue(param.min_area);
    ui->maxAreaSpinBox->setValue(param.max_area);

    ui->maxTBDisSpinBox->setValue(param.max_TBDis);
    ui->minTBDisSpinBox->setValue(param.min_TBDis);

    ui->minPYSpinBox->setValue(param.min_PY);
    ui->maxPYSpinBox->setValue(param.max_PY);

    ui->pix2mmScale_Z_x->setValue(param.pix2mmScale_Z_x);
    ui->pix2mmScale_Z_y->setValue(param.pix2mmScale_Z_y);
    ui->pix2mmScale_F_x->setValue(param.pix2mmScale_F_x);
    ui->pix2mmScale_F_y->setValue(param.pix2mmScale_F_y);


    // 瑕疵面积控制的UI更新
    ui->ltAllAreaSpinBox->setValue(param.LT_ALLArea_max);
    ui->ltMaxAreaSpinBox->setValue(param.LT_MAXArea_max);
    ui->hdAllAreaSpinBox->setValue(param.HD_ALLArea_max);
    ui->hdMaxAreaSpinBox->setValue(param.HD_MAXArea_max);
    ui->moonRadiusSpinBox->setValue(param.min_radius);
    ui->moonDiffAreaSpinBox->setValue(param.HX_DiffArea_NG);
}

void AlgConfigForm::updateParamFromUi(InThresholdParam &param)
{
    // 从UI控件获取参数值
    param.min_w = ui->minWSpinBox->value();
    param.min_h = ui->minHSpinBox->value();
    param.max_w = ui->maxWSpinBox->value();
    param.max_h = ui->maxHSpinBox->value();

    param.min_area = ui->minAreaSpinBox->value();
    param.max_area = ui->maxAreaSpinBox->value();

    param.max_TBDis = ui->maxTBDisSpinBox->value();
    param.min_TBDis = ui->minTBDisSpinBox->value();

    param.min_PY = ui->minPYSpinBox->value();
    param.max_PY = ui->maxPYSpinBox->value();

    param.pix2mmScale_Z_x = ui->pix2mmScale_Z_x->value();
    param.pix2mmScale_Z_y = ui->pix2mmScale_Z_y->value();
    param.pix2mmScale_F_x = ui->pix2mmScale_F_x->value();
    param.pix2mmScale_F_y = ui->pix2mmScale_F_y->value();

    // 瑕疵面积控制的参数获取
    param.LT_ALLArea_max = ui->ltAllAreaSpinBox->value();
    param.LT_MAXArea_max = ui->ltMaxAreaSpinBox->value();
    param.HD_ALLArea_max = ui->hdAllAreaSpinBox->value();
    param.HD_MAXArea_max = ui->hdMaxAreaSpinBox->value();
    param.min_radius = ui->moonRadiusSpinBox->value();
    param.HX_DiffArea_NG = ui->moonDiffAreaSpinBox->value();
}

void AlgConfigForm::on_applyButton_clicked()
{
    InThresholdParam param;
    updateParamFromUi(param);
    AlgorithmParamManager::instance()->setAlgParam(param);
    QMessageBox::information(this, tr("Tip"), tr("Parameters applied!"));
}

void AlgConfigForm::on_saveButton_clicked()
{
    InThresholdParam param;
    updateParamFromUi(param);
    AlgorithmParamManager::instance()->setAlgParam(param);
    AlgorithmParamManager::instance()->saveConfig();
    QMessageBox::information(this, tr("Tip"), tr("Parameters saved!"));
}

void AlgConfigForm::on_resetButton_clicked()
{
    readConfig();
    QMessageBox::information(this, tr("Tip"), tr("Parameters reset!"));
}
