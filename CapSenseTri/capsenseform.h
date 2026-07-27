// capsenseform.h
#ifndef CAPSENSEFORM_H
#define CAPSENSEFORM_H

#include <QWidget>
#include <QMap>
#include <QColor>
#include <QPair>

namespace Ui {
class CapSenseForm;
}

class CapSenseForm : public QWidget
{
    Q_OBJECT

public:
    explicit CapSenseForm(QWidget *parent = nullptr);
    ~CapSenseForm();

    void initWindow();
    void loadQssFile();

    void setTitle(const QString& title);
    void showImg(int index,const QImage& img);

    void setTableData(const QStringList &columnHeaders = QStringList(),
                      const QVariantList &rowData = QVariantList());

    void clearTable();

    // 列颜色设置
    void setColumnTextColor(int column, const QColor &color);
    void clearColumnTextColor(int column);

    // 行颜色设置
    void setRowTextColor(int row, const QColor &color);
    void clearRowTextColor(int row);
    void clearAllRowTextColors();

    // 单元格颜色设置
    void setCellTextColor(int row, int column, const QColor &color);
    void clearCellTextColor(int row, int column);
    void clearAllCellTextColors();

private:
    Ui::CapSenseForm *ui;
    QMap<int, QColor> m_columnColors; // 存储列颜色设置
    QMap<int, QColor> m_rowColors;    // 存储行颜色设置
    QMap<QPair<int, int>, QColor> m_cellColors; // 存储单元格颜色设置
};

#endif // CAPSENSEFORM_H
