#ifndef DATAFORM_H
#define DATAFORM_H

#include <QWidget>
#include <QScrollBar>
#include <SqliteUtil/sqliteutil.h>

namespace Ui {
class DataForm;
}

class DataForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataForm(QWidget *parent = nullptr);
    ~DataForm();

    void initWindow();
    void setWeldingDataPage(); // 设置为焊锡数据页面
signals:
    void sendBatchInfo(int batchId);

public slots:
    void OnSlideAction(int nRowIndex);
    void updateWeldingData(); // 更新焊锡数据

private slots:
    void on_Bt_PrePage_clicked();
    void on_Bt_NextPage_clicked();
    void on_Bt_Delete_clicked();
    void on_Bt_Query_clicked();
    void on_tableWidget_cellClicked(int row, int column);
    void on_tableWidget_cellDoubleClicked(int row, int column);
    void on_Bt_Search_clicked();
    void on_Bt_Export_clicked();

private:
    Ui::DataForm *ui;

    QVector<int> m_vecData;

    int m_nCurPage; //当前页数
    int m_nRowsPerPage; // 每页显示的行数
    int m_nTotalPages;  // 表格总页数

    int m_nCurID; // 当前选中的记录ID
    bool m_bDB;

    QList<DB_WeldingResult> m_currentResults; // 当前查询结果

    SqliteUtil m_sql;
private:
    void exportToCSV();

};

#endif // DATAFORM_H
