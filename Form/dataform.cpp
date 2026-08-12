#include "algorithmparammanager.h"
#include "dataform.h"
#include "ui_dataform.h"
#include <QGraphicsDropShadowEffect>
#include <QListView>
#include <QMessageBox>
#include <QDate>
#include <QByteArray>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QProgressDialog>
#include <QSignalBlocker>
#include <QTextCodec>

namespace {

void appendCsvCell(QString &line, const QString &value, bool lastCell = false)
{
    QString cell = value;
    const bool needsQuotes = cell.contains(',') || cell.contains('"') ||
                             cell.contains('\n') || cell.contains('\r');
    if (needsQuotes) {
        cell.replace("\"", "\"\"");
        line.append('"');
        line.append(cell);
        line.append('"');
    } else {
        line.append(cell);
    }

    if (!lastCell) {
        line.append(',');
    }
}

void appendCsvRow(QString &buffer, const QStringList &fields)
{
    for (int i = 0; i < fields.size(); ++i) {
        appendCsvCell(buffer, fields.at(i), i == fields.size() - 1);
    }
    buffer.append('\n');
}

}

DataForm::DataForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataForm)
{
    ui->setupUi(this);


    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowModality(Qt::ApplicationModal);
    initWindow();

    // 设置日期范围，默认最近7天
    QDate currentDate = QDate::currentDate();
    ui->dateStart->setDate(currentDate.addDays(-7));
    ui->dateEnd->setDate(currentDate);

    m_nCurPage = 1;
    m_nRowsPerPage = 10;
    m_nTotalPages = 1;

    ui->Bt_Search->setEnabled(false);
    ui->Bt_Delete->setEnabled(false);
    m_nCurID = -1;

    connect(ui->tableWidget->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(OnSlideAction(int)));
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged,
            this, &DataForm::updateSelectionActions);

    // 样式设置
    QFile file(":/qss/dlg.qss");
    if (file.open(QFile::ReadOnly)) {
        QTextStream filetext(&file);
        QString stylesheet = filetext.readAll();
        this->setStyleSheet(stylesheet);
        file.close();
    }

    // 阴影效果
    QGraphicsDropShadowEffect *shadow_effectA = new QGraphicsDropShadowEffect(this);
    QGraphicsDropShadowEffect *shadow_effectB = new QGraphicsDropShadowEffect(this);
    QGraphicsDropShadowEffect *shadow_effectC = new QGraphicsDropShadowEffect(this);

    shadow_effectA->setOffset(0, 0);
    shadow_effectA->setColor(QColor(150,150,150));
    shadow_effectA->setBlurRadius(6);

    shadow_effectB->setOffset(0, 0);
    shadow_effectB->setColor(QColor(150,150,150));
    shadow_effectB->setBlurRadius(6);

    shadow_effectC->setOffset(0, 0);
    shadow_effectC->setColor(QColor(150,150,150));
    shadow_effectC->setBlurRadius(6);

    ui->Data_gBox_1->setGraphicsEffect(shadow_effectA);
    ui->Data_gBox_2->setGraphicsEffect(shadow_effectB);
    ui->Data_gBox_3->setGraphicsEffect(shadow_effectC);


    m_sql.init("welding_db");
    // 初始加载数据
    updateWeldingData();
}

DataForm::~DataForm()
{
    delete ui;
}

void DataForm::initWindow()
{
    // 表格设置
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 支持Ctrl/Shift多选
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->verticalHeader()->hide();
    InThresholdParam param = AlgorithmParamManager::instance()->getAlgParam();
    // 设置表格列（焊锡检测结果）
    QStringList header;
    header  << tr("id")
           << tr("治具码")
           << tr("外壳码")
           << tr("结果图片")
           << tr("hxPort")
           << tr("hxId")
           << tr("Weight(%1~%2mm)").arg(param.min_w).arg(param.max_w)
           << tr("Height(%1~%2mm)").arg(param.min_h).arg(param.max_h)
           << tr("Area(%1~%2mm²)").arg(param.min_area).arg(param.max_area)
           << tr("TBtoHX(%1~%2mm)").arg(param.min_TBDis).arg(param.max_TBDis)
           << tr("Offset(%1~%2mm)").arg(param.min_PY).arg(param.max_PY)
           << tr("LTsum(0~%1mm²)").arg(param.LT_ALLArea_max)
           << tr("LTmax(0~%1mm²)").arg(param.LT_MAXArea_max)
           << tr("HDsum(0~%1mm²)").arg(param.HD_ALLArea_max)
           << tr("HDmax(0~%1mm²)").arg(param.HD_MAXArea_max)
           << tr("hxReult")
           << tr("hxReultsDescribe")
           << tr("changeTime");
    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);

    /* 关键：让列宽根据内容自适应，并允许用户拖动 */
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /* 如果想再紧一点，可以限制最小宽度 */
    for (int c = 0; c < header.size(); ++c)
        ui->tableWidget->horizontalHeader()->setMinimumSectionSize(80);
}

void DataForm::setWeldingDataPage()
{
    updateWeldingData();
}

void DataForm::updateWeldingData()
{


    QDateTime startTime = QDateTime(ui->dateStart->date(), QTime(0, 0, 0));
    QDateTime endTime = QDateTime(ui->dateEnd->date(), QTime(23, 59, 59));

    m_currentResults = m_sql.queryWeldingResultsByTimeRange(startTime, endTime);

    ui->tableWidget->clearSelection();
    ui->tableWidget->clearContents();

    // 计算分页
    if(m_currentResults.size() % m_nRowsPerPage == 0) {
        m_nTotalPages = m_currentResults.size() / m_nRowsPerPage;
    } else {
        m_nTotalPages = m_currentResults.size() / m_nRowsPerPage + 1;
    }

    if (m_nTotalPages == 0) m_nTotalPages = 1;
    m_nCurPage = qBound(1, m_nCurPage, m_nTotalPages);

    ui->labPage->setText(QString("%1/%2").arg(m_nCurPage).arg(m_nTotalPages));

    // 计算当前页显示的数据范围
    int startIndex = (m_nCurPage - 1) * m_nRowsPerPage;
    int endIndex = qMin(startIndex + m_nRowsPerPage, m_currentResults.size());

    ui->tableWidget->setRowCount(endIndex - startIndex);

    // 统计信息
    int totalCount = m_currentResults.size();
    int okCount = 0;
    int ngCount = 0;

    for (const auto& result : m_currentResults) {
        if (result.hxReult) {
            okCount++;
        } else {
            ngCount++;
        }
    }

    // 更新统计标签
    ui->data_number->setText(QString::number(totalCount));
    ui->data_weight->setText(QString::number(okCount));
    ui->data_ng_value->setText(QString::number(ngCount));

    for(int i = startIndex; i < endIndex; i++) {
        int row = i - startIndex;
        const DB_WeldingResult& result = m_currentResults[i];

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(result.id)));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(result.fixtureCode));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(result.shellCode));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(result.imageCode));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(result.hxPort));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(result.hxId)));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(result.hxWeight, 'f', 2)));
        ui->tableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(result.hxHeight, 'f', 2)));
        ui->tableWidget->setItem(row, 8, new QTableWidgetItem(QString::number(result.hxArea, 'f', 3)));
        ui->tableWidget->setItem(row, 9, new QTableWidgetItem(QString::number(result.hxDistance, 'f', 2)));
        ui->tableWidget->setItem(row, 10, new QTableWidgetItem(QString::number(result.hxPianyi, 'f', 2)));
        ui->tableWidget->setItem(row, 11, new QTableWidgetItem(QString::number(result.hxLTSumArea, 'f', 2)));
        ui->tableWidget->setItem(row, 12, new QTableWidgetItem(QString::number(result.hxLTMaxArea, 'f', 2)));
        ui->tableWidget->setItem(row, 13, new QTableWidgetItem(QString::number(result.hxHDSumArea, 'f', 2)));
        ui->tableWidget->setItem(row, 14, new QTableWidgetItem(QString::number(result.hxHDMaxArea, 'f', 2)));
        ui->tableWidget->setItem(row, 15, new QTableWidgetItem(result.hxReult ? tr("OK") : tr("NG")));
        ui->tableWidget->setItem(row, 16, new QTableWidgetItem(result.hxReultsDescribe));
        ui->tableWidget->setItem(row, 17, new QTableWidgetItem(result.changeTime));

        // 设置文本居中
        for(int j = 0; j < ui->tableWidget->columnCount(); j++) {
            if (ui->tableWidget->item(row, j)) {
                ui->tableWidget->item(row, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }
        }

        // 根据检测结果设置行颜色
        if (!result.hxReult) {
            for(int j = 0; j < ui->tableWidget->columnCount(); j++) {
                if (ui->tableWidget->item(row, j)) {
                    ui->tableWidget->item(row, j)->setBackground(QColor(255, 200, 200)); // NG显示为浅红色
                }
            }
        }
    }

    ui->cb_SelectAllResults->setEnabled(!m_currentResults.isEmpty());
    if (m_selectAllResults)
        ui->tableWidget->selectAll();
    updateSelectionActions();
}

void DataForm::OnSlideAction(int nRowIndex)
{
    ++nRowIndex;
    if (nRowIndex % m_nRowsPerPage == 0) {
        m_nCurPage = nRowIndex / m_nRowsPerPage;
    } else {
        m_nCurPage = nRowIndex / m_nRowsPerPage + 1;
    }
    ui->labPage->setText(QString("%1/%2").arg(m_nCurPage).arg(m_nTotalPages));
}

void DataForm::on_Bt_PrePage_clicked()
{
    m_nCurPage--;
    if(m_nCurPage <= 0){
        m_nCurPage = m_nTotalPages;
    }
    updateWeldingData();
}

void DataForm::on_Bt_NextPage_clicked()
{
    m_nCurPage++;
    if (m_nCurPage > m_nTotalPages) {
        m_nCurPage = 1;
    }
    updateWeldingData();
}

void DataForm::on_Bt_Delete_clicked()
{
    QList<int> selectedIds;
    if (m_selectAllResults) {
        selectedIds.reserve(m_currentResults.size());
        for (const DB_WeldingResult& result : m_currentResults)
            selectedIds.append(result.id);
    } else {
        // 普通多选只收集当前页选中行的 ID。
        const QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
        for (QTableWidgetItem* item : selectedItems) {
            QTableWidgetItem* idItem = ui->tableWidget->item(item->row(), 0);
            if (idItem) {
                const int id = idItem->text().toInt();
                if (!selectedIds.contains(id))
                    selectedIds.append(id);
            }
        }
    }

    if (selectedIds.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择要删除的记录"));
        return;
    }

    // 确认对话框
    QString msg;
    if (selectedIds.size() == 1) {
        msg = tr("确定要删除这条记录吗？");
    } else {
        msg = tr("确定要删除选中的 %1 条记录吗？").arg(selectedIds.size());
    }

    if (QMessageBox::question(this, tr("批量删除"), msg,
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        QList<int> idList = selectedIds;
        bool success = false;

        if (idList.size() == 1) {
            success = m_sql.deleteWeldingResult(idList.first());
        } else {
            success = m_sql.deleteWeldingResults(idList);
        }

        if (success) {
            QMessageBox::information(this, tr("成功"),
                tr("成功删除 %1 条记录").arg(idList.size()));
            m_selectAllResults = false;
            ui->cb_SelectAllResults->setChecked(false);
            updateWeldingData();
            m_nCurID = -1;
            ui->Bt_Delete->setEnabled(false);
            ui->Bt_Delete->setText(tr("删除"));
            ui->Bt_Search->setEnabled(false);
        } else {
            QMessageBox::warning(this, tr("错误"), tr("删除记录失败"));
        }
    }
}

void DataForm::on_Bt_Query_clicked()
{
    m_selectAllResults = false;
    ui->cb_SelectAllResults->setChecked(false);
    m_nCurPage = 1;
    updateWeldingData();
}

void DataForm::on_cb_SelectAllResults_toggled(bool checked)
{
    m_selectAllResults = checked && !m_currentResults.isEmpty();
    if (m_selectAllResults)
        ui->tableWidget->selectAll();
    else
        ui->tableWidget->clearSelection();
    updateSelectionActions();
}

void DataForm::updateSelectionActions()
{
    if (m_selectAllResults &&
        ui->tableWidget->selectionModel()->selectedRows().size() !=
            ui->tableWidget->rowCount()) {
        const QSignalBlocker blocker(ui->tableWidget->selectionModel());
        ui->tableWidget->selectAll();
    }

    QList<int> selectedRows;
    if (!m_selectAllResults) {
        const QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
        for (QTableWidgetItem* item : selectedItems) {
            if (!selectedRows.contains(item->row()))
                selectedRows.append(item->row());
        }
    }

    const int selectedCount = m_selectAllResults
        ? m_currentResults.size() : selectedRows.size();
    ui->Bt_Delete->setEnabled(selectedCount > 0);
    ui->Bt_Delete->setText(selectedCount > 1
        ? tr("删除(%1)").arg(selectedCount) : tr("删除"));
}

void DataForm::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    // 获取选中的行数（去重）
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    QList<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) {
        const int selectedRow = item->row();
        if (!selectedRows.contains(selectedRow)) {
            selectedRows.append(selectedRow);
        }
    }
    int selectedRowCount = selectedRows.size();
    updateSelectionActions();

    // 如果只选中一条，支持查看详情
    if (selectedRowCount == 1) {
        // 从 selectedRows 中获取实际选中的行，而不是用 row 参数
        int actualSelectedRow = *selectedRows.begin();
        QTableWidgetItem* idItem = ui->tableWidget->item(actualSelectedRow, 0);
        m_nCurID = idItem ? idItem->text().toInt() : -1;
        ui->Bt_Search->setEnabled(true);
    } else {
        m_nCurID = -1;
        ui->Bt_Search->setEnabled(false);
    }
}

void DataForm::on_tableWidget_cellDoubleClicked(int row, int column)
{
    on_Bt_Search_clicked();
}

void DataForm::on_Bt_Search_clicked()
{
    if (m_nCurID == -1) {
        QMessageBox::warning(this, tr("警告"), tr("请选择要查看的记录"));
        return;
    }

    // 查找选中的记录
    for (const auto& result : m_currentResults) {
        if (result.id == m_nCurID) {
            // 获取算法参数用于显示阈值范围
            InThresholdParam param = AlgorithmParamManager::instance()->getAlgParam();

            QString detail = QString(
                                 "═══════════════════════════════════════════\n"
                                 "             焊锡检测记录详情\n"
                                 "═══════════════════════════════════════════\n\n"
                                 "【基本信息】\n"
                                 "  端口编号: %1\n"
                                 "  焊锡编号: %2\n"
                                 "  批次编号: %3\n\n"
                                 "【尺寸检测】\n"
                                 "  焊锡长度: %4 mm (标准: %5~%6mm)\n"
                                 "  焊锡宽度: %7 mm (标准: %8~%9mm)\n"
                                 "  瑕疵面积: %10 mm² (标准: %11~%12mm²)\n\n"
                                 "【位置检测】\n"
                                 "  裸露铜引脚长度: %13 mm (标准: %14~%15mm)\n"
                                 "  焊锡偏移: %16 mm (标准: 0~%17mm)\n\n"
                                 "【瑕疵检测】\n"
                                 "  漏铜总面积: %18 mm² (标准: 0~%19mm²)\n"
                                 "  漏铜最大面积: %20 mm² (标准: 0~%21mm²)\n"
                                 "  黑点总面积: %22 mm² (标准: 0~%23mm²)\n"
                                 "  黑点最大面积: %24 mm² (标准: 0~%25mm²)\n\n"
                                 "【检测结果】\n"
                                 "  最终判定: %26\n"
                                 "  结果说明: %27\n\n"
                                 "【时间信息】\n"
                                 "  创建时间: %28\n"
                                 "  修改时间: %29\n\n"
                                 "【二维码】\n"
                                 "  二维码: %30\n"
                                 "═══════════════════════════════════════════")
                                 .arg(result.hxPort)
                                 .arg(result.hxId)
                                 .arg(result.hxBatch)

                                 // 尺寸检测
                                 .arg(result.hxWeight, 0, 'f', 2)
                                 .arg(param.min_w, 0, 'f', 1).arg(param.max_w, 0, 'f', 1)
                                 .arg(result.hxHeight, 0, 'f', 2)
                                 .arg(param.min_h, 0, 'f', 1).arg(param.max_h, 0, 'f', 1)
                                 .arg(result.hxArea, 0, 'f', 3)
                                 .arg(param.min_area, 0, 'f', 1).arg(param.max_area, 0, 'f', 1)

                                 // 位置检测
                                 .arg(result.hxDistance, 0, 'f', 2)
                                 .arg(param.min_TBDis, 0, 'f', 2).arg(param.max_TBDis, 0, 'f', 1)
                                 .arg(result.hxPianyi, 0, 'f', 2)
                                 .arg(param.max_PY, 0, 'f', 1)

                                 // 瑕疵检测
                                 .arg(result.hxLTSumArea, 0, 'f', 3)
                                 .arg(param.LT_ALLArea_max, 0, 'f', 2)
                                 .arg(result.hxLTMaxArea, 0, 'f', 3)
                                 .arg(param.LT_MAXArea_max, 0, 'f', 2)
                                 .arg(result.hxHDSumArea, 0, 'f', 3)
                                 .arg(param.HD_ALLArea_max, 0, 'f', 1)
                                 .arg(result.hxHDMaxArea, 0, 'f', 3)
                                 .arg(param.HD_MAXArea_max, 0, 'f', 1)

                                 // 检测结果
                                 .arg(result.hxReult ? "✅ 合格(OK)" : "❌ 不合格(NG)")
                                 .arg(result.hxReultsDescribe.isEmpty() ? "无" : result.hxReultsDescribe)

                                 // 时间信息
                                 .arg(result.createTime)
                                 .arg(result.changeTime)
                                 .arg(result.classifyStr);

            // 创建详细信息的对话框
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("焊锡检测记录详情"));
            msgBox.setText(detail);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);

            // 设置对话框大小
            msgBox.setMinimumWidth(500);

            msgBox.exec();
            break;
        }
    }
}

void DataForm::on_Bt_Export_clicked()
{
    exportToCSV();
}

void DataForm::exportToCSV()
{
    if (m_currentResults.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("没有数据可以导出！"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("导出CSV文件"),
        QDir::homePath() + "/焊锡检测数据_" + QDate::currentDate().toString("yyyyMMdd") + ".csv",
        tr("CSV文件 (*.csv);;所有文件 (*.*)")
        );

    if (fileName.isEmpty()) return;

    // 确保文件扩展名
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) {
        fileName += ".csv";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("错误"), tr("无法创建文件：\n%1").arg(file.errorString()));
        return;
    }

    // **修改：使用本地编码（GBK/GB2312）而不是UTF-8**
    // 这样可以避免Excel打开时的编码问题
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (!codec) {
        codec = QTextCodec::codecForName("GB2312");
    }
    if (!codec) {
        codec = QTextCodec::codecForLocale(); // 使用系统默认编码
    }

    QProgressDialog progress(tr("正在导出CSV..."), tr("取消"), 0, m_currentResults.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    // 写入表头
    QStringList headers;
    headers << "序号" << "治具码" << "外壳码" << "结果图片" << "端口编号" << "焊锡编号"
            << "焊锡长度(mm)" << "焊锡宽度(mm)" << "焊锡面积(mm²)"
            << "裸露铜引脚长度(mm)" << "焊锡偏移(mm)"
            << "漏铜总面积(mm²)" << "漏铜最大面积(mm²)"
            << "黑点总面积(mm²)" << "黑点最大面积(mm²)"
            << "检测结果" << "结果说明" << "修改时间";

    QString buffer;
    buffer.reserve(1024 * 1024);
    appendCsvRow(buffer, headers);

    const int flushBatchRows = 2000;
    auto flushBuffer = [&]() -> bool {
        if (buffer.isEmpty()) {
            return true;
        }
        const QByteArray encoded = codec->fromUnicode(buffer);
        const qint64 written = file.write(encoded);
        buffer.clear();
        return written == encoded.size();
    };

    // 写入数据
    for (int i = 0; i < m_currentResults.size(); ++i) {
        if (progress.wasCanceled()) {
            file.close();
            QFile::remove(fileName);
            return;
        }

        const DB_WeldingResult &result = m_currentResults[i];

        appendCsvCell(buffer, QString::number(result.id));
        appendCsvCell(buffer, result.fixtureCode);
        appendCsvCell(buffer, result.shellCode);
        appendCsvCell(buffer, result.imageCode);
        appendCsvCell(buffer, result.hxPort);
        appendCsvCell(buffer, QString::number(result.hxId));
        appendCsvCell(buffer, QString::number(result.hxWeight, 'f', 2));
        appendCsvCell(buffer, QString::number(result.hxHeight, 'f', 2));
        appendCsvCell(buffer, QString::number(result.hxArea, 'f', 3));
        appendCsvCell(buffer, QString::number(result.hxDistance, 'f', 2));
        appendCsvCell(buffer, QString::number(result.hxPianyi, 'f', 2));
        appendCsvCell(buffer, QString::number(result.hxLTSumArea, 'f', 3));
        appendCsvCell(buffer, QString::number(result.hxLTMaxArea, 'f', 3));
        appendCsvCell(buffer, QString::number(result.hxHDSumArea, 'f', 3));
        appendCsvCell(buffer, QString::number(result.hxHDMaxArea, 'f', 3));
        appendCsvCell(buffer, result.hxReult ? QStringLiteral("OK") : QStringLiteral("NG"));
        appendCsvCell(buffer, result.hxReultsDescribe);
        appendCsvCell(buffer, result.changeTime, true);
        buffer.append('\n');

        if ((i + 1) % flushBatchRows == 0) {
            if (!flushBuffer()) {
                file.close();
                QFile::remove(fileName);
                QMessageBox::critical(this, tr("错误"), tr("写入文件失败：\n%1").arg(file.errorString()));
                return;
            }
            progress.setValue(i + 1);
            QApplication::processEvents();
        }
    }

    if (!flushBuffer()) {
        file.close();
        QFile::remove(fileName);
        QMessageBox::critical(this, tr("错误"), tr("写入文件失败：\n%1").arg(file.errorString()));
        return;
    }

    progress.setValue(m_currentResults.size());
    file.flush();
    file.close();

    if (!progress.wasCanceled()) {
        // 检查文件是否成功创建
        QFileInfo fileInfo(fileName);
        if (fileInfo.size() > 0) {
            QString message = tr("CSV数据已成功导出！\n"
                                 "文件位置：%1\n\n"
                                 "文件已使用本地编码保存，可直接用Excel打开。").arg(fileName);

            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("导出成功"));
            msgBox.setText(message);

            // 添加打开选项
            QPushButton *openBtn = msgBox.addButton(tr("打开文件"), QMessageBox::ActionRole);
            QPushButton *folderBtn = msgBox.addButton(tr("打开所在文件夹"), QMessageBox::ActionRole);
            msgBox.addButton(QMessageBox::Ok);

            msgBox.exec();

            if (msgBox.clickedButton() == openBtn) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            } else if (msgBox.clickedButton() == folderBtn) {
                // 打开文件所在文件夹
                QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(fileName).absolutePath()));
            }
        } else {
            QMessageBox::critical(this, tr("错误"), tr("导出失败，文件为空！"));
        }
    }
}
