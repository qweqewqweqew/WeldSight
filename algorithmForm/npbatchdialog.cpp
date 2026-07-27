#include "npbatchdialog.h"
#include "ui_npbatchdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QDir>
#include <QListWidget>
#include <opencv2/imgcodecs.hpp>

NpBatchDialog::NpBatchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NpBatchDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("NP联合测试"));

    connect(ui->btnLoadN, &QPushButton::clicked, this, &NpBatchDialog::onLoadN);
    connect(ui->btnLoadP, &QPushButton::clicked, this, &NpBatchDialog::onLoadP);
    connect(ui->btnStart, &QPushButton::clicked, this, &NpBatchDialog::onStartTest);
    connect(ui->btnCancel, &QPushButton::clicked, this, &NpBatchDialog::reject);

    m_lastDirN = QDir::homePath();
    m_lastDirP = QDir::homePath();
}

NpBatchDialog::~NpBatchDialog()
{
    delete ui;
}

void NpBatchDialog::onLoadN()
{
    QString file = QFileDialog::getOpenFileName(this, tr("选择N端融合图"),
                                                m_lastDirN,
                                                tr("Images (*.png *.jpg *.bmp)"));
    if (file.isEmpty())
        return;
    QFileInfo fi(file);
    m_lastDirN = fi.absolutePath();
    if (!loadSingleImage(file, m_nImage)) {
        QMessageBox::warning(this, tr("错误"), tr("N端图像加载失败，请检查文件。"));
        return;
    }
    ui->listN->clear();
    ui->listN->addItem(file);
}

void NpBatchDialog::onLoadP()
{
    QString file = QFileDialog::getOpenFileName(this, tr("选择P端融合图"),
                                                m_lastDirP,
                                                tr("Images (*.png *.jpg *.bmp)"));
    if (file.isEmpty())
        return;
    QFileInfo fi(file);
    m_lastDirP = fi.absolutePath();
    if (!loadSingleImage(file, m_pImage)) {
        QMessageBox::warning(this, tr("错误"), tr("P端图像加载失败，请检查文件。"));
        return;
    }
    ui->listP->clear();
    ui->listP->addItem(file);
}

bool NpBatchDialog::loadSingleImage(const QString& file, cv::Mat& image)
{
    QTextCodec *code = QTextCodec::codecForName("GB2312");
    std::string path = code->fromUnicode(file).data();
    image = cv::imread(path, cv::IMREAD_COLOR);
    return !image.empty();
}

void NpBatchDialog::onStartTest()
{
    if (m_nImage.empty() || m_pImage.empty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先为N端和P端各选择1张融合图像。"));
        return;
    }
    accept();
}
