#include "capsenseinterface.h"
#include "capsensetriform.h"
#include <QApplication>
#include <InitForm/initwidget.h>

CapSenseInterface::CapSenseInterface(QObject *parent)
    : QObject(parent)
{}

void CapSenseInterface::setDisplayTitle(int displayIndex, const QString &title)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->setTitle(title);
        }
    }
}

void CapSenseInterface::setDisplayImage(int displayIndex, int imgIndex,const QImage &image)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->showImg(imgIndex,image);
        }
    }
}

void CapSenseInterface::setTableData(int displayIndex,
                                     const QStringList &columnHeaders,
                                     const QVariantList &rowData)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->setTableData(columnHeaders, rowData);
        }
    }
}

void CapSenseInterface::clearTableData(int displayIndex)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearTable();
        }
    }
}

void CapSenseInterface::updateStatistics(int group,
                                         const QList<QPair<QString, QVariant>> &data)
{
    if(auto form = findTriForm()) {
        form->updateDataGroup(
            static_cast<CapSenseTriForm::DataGroup>(group),
            data
            );
    }
}

// 列颜色设置
void CapSenseInterface::setColumnTextColor(int displayIndex, int column, const QColor &color)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->setColumnTextColor(column, color);
        }
    }
}

void CapSenseInterface::clearColumnTextColor(int displayIndex, int column)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearColumnTextColor(column);
        }
    }
}

// 行颜色设置
void CapSenseInterface::setRowTextColor(int displayIndex, int row, const QColor &color)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->setRowTextColor(row, color);
        }
    }
}

void CapSenseInterface::clearRowTextColor(int displayIndex, int row)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearRowTextColor(row);
        }
    }
}

void CapSenseInterface::clearAllRowTextColors(int displayIndex)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearAllRowTextColors();
        }
    }
}

// 单元格颜色设置
void CapSenseInterface::setCellTextColor(int displayIndex, int row, int column, const QColor &color)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->setCellTextColor(row, column, color);
        }
    }
}

void CapSenseInterface::clearCellTextColor(int displayIndex, int row, int column)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearCellTextColor(row, column);
        }
    }
}

void CapSenseInterface::clearAllCellTextColors(int displayIndex)
{
    if(auto form = findTriForm()) {
        if(auto display = form->getDisplay(displayIndex)) {
            display->clearAllCellTextColors();
        }
    }
}

CapSenseTriForm* CapSenseInterface::findTriForm()
{
    InitWidget* init = nullptr;
    for (QWidget* w : QApplication::topLevelWidgets())
    {
        init = qobject_cast<InitWidget*>(w);
        if (init) break;
    }
    if (!init) return nullptr;

    return init->findChild<CapSenseTriForm*>();
}
