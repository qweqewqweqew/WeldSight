#include "capsenseform.h"
#include "ui_capsenseform.h"

#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QTextStream>
#include <algorithm>

namespace {

QString headerTextWithoutDetail(const QString& text)
{
    const int asciiParen = text.indexOf('(');
    const int fullParen = text.indexOf(QChar(0xFF08));
    int cut = -1;
    if (asciiParen >= 0 && fullParen >= 0) {
        cut = (std::min)(asciiParen, fullParen);
    } else {
        cut = (std::max)(asciiParen, fullParen);
    }
    return (cut >= 0 ? text.left(cut) : text).trimmed();
}

QString compactHeaderText(const QString& fullText)
{
    const QString baseText = headerTextWithoutDetail(fullText);
    if (baseText.size() <= 3) {
        return baseText;
    }
    return baseText.left(2) + QStringLiteral("...");
}

void applyCompactHeaders(QTableWidget* table, const QStringList& headers)
{
    if (table->columnCount() == headers.size()) {
        bool unchanged = true;
        for (int col = 0; col < headers.size(); ++col) {
            const QTableWidgetItem* item = table->horizontalHeaderItem(col);
            if (!item || item->data(Qt::UserRole).toString() != headers[col]) {
                unchanged = false;
                break;
            }
        }
        if (unchanged) {
            return;
        }
    }

    table->setColumnCount(headers.size());
    for (int col = 0; col < headers.size(); ++col) {
        QTableWidgetItem* item = new QTableWidgetItem(compactHeaderText(headers[col]));
        item->setTextAlignment(Qt::AlignCenter);
        if (col != 0 && col != headers.size() - 1) {
            item->setToolTip(headers[col]);
        }
        item->setData(Qt::UserRole, headers[col]);
        table->setHorizontalHeaderItem(col, item);
    }
}

void applyCompactRows(QTableWidget* table)
{
    const int rowCount = table->rowCount();
    const int rowHeight = rowCount >= 10 ? 20 : 22;
    table->verticalHeader()->setDefaultSectionSize(rowHeight);
    table->verticalHeader()->setMinimumSectionSize(18);
    for (int row = 0; row < rowCount; ++row) {
        table->setRowHeight(row, rowHeight);
    }
}

}

CapSenseForm::CapSenseForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CapSenseForm)
{
    ui->setupUi(this);
    initWindow();
}

CapSenseForm::~CapSenseForm()
{
    delete ui;
}

void CapSenseForm::initWindow()
{
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setWordWrap(false);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(false);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidget->horizontalHeader()->setMinimumSectionSize(24);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->hide();
    ui->verticalLayout->setStretch(0, 4);
    ui->verticalLayout->setStretch(1, 3);

    QStringList header;
    header << tr("HXID")
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

    applyCompactHeaders(ui->tableWidget, header);
    applyCompactRows(ui->tableWidget);

    /* 关键：让列宽根据内容自适应，并允许用户拖动 */
    /* 如果想再紧一点，可以限制最小宽度 */

    ui->tableWidget->setAlternatingRowColors(true);
}

void CapSenseForm::setTitle(const QString &title)
{
    // ui->text_title->setText(title);
}

void CapSenseForm::showImg(int index,const QImage &img)
{
    QPixmap pixmap = QPixmap::fromImage(img);
    ImageZoomView* pImg;
    if(index == 1){
        pImg = ui->img_view_1;
    }else{
        pImg = ui->img_view_1;
    }

    if (!pixmap.isNull()) {
        pImg->setPixmap(pixmap.scaled(ui->img_view_1->size(),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation));
    }
}

void CapSenseForm::setTableData(const QStringList &columnHeaders, const QVariantList &rowData)
{
    QTableWidget *table = ui->tableWidget;

    if(!columnHeaders.isEmpty()) {
        applyCompactHeaders(table, columnHeaders);
    }

    table->setRowCount(0);

    for(const QVariant &rowVariant : rowData) {
        QVariantList row = rowVariant.toList();
        if(row.size() != table->columnCount()) continue;

        int rowIndex = table->rowCount();
        table->insertRow(rowIndex);

        for(int col = 0; col < row.size(); ++col) {
            const QString cellText = row[col].toString();
            QTableWidgetItem *item = new QTableWidgetItem(cellText);
            item->setTextAlignment(Qt::AlignCenter);
            if (col != 0 && col != row.size() - 1) {
                item->setToolTip(cellText);
            }

            // 应用颜色设置（优先级：单元格 > 行 > 列）
            QPair<int, int> cellKey(rowIndex, col);
            if (m_cellColors.contains(cellKey)) {
                item->setForeground(m_cellColors[cellKey]);
            } else if (m_rowColors.contains(rowIndex)) {
                item->setForeground(m_rowColors[rowIndex]);
            } else if (m_columnColors.contains(col)) {
                item->setForeground(m_columnColors[col]);
            }

            table->setItem(rowIndex, col, item);
        }
    }

    applyCompactRows(table);
}

void CapSenseForm::clearTable()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

// 列颜色设置
void CapSenseForm::setColumnTextColor(int column, const QColor &color)
{
    if (column >= 0 && column < ui->tableWidget->columnCount()) {
        m_columnColors[column] = color;

        // 立即更新现有数据的颜色
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            if (auto item = ui->tableWidget->item(row, column)) {
                // 只有该单元格没有被单独设置颜色时才应用列颜色
                QPair<int, int> cellKey(row, column);
                if (!m_cellColors.contains(cellKey) && !m_rowColors.contains(row)) {
                    item->setForeground(color);
                }
            }
        }
    }
}

void CapSenseForm::clearColumnTextColor(int column)
{
    if (m_columnColors.remove(column)) {
        // 恢复默认颜色
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            if (auto item = ui->tableWidget->item(row, column)) {
                // 检查是否有行颜色或单元格颜色
                QPair<int, int> cellKey(row, column);
                if (m_cellColors.contains(cellKey)) {
                    item->setForeground(m_cellColors[cellKey]);
                } else if (m_rowColors.contains(row)) {
                    item->setForeground(m_rowColors[row]);
                } else {
                    item->setForeground(QBrush()); // 清除设置的颜色
                }
            }
        }
    }
}

// 行颜色设置
void CapSenseForm::setRowTextColor(int row, const QColor &color)
{
    if (row >= 0 && row < ui->tableWidget->rowCount()) {
        m_rowColors[row] = color;

        // 立即更新现有数据的颜色
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            if (auto item = ui->tableWidget->item(row, col)) {
                // 只有该单元格没有被单独设置颜色时才应用行颜色
                QPair<int, int> cellKey(row, col);
                if (!m_cellColors.contains(cellKey)) {
                    item->setForeground(color);
                }
            }
        }
    }
}

void CapSenseForm::clearRowTextColor(int row)
{
    if (m_rowColors.remove(row)) {
        // 恢复默认颜色或列颜色
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            if (auto item = ui->tableWidget->item(row, col)) {
                QPair<int, int> cellKey(row, col);
                if (m_cellColors.contains(cellKey)) {
                    item->setForeground(m_cellColors[cellKey]);
                } else if (m_columnColors.contains(col)) {
                    item->setForeground(m_columnColors[col]);
                } else {
                    item->setForeground(QBrush()); // 清除设置的颜色
                }
            }
        }
    }
}

void CapSenseForm::clearAllRowTextColors()
{
    m_rowColors.clear();
    // 更新所有行的颜色
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            if (auto item = ui->tableWidget->item(row, col)) {
                QPair<int, int> cellKey(row, col);
                if (m_cellColors.contains(cellKey)) {
                    item->setForeground(m_cellColors[cellKey]);
                } else if (m_columnColors.contains(col)) {
                    item->setForeground(m_columnColors[col]);
                } else {
                    item->setForeground(QBrush()); // 清除设置的颜色
                }
            }
        }
    }
}

// 单元格颜色设置
void CapSenseForm::setCellTextColor(int row, int column, const QColor &color)
{
    if (row >= 0 && row < ui->tableWidget->rowCount() &&
        column >= 0 && column < ui->tableWidget->columnCount()) {
        QPair<int, int> cellKey(row, column);
        m_cellColors[cellKey] = color;

        // 立即更新该单元格的颜色
        if (auto item = ui->tableWidget->item(row, column)) {
            item->setForeground(color);
        }
    }
}

void CapSenseForm::clearCellTextColor(int row, int column)
{
    QPair<int, int> cellKey(row, column);
    if (m_cellColors.remove(cellKey)) {
        // 恢复默认颜色、行颜色或列颜色
        if (auto item = ui->tableWidget->item(row, column)) {
            if (m_rowColors.contains(row)) {
                item->setForeground(m_rowColors[row]);
            } else if (m_columnColors.contains(column)) {
                item->setForeground(m_columnColors[column]);
            } else {
                item->setForeground(QBrush()); // 清除设置的颜色
            }
        }
    }
}

void CapSenseForm::clearAllCellTextColors()
{
    m_cellColors.clear();
    // 更新所有单元格的颜色
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
            if (auto item = ui->tableWidget->item(row, col)) {
                if (m_rowColors.contains(row)) {
                    item->setForeground(m_rowColors[row]);
                } else if (m_columnColors.contains(col)) {
                    item->setForeground(m_columnColors[col]);
                } else {
                    item->setForeground(QBrush()); // 清除设置的颜色
                }
            }
        }
    }
}
