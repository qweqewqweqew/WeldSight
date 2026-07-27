#ifndef CAPSENSEINTERFACE_H
#define CAPSENSEINTERFACE_H

#include "capsensetriform.h"
#include <QObject>
#include <QImage>
#include <QStringList>
#include <QVariantList>

class CapSenseInterface : public QObject
{
    Q_OBJECT
public:
    explicit CapSenseInterface(QObject *parent = nullptr);

    // 设置指定显示单元的标题
    Q_INVOKABLE void setDisplayTitle(int displayIndex, const QString &title);

    // 设置指定显示单元的图像
    Q_INVOKABLE void setDisplayImage(int displayIndex, int imgIndex, const QImage &image);

    // 设置表格数据
    Q_INVOKABLE void setTableData(int displayIndex,
                                  const QStringList &columnHeaders = QStringList(),
                                  const QVariantList &rowData = QVariantList());

    // 清除表格数据
    Q_INVOKABLE void clearTableData(int displayIndex);

    // 更新统计区域数据
    Q_INVOKABLE void updateStatistics(int group,
                                      const QList<QPair<QString, QVariant>> &data);

    // 列颜色设置
    Q_INVOKABLE void setColumnTextColor(int displayIndex, int column, const QColor &color);
    Q_INVOKABLE void clearColumnTextColor(int displayIndex, int column);

    // 行颜色设置
    Q_INVOKABLE void setRowTextColor(int displayIndex, int row, const QColor &color);
    Q_INVOKABLE void clearRowTextColor(int displayIndex, int row);
    Q_INVOKABLE void clearAllRowTextColors(int displayIndex);

    // 单元格颜色设置
    Q_INVOKABLE void setCellTextColor(int displayIndex, int row, int column, const QColor &color);
    Q_INVOKABLE void clearCellTextColor(int displayIndex, int row, int column);
    Q_INVOKABLE void clearAllCellTextColors(int displayIndex);

private:
    CapSenseTriForm* findTriForm();
};

#endif // CAPSENSEINTERFACE_H
