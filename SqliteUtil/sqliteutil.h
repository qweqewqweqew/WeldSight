#ifndef SQLITEUTIL_H
#define SQLITEUTIL_H
#include <QDebug>
#include <QtSql/qsql.h>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QList>
#include <QObject>
#include <QTime>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QVector>
#include <QtMath>
#include <Common/common.h>

class SqliteUtil
{
public:
    SqliteUtil();
    ~SqliteUtil();

    void init(QString db);

    int excute_sql_file(QString sql_file_path);

    bool createConnection(QString dbname);

    bool disConnection();

    bool createTabel(QString fileName);

    bool isExistTab(QString tbname);

    void strToSql(QString& str);

    void deleteSingleData(int id, QString tbname);

    void deleteDefectschemes(int id);

    // 焊锡检测结果操作方法
    bool insertWeldingResult(const DB_WeldingResult& result);
    bool deleteWeldingResult(int id);
    bool deleteWeldingResults(const QList<int>& ids);  // 批量删除
    QList<DB_WeldingResult> queryWeldingResults(int batchId = -1,
                                                const QDateTime& startTime = QDateTime(),
                                                const QDateTime& endTime = QDateTime(),
                                                int port = -1);
    QList<DB_WeldingResult> queryWeldingResultsByTimeRange(const QDateTime& startTime,
                                                           const QDateTime& endTime);
private:
    QFile file;
    int m_nRow;
    int m_nRowMax;


    QString m_strDbPath;
    QString m_strSqlPath;
    QString m_DB;

    bool m_bDiscontinue;
    QString m_sTbname;
};

#endif // SQLITEUTIL_H
