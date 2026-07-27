CapSenseTri 是一个基于Qt框架开发的图形用户界面应用程序，专门用于展示电容感应测试结果。该系统提供多显示单元布局，支持图像展示、表格数据呈现和统计信息可视化。



功能特点

多显示单元布局



每个单元可独立展示图像和表格数据



图像展示功能



支持缩放、拖动查看图像细节



支持绘制和清除ROI（感兴趣区域）



双击重置视图



右键菜单提供快速操作



表格数据显示



自定义表头和行数据



支持设置列文本颜色



交替行颜色提高可读性



统计信息面板



配置信息区



良品率统计区



不良品统计区



总计信息区



编程接口



提供完整的API接口用于外部控制



支持动态更新所有显示内容



文件结构

text

CapSenseTri/

├── capsenseform.h/cpp         # 单个显示单元实现

├── capsensetriform.h/cpp      # 主窗口和显示管理

├── capsenseinterface.h/cpp    # 外部控制接口

├── imagezoomview.h/cpp        # 图像缩放和ROI功能

├── CapSenseTri.pri            # 项目包含文件

└── CapRes.qrc                 # 资源文件

使用示例

cpp

// 初始化接口

CapSenseInterface interface;



// 设置第一个显示单元的标题

interface.setDisplayTitle(0, "侧面检测结果");



// 设置图像

QImage image("path/to/image.jpg");

interface.setDisplayImage(0, image);



// 设置表格数据

QStringList headers = {"项目", "值", "最小值", "最大值", "缺陷"};

QVariantList data = {

    QVariantList{"项目A", "0.00", "0.87", "2.30", "4"},

    QVariantList{"项目B", "0.14", "0.87", "2.30", "4"},

    QVariantList{"项目C", "50.00", "0.87", "2.30", "4"}

};

interface.setTableData(0, headers, data);



// 更新统计数据

QList<QPair<QString, QVariant>> stats = {

    {"总检测:", 150},

    {"不良数:", 5},

    {"良品率:", "96.7%"}

};

interface.updateStatistics(CapSenseTriForm::TotalData, stats);



// 设置列文本颜色

interface.setColumnTextColor(0, 4, Qt::red); // 将缺陷列设为红色

界面操作

图像查看



鼠标滚轮：缩放图像



鼠标拖动：平移图像



双击：重置视图



右键菜单：提供"恢复"和"清除ROI"选项



ROI操作



调用startROIDrawing()进入绘制模式



鼠标拖动画出矩形区域



getROI()获取当前ROI区域



clearROI()清除ROI



编译说明

确保安装Qt 5.x或更高版本



在.pro文件中包含CapSenseTri.pri



使用qmake生成Makefile



编译并运行



依赖项

Qt Core



Qt GUI



Qt Widgets



注意事项

图像缩放最大倍数为8倍（可通过setMaxScaleFactor()调整）



显示单元索引从0开始（0,1,2）



表格数据列索引从0开始



该系统为电容感应测试提供了直观的结果展示界面，特别适合需要同时查看多个测试点结果的工业检测场景。

