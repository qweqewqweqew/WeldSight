#include "algorithmtester.h"
#include "ui_algorithmtester.h"
#include <QFileDialog>
#include <QProgressDialog>
#include <QThread>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QDebug>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTextStream>
#include <QDateTime>
#include <QLabel>
#include <QSpinBox>
#include <QTextCodec>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <Alg/ImageConvert.h>
#include <Alg/weldingalgorithmmanager.h>
#include <Form/algorithmparammanager.h>
#include <Common/cvqt_text.hpp>
#include "npbatchdialog.h"

namespace {

// 结果图文字按分辨率相对 960 基准缩放，保证大图缩放到界面后文字清晰
float resultTextScale(const cv::Mat& image)
{
    if (image.empty()) return 1.0f;
    const float scale = (std::max)(static_cast<float>(image.cols) / 960.0f,
                                   static_cast<float>(image.rows) / 720.0f);
    return (std::max)(1.0f, scale);
}

std::vector<LineStyle> scaleLineStyles(const std::vector<LineStyle>& styles, float scale)
{
    std::vector<LineStyle> scaled = styles;
    for (LineStyle& style : scaled)
        style.fontSize = (std::max)(1, static_cast<int>(style.fontSize * scale + 0.5f));
    return scaled;
}

}

AlgorithmTester::AlgorithmTester(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AlgorithmTester)
    , m_worker(nullptr)
    , m_workerThread(nullptr)
    , m_progressDlg(nullptr)
    , m_testTimer(nullptr)
    , m_testCount(0)
    , m_lastDir(QDir::homePath()) // 初始化上次目录为用户主目录
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle(tr("Local Test (Algo Debug)"));

    initWindow();
    ui->cBox_1->addItem(tr("CCD1"));
    ui->cBox_1->addItem(tr("CCD2"));

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->cBox_1->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        item->setSizeHint(QSize(item->sizeHint().width(), 30)); // 设置每一项高度
    }

    connect(ui->btnLoadFour, &QPushButton::clicked, this, &AlgorithmTester::onLoadFour);
    connect(ui->btnLoadOne,  &QPushButton::clicked, this, &AlgorithmTester::onLoadOne);
    connect(ui->btnNpBatch, &QPushButton::clicked, this, &AlgorithmTester::onNpBatchTest);
    connect(ui->btnLoopTest, &QPushButton::clicked, this, &AlgorithmTester::onLoopTest);
    QSpinBox* weldPointSpin = new QSpinBox(this);
    weldPointSpin->setObjectName("spinWeldPoint");
    weldPointSpin->setRange(0, 20);
    weldPointSpin->setSpecialValueText(tr("Auto"));
    weldPointSpin->setMinimumSize(90, 30);
    ui->horizontalLayout_Buttons->insertWidget(3, new QLabel(tr("Weld:"), this));
    ui->horizontalLayout_Buttons->insertWidget(4, weldPointSpin);
    // 创建测试定时器
    m_testTimer = new QTimer(this);
    // m_testTimer->setSingleShot(false);
    connect(m_testTimer, &QTimer::timeout, this, &AlgorithmTester::onTestTimer);


    // 初始化本地参数
    connect(AlgorithmParamManager::instance(), &AlgorithmParamManager::algParamChanged,
            this, &AlgorithmTester::onAlgParamChanged);
    m_currentAlgParam = AlgorithmParamManager::instance()->getAlgParam();
}

AlgorithmTester::~AlgorithmTester()
{
    stopMemoryLeakTest();

    // 清理工作线程
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            m_workerThread->wait();
        }
        delete m_workerThread;
    }

    delete m_worker;
    delete ui;
}

void AlgorithmTester::initWindow()
{
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 自动调整窗口列表的【宽、高】---适应窗口大小
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget->verticalHeader()->hide(); // 取消默认行号
    QStringList header;
    header<< tr("HXID")
           << tr("Weight(mm)")
           << tr("Height(mm)")
           << tr("Area(mm²)")
           << tr("TBtoHX(mm)")
           << tr("Offset(mm)")
           << tr("LTsum(mm²)")
           << tr("LTmax(mm²)")
           << tr("HDsum(mm²)")
           << tr("HDmax(mm²)")
           << tr("NO_HX_NGReults");
    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);


    /* 关键：让列宽根据内容自适应，并允许用户拖动 */
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /* 如果想再紧一点，可以限制最小宽度 */
    for (int c = 0; c < header.size(); ++c)
        ui->tableWidget->horizontalHeader()->setMinimumSectionSize(80);

    ui->tableWidget->setAlternatingRowColors(true);

}

void AlgorithmTester::startMemoryLeakTest(const QString &testDir, int intervalMs)
{
    m_testDir = testDir;
    m_testCount = 0;
    m_testTimer->start(intervalMs);
    qDebug() << "Starting memory leak test, directory:" << testDir << "interval:" << intervalMs << "ms";
}

void AlgorithmTester::stopMemoryLeakTest()
{
    if (m_testTimer && m_testTimer->isActive()) {
        m_testTimer->stop();
        qDebug() << "Stopped memory leak test, total runs:" << m_testCount;
    }
}


void AlgorithmTester::onTestTimer()
{

}

void AlgorithmTester::setupInputImage(int index, const QString &path)
{
    QLabel *lab = findChild<QLabel*>(QString("labelInput%1").arg(index));
    if (lab) {
        QImage img(path);
        if (!img.isNull()) {
            lab->setPixmap(QPixmap::fromImage(
                img.scaled(lab->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
    }
}

void AlgorithmTester::onAlgParamChanged(const InThresholdParam &param)
{
    m_currentAlgParam = param;
    Obj<WeldingDetector>::GetInstance()->SetThresholdParams(param);


    auto setHeader = [this](int col, const QString &base,
                            float min, float max, const QString &unit = "")
    {
        QString txt = unit.isEmpty()
        ? QString("%1(%2~%3)").arg(base).arg(min).arg(max)
        : QString("%1(%2~%3 %4)").arg(base).arg(min).arg(max).arg(unit);
        ui->tableWidget->horizontalHeaderItem(col)->setText(txt);
    };

    setHeader(1, tr("Weight"), param.min_w, param.max_w, "mm");
    setHeader(2, tr("Height"), param.min_h, param.max_h, "mm");
    setHeader(3, tr("Area"),   param.min_area, param.max_area, "mm²");
    setHeader(4, tr("TBtoHX"), param.min_TBDis, param.max_TBDis, "mm");
    setHeader(5, tr("Offset"), param.min_PY, param.max_PY, "mm");
    setHeader(6, tr("LTsum"), 0, param.LT_ALLArea_max, "mm²");
    setHeader(7, tr("LTmax"), 0, param.LT_MAXArea_max, "mm²");
    setHeader(8, tr("HDsum"), 0, param.HD_ALLArea_max, "mm²");
    setHeader(9, tr("HDmax"), 0, param.HD_MAXArea_max, "mm²");
}

void AlgorithmTester::onLoadFour()
{
    // 清除之前的图像
    clearImages();
    m_isFourMode = true;
    m_currentTestImages.clear();

    // 允许选择多个图像文件
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select 4 images (or cancel to load first 4 from folder)"),
                                                      m_lastDir,
                                                      tr("Images (*.png *.jpg *.bmp)"));

    // 如果用户选择了文件，更新最后访问目录
    if (!files.isEmpty()) {
        QFileInfo fileInfo(files.first());
        m_lastDir = fileInfo.absolutePath();
    }

    // 如果没有选择任何文件，则从目录中加载前4个图像
    if (files.isEmpty()) {
        QDir d(m_lastDir);
        QStringList filters = { "*.png", "*.jpg", "*.bmp" };
        QStringList allFiles = d.entryList(filters, QDir::Files, QDir::Name);

        if (allFiles.size() < 4) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Folder must contain at least 4 images. Found only %1.").arg(allFiles.size()));
            return;
        }

        // 取前4个文件
        for (int i = 0; i < 4 && i < allFiles.size(); ++i) {
            files.append(d.filePath(allFiles[i]));
        }
    }
    // 如果选择了文件但不足4个，尝试从同一目录补足
    else if (files.size() < 4) {
        QDir d(QFileInfo(files.first()).absolutePath());
        QStringList filters = { "*.png", "*.jpg", "*.bmp" };
        QStringList allFiles = d.entryList(filters, QDir::Files, QDir::Name);

        // 移除已经选择的文件
        for (const QString& selectedFile : files) {
            QFileInfo fi(selectedFile);
            allFiles.removeAll(fi.fileName());
        }

        // 补足到4个文件
        while (files.size() < 4 && !allFiles.isEmpty()) {
            files.append(d.filePath(allFiles.takeFirst()));
        }

        if (files.size() < 4) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Not enough images in folder. Only %1 images available.").arg(files.size()));
            // 继续处理，但只加载可用的图像
        }
    }
    // 如果选择了超过4个文件，只取前4个
    else if (files.size() > 4) {
        files = files.mid(0, 4);
        QMessageBox::information(this, tr("Information"),
                                 tr("Only the first 4 images will be loaded."));
    }

    QTextCodec *code = QTextCodec::codecForName("GB2312");//解决中文路径问题

    // 加载图像
    for (int i = 0; i < files.size() && i < 4; ++i) {
        QString filePath = files[i];
        std::string strFileName = code->fromUnicode(filePath).data();
        cv::Mat img = cv::imread(strFileName, cv::IMREAD_COLOR);
        if (img.empty()) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load image: %1").arg(filePath));
            clearImages();
            return;
        }
        m_inputImages.push_back(img);
        m_currentTestImages.push_back(filePath);
        setupInputImage(i, filePath);
    }

    // 如果有4张图像，运行算法
    if (m_inputImages.size() == 4) {
        runAlgorithm();
    } else {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Algorithm requires 4 images. Only %1 images loaded.").arg(m_inputImages.size()));
    }
}

void AlgorithmTester::onLoadOne()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select fused image"),
                                                m_lastDir,
                                                tr("Images (*.png *.jpg *.bmp)"));
    if (file.isEmpty()) return;

    // 更新最后访问目录
    QFileInfo fileInfo(file);
    m_lastDir = fileInfo.absolutePath();
    QTextCodec *code = QTextCodec::codecForName("GB2312");//解决中文路径问题
    std::string strFileName = code->fromUnicode(file).data();
    cv::Mat img = cv::imread(strFileName, cv::IMREAD_COLOR);
    if (img.empty()) return;

    clearImages();
    m_isFourMode = false;
    m_currentTestImages.clear();
    m_currentTestImages.push_back(file);
    m_inputImages.push_back(img);

    cv::Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);
    QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    ui->labelFusion->setPixmap(QPixmap::fromImage(
        qimg.copy().scaled(ui->labelFusion->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    qDebug() << "AlgorithmTester::onLoadOne()";
    runAlgorithmOne();

}

void AlgorithmTester::onNpBatchTest()
{
    NpBatchDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    // 清除之前的显示
    clearImages();
    m_currentTestImages.clear();
    m_inputImages.clear();
    m_isFourMode = false;

    // 分别入队 N/P (单张融合图)
    const cv::Mat& nImg = dlg.nImage();
    const cv::Mat& pImg = dlg.pImage();
    if (!nImg.empty()) {
        Alg::CameraFunc(nImg, 0);
    }
    if (!pImg.empty()) {
        Alg::CameraFunc(pImg, 1);
    }
}

void AlgorithmTester::onLoopTest()
{
    // 如果正在循环测试，则停止
    if (m_isLoopTesting) {
        m_isLoopTesting = false;
        ui->btnLoopTest->setText(tr("循环测试"));
        // 关闭文件
        if (m_loopLogFile) {
            m_loopLogFile->close();
            delete m_loopLogFile;
            m_loopLogFile = nullptr;
        }
        return;
    }

    // 1. 弹窗选择图像
    NpBatchDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    // 2. 预加载图像到内存（只执行一次）- 单张融合图
    m_loopNImage = dlg.nImage().clone();
    m_loopPImage = dlg.pImage().clone();

    if (m_loopNImage.empty() || m_loopPImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("N端和P端各需要1张融合图像"));
        return;
    }

    // 3. 选择测试模式
    QStringList modes;
    modes << tr("双路测试 (N+P)") << tr("仅 N 端") << tr("仅 P 端");
    bool ok;
    QString selectedMode = QInputDialog::getItem(this, tr("测试模式"),
                                                  tr("请选择测试模式:"),
                                                  modes, 0, false, &ok);
    if (!ok) {
        return;
    }
    m_loopMode = modes.indexOf(selectedMode);  // 0=双路, 1=仅N, 2=仅P
    m_loopExpectCount = (m_loopMode == 0) ? 2 : 1;

    // 4. 询问循环次数
    int loopCount = QInputDialog::getInt(this, tr("循环测试"),
                                         tr("请输入循环次数:"),
                                         100, 1, 10000, 1, &ok);
    if (!ok) {
        return;
    }

    // 5. 创建桌面日志文件
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString modeSuffix = (m_loopMode == 0) ? "NP" : (m_loopMode == 1 ? "N" : "P");
    QString logPath = desktop + "/loop_test_" + modeSuffix + "_" + timestamp + ".txt";
    
    m_loopLogFile = new QFile(logPath);
    if (!m_loopLogFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法创建日志文件: %1").arg(logPath));
        delete m_loopLogFile;
        m_loopLogFile = nullptr;
        return;
    }

    // 写入文件头
    QTextStream out(m_loopLogFile);
    out << "=== Loop Test Started ===" << "\n";
    out << "Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    out << "Mode: " << selectedMode << "\n";
    out << "Total Loops: " << loopCount << "\n";
    out << "=========================" << "\n";
    out << "Loop\tCam\tTotal\tClone\tResize\tInfer\tBBox\tROI\tImwrite\tResizePlus\tSeg\tPost1\tPost2\tPost3\tPost4\tDraw" << "\n";
    out.flush();

    // 6. 开始循环测试
    m_loopTotal = loopCount;
    m_loopCurrent = 0;
    m_loopResultCount = 0;
    m_isLoopTesting = true;
    ui->btnLoopTest->setText(tr("停止测试"));

    // 7. 启动第一轮 (单张融合图)
    m_loopTimer.start();
    if (m_loopMode == 0 || m_loopMode == 1) {
        Alg::CameraFunc(m_loopNImage, 0);  // N端
    }
    if (m_loopMode == 0 || m_loopMode == 2) {
        Alg::CameraFunc(m_loopPImage, 1);  // P端
    }
}

void AlgorithmTester::runAlgorithm()
{
    if (m_inputImages.size() == 0) return;
    int nCamera = ui->cBox_1->currentIndex();
    int weldPoint = 0;
    if (QSpinBox* spin = findChild<QSpinBox*>("spinWeldPoint")) {
        weldPoint = spin->value();
    }
    m_Elapsetimer.start();
    Alg::CameraFuncForWeldPoint(m_inputImages, nCamera, weldPoint);
}

void AlgorithmTester::runAlgorithmOne()
{
    if (m_inputImages.size() == 0) return;
    int nCamera = ui->cBox_1->currentIndex();
    int weldPoint = 0;
    if (QSpinBox* spin = findChild<QSpinBox*>("spinWeldPoint")) {
        weldPoint = spin->value();
    }
    m_Elapsetimer.start();
    Alg::CameraFuncForWeldPoint(m_inputImages[0], nCamera, weldPoint);
}

void AlgorithmTester::onAlgorithmFinished(const WeldingResult &result)
{
    // 循环测试模式
    if (m_isLoopTesting) {
        // 记录每次算法关键步骤耗时
        if (m_loopLogFile && m_loopLogFile->isOpen()) {
            QTextStream out(m_loopLogFile);
            out << m_loopCurrent + 1 << "\t"
                << (result.id + 1) << "\t"
                << result.t_total << "\t"
                << result.t_clone << "\t"
                << result.t_resize << "\t"
                << result.t_infer << "\t"
                << result.t_bbox << "\t"
                << result.t_roi << "\t"
                << result.t_imwrite << "\t"
                << result.t_resizePlus << "\t"
                << result.t_seg << "\t"
                << result.t_post1 << "\t"
                << result.t_post2 << "\t"
                << result.t_post3 << "\t"
                << result.t_post4 << "\t"
                << result.t_draw << "\n";
            out.flush();
        }

        m_loopResultCount++;

        // 等待预期数量的结果
        if (m_loopResultCount >= m_loopExpectCount) {
            m_loopCurrent++;
            m_loopResultCount = 0;

            // 检查是否继续
            if (m_loopCurrent < m_loopTotal && m_isLoopTesting) {
                // 启动下一轮 (单张融合图)
                if (m_loopMode == 0 || m_loopMode == 1) {
                    Alg::CameraFunc(m_loopNImage, 0);  // N端
                }
                if (m_loopMode == 0 || m_loopMode == 2) {
                    Alg::CameraFunc(m_loopPImage, 1);  // P端
                }
            } else {
                // 测试完成，写入统计信息并关闭文件
                if (m_loopLogFile && m_loopLogFile->isOpen()) {
                    QTextStream out(m_loopLogFile);
                    out << "=========================" << "\n";
                    out << "Test Completed: " << m_loopCurrent << " loops" << "\n";
                    out.flush();
                    m_loopLogFile->close();
                    delete m_loopLogFile;
                    m_loopLogFile = nullptr;
                }
                
                m_isLoopTesting = false;
                ui->btnLoopTest->setText(tr("循环测试"));
                QMessageBox::information(this, tr("完成"),
                    tr("循环测试完成，共 %1 轮\n结果已保存到桌面").arg(m_loopCurrent));
            }
        }
        return;  // 循环测试模式下不更新 UI
    }

    QStringList lines;
    std::vector<LineStyle> styles;
    QString strNGReults;
    switch (result.NO_HX_NGReults)
    {
    case OK:
        strNGReults = tr("OK");
        lines << strNGReults;
        styles.push_back(LineStyle{40, Qt::green, "Consolas", QFont::Bold});
        break;
    case HXAREA_Err:
        strNGReults = tr("HXAREA_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.HX_Area, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});

        strNGReults = tr("LT_SUM:");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.LT_SumArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});

        strNGReults = tr("LT_MAX:");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.LT_MaxArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});

        strNGReults = tr("HD_SUM:");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.HD_SumArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});

        strNGReults = tr("HD_MAX:");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.HD_MaxArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});

        strNGReults = tr("HXAREA_Err");
        break;
    case HXSIZE_H_Err:
        strNGReults = tr("HXSIZE_H_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.hxResult[0].hx_height, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case HXSIZE_W_Err:
        strNGReults = tr("HXSIZE_W_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.hxResult[0].hx_weight, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case YJ_Err:
        strNGReults = tr("YJ_Err");
        lines << strNGReults;
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case HXPY_Err:
        strNGReults = tr("HXPY_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.hxResult[0].center_pianyi, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case TBDIS_Err:
        strNGReults = tr("TBDIS_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.hxResult[0].TBtoHX_distance, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case LT_Err:
        strNGReults = tr("LT_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.LT_SumArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case HD_Err:
        strNGReults = tr("HD_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.HD_SumArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case HXGAP_Err:
        strNGReults = tr("HXGAP_Err");
        lines << QString("%1(%2)").arg(strNGReults).arg(result.HX_DiffArea, 0, 'f', 2);
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    case NG:
        strNGReults = tr("NG");
        lines << strNGReults;
        styles.push_back(LineStyle{30, Qt::red, "Consolas"});
        break;
    default:
        break;
    }

    /* ---------- 2. 其余逻辑保持不变 ---------- */
    qint64 ms = m_Elapsetimer.elapsed();
    lines << QString(tr("elapsed:%1ms")).arg(ms);
    styles.push_back(LineStyle{30, Qt::green, "Consolas", QFont::Bold});

    // 按图像分辨率放大字号、起点和行距，保证大图显示时文字清晰
    const float textScale = resultTextScale(result.dst);
    const std::vector<LineStyle> scaledStyles = scaleLineStyles(styles, textScale);
    const cv::Point textOrigin(static_cast<int>(30 * textScale + 0.5f),
                               static_cast<int>(50 * textScale + 0.5f));
    const int lineSpacing = static_cast<int>(10 * textScale + 0.5f);
    QPixmap pix = cvqt::drawText(result.dst, lines, scaledStyles, textOrigin, lineSpacing);
    ui->labelResult->setPixmap(pix);

    ui->labelFusion->setPixmap(
        QPixmap::fromImage(
            ImageConvert::CvMatToQImage(result.FusionMap)
                .scaled(ui->labelFusion->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    ui->tableWidget->setRowCount(1);
    for (int i = 0; i < result.hxResult.size(); ++i)
    {
        const HXResult &r = result.hxResult.at(i);
        auto mkItem = [](const QString &text) -> QTableWidgetItem* {
            auto it = new QTableWidgetItem(text);
            it->setTextAlignment(Qt::AlignCenter);
            return it;
        };
        ui->tableWidget->setItem(i, 0, mkItem(QString::number(r.hxid)));
        ui->tableWidget->setItem(i, 1, mkItem(QString::number(r.hx_weight, 'f', 2)));
        ui->tableWidget->setItem(i, 2, mkItem(QString::number(r.hx_height, 'f', 2)));
        ui->tableWidget->setItem(i, 3, mkItem(QString::number(result.HX_Area, 'f', 2)));
        ui->tableWidget->setItem(i, 4, mkItem(QString::number(r.TBtoHX_distance, 'f', 2)));
        ui->tableWidget->setItem(i, 5, mkItem(QString::number(r.center_pianyi, 'f', 2)));
        ui->tableWidget->setItem(i, 6, mkItem(QString::number(result.LT_SumArea, 'f', 2)));
        ui->tableWidget->setItem(i, 7, mkItem(QString::number(result.LT_MaxArea, 'f', 2)));
        ui->tableWidget->setItem(i, 8, mkItem(QString::number(result.HD_SumArea, 'f', 2)));
        ui->tableWidget->setItem(i, 9, mkItem(QString::number(result.HD_MaxArea, 'f', 2)));
        ui->tableWidget->setItem(i, 10, mkItem(strNGReults));
        break;
    }
}

void AlgorithmTester::onAlgorithmError(const QString &err)
{
    if (m_progressDlg) {
        m_progressDlg->close();
        m_progressDlg->deleteLater();
        m_progressDlg = nullptr;
    }
}

void AlgorithmTester::clearImages()
{
    m_inputImages.clear();
    m_currentTestImages.clear();
    for (int i = 0; i < 4; ++i) {
        QLabel *lab = findChild<QLabel*>(QString("labelInput%1").arg(i));
        if (lab) lab->clear();
    }
    ui->labelFusion->clear();
    ui->labelResult->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void AlgorithmTester::onAlgorithmProgress(int percent, const QString &msg)
{
    if (m_progressDlg) {
        m_progressDlg->setValue(percent);
        m_progressDlg->setLabelText(msg);
    }
}



void AlgorithmTester::on_checkBox_toggled(bool checked)
{
    Obj<WeldingDetector>::GetInstance()->SetAutoCalibration(checked);
}
