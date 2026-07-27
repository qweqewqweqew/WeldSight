#ifndef NPBATCHDIALOG_H
#define NPBATCHDIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <vector>
#include <opencv2/core/mat.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class NpBatchDialog; }
QT_END_NAMESPACE

// 简单的本地批量测试对话框，分别为N端/P端选择各1张融合图
class NpBatchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NpBatchDialog(QWidget *parent = nullptr);
    ~NpBatchDialog();

    const cv::Mat& nImage() const { return m_nImage; }
    const cv::Mat& pImage() const { return m_pImage; }

private slots:
    void onLoadN();
    void onLoadP();
    void onStartTest();

private:
    bool loadSingleImage(const QString& file, cv::Mat& image);

private:
    Ui::NpBatchDialog *ui;
    cv::Mat m_nImage;
    cv::Mat m_pImage;
    QString m_lastDirN;
    QString m_lastDirP;
};

#endif // NPBATCHDIALOG_H
