#include "sqliteutil.h"
#include <QProgressDialog>
#include <QApplication>
#include <QDate>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>


SqliteUtil::SqliteUtil()
{
    m_nRow = 0;
    m_nRowMax = 100;
    m_bDiscontinue = false;
}

SqliteUtil::~SqliteUtil()
{
    //关闭数据库
    disConnection();
    return;
}

void SqliteUtil::init(QString db)
{
    QSqlDatabase _sql;
    if(QSqlDatabase::contains(db)){
        _sql = QSqlDatabase::database(db);
        return;
    }

    m_DB = db;
    m_strDbPath = QDir::currentPath();
    m_strSqlPath = m_strDbPath;
    m_strDbPath += "/data/info.db";

    QFile fp;
    fp.setFileName(m_strDbPath);
    if (fp.exists())
    {
        createConnection(m_strDbPath);
    }
    else
    {
        fp.open(QIODevice::ReadWrite);
        fp.close();
        createConnection(m_strDbPath);
        excute_sql_file(m_strSqlPath);
    }

    // 兼容旧数据库：添加 classifyStr 列
    {
        QSqlDatabase db = QSqlDatabase::database(m_DB);
        QSqlQuery q(db);
        q.exec("DROP TRIGGER IF EXISTS auto_clean_welding_results");
        q.exec("ALTER TABLE welding_results ADD COLUMN classifyStr TEXT DEFAULT ''");
        q.exec("ALTER TABLE welding_results ADD COLUMN fixtureCode TEXT DEFAULT ''");
        q.exec("ALTER TABLE welding_results ADD COLUMN shellCode TEXT DEFAULT ''");
        q.exec("ALTER TABLE welding_results ADD COLUMN imageCode TEXT DEFAULT ''");
    }
}

bool SqliteUtil::createConnection(QString dbname)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_DB);
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qInfo() << "drivers  Error!Unable to load databas!This App needs the SQLITE driver";
    db.setDatabaseName(dbname);
    QString strname = db.connectionName();
    if (!db.open()) {
        qInfo() << "open db Error!" << db.lastError();
        return false;
    }
    else {
        return true;
    }
}

int SqliteUtil::excute_sql_file(QString sql_file_path)
{
    int iRet = 0;

    sql_file_path = m_strSqlPath + "/data/welding_results.sql";
    QFile qfile;
    qfile.setFileName(sql_file_path);
    if (!qfile.exists()) {
        return -1;
    }
    if (!qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }
    QTextStream in(&qfile);
    in.setCodec("UTF-8");
    QString qstr_file_data = in.readAll();
    QStringList qstrlist_sql = qstr_file_data.split(";");

    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery qsql(db);
    // 创建表

    for (int i = 0; i < qstrlist_sql.size() - 1; i++) {
        QString qstr_sql_part = qstrlist_sql.at(i).toUtf8();
        bool sql_result = qsql.exec(qstr_sql_part);
        if (!sql_result) {
            QSqlError sql_error = qsql.lastError();
            qInfo()<< sql_error;
            iRet = -1;
            break;
        }
    }

    // // 创建触发器
    // QStringList qstrTrigger_sql;
    // qstrTrigger_sql.append(m_strSqlPath + "/data/change_TRIGGER.sql");
    // qstrTrigger_sql.append(m_strSqlPath + "/data/default_TRIGGER.sql");

    // for (int i = 0; i < qstrTrigger_sql.size(); i++) {
    //     QString qstr_Trigger = qstrTrigger_sql.at(i).toUtf8();
    //     qfile.setFileName(qstr_Trigger);
    //     QTextStream in(&qfile);
    //     if (!qfile.exists()) {
    //         return -1;
    //     }
    //     if (!qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //         return -1;
    //     }
    //     in.setCodec("UTF-8");
    //     QString qstr_data = in.readAll();
    //     bool sql_result = qsql.exec(qstr_data);
    //     if (!sql_result) {
    //         QSqlError sql_error = qsql.lastError();
    //         qInfo() << sql_error;
    //         iRet = -1;
    //         break;
    //     }
    // }


    return iRet;
}

bool SqliteUtil::disConnection()
{
    {
        QSqlDatabase db = QSqlDatabase::database(m_DB);
        QSqlQuery query("SELECT NAME, DOB FROM EMPLOYEES", db);
    }
    // Both "db" and "query" are destroyed because they are out of scope
    QSqlDatabase::removeDatabase(m_DB); // correct
    return true;
}

bool SqliteUtil::createTabel(QString fileName)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);
    QString tbname = QFileInfo(fileName).fileName();
    QStringList list = tbname.split(".");
    m_sTbname = list[0];
    strToSql(m_sTbname);
    file.setFileName(fileName);
    QString create_sql = QString("create table %1 (%2 int)").arg(m_sTbname).arg("js");
    if (!sql_query.exec(create_sql))
    {
        qInfo() << "Error: Fail to create table." << sql_query.lastError();
        return false;
    }
    else
    {
        qInfo() << "Table created!";
        return true;
    }
}

bool SqliteUtil::isExistTab(QString tbname)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);

    QString fname = QFileInfo(tbname).fileName();
    QStringList list = fname.split(".");
    m_sTbname = list[0];
    strToSql(m_sTbname);
    QString strSql = QString("select count(*) from sqlite_master where name='%1'").arg(m_sTbname);

    if (!sql_query.exec(strSql))
    {
        return false;
    }
    else
    {
        while (sql_query.next())
        {
            int id = sql_query.value(0).toInt();
            if (id)
            {
                return true;
            }
            else {
                return false;
            }
        }
        return false;
    }
}


void SqliteUtil::strToSql(QString& str)
{
    str = str.replace(" ", "");
    str = str.replace("-", "0");
    str = str.replace("+", "0");
}

void SqliteUtil::deleteSingleData(int id, QString tbname)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);
    QString strSql = QString("DELETE FROM %1 WHERE id = %2").arg(tbname).arg(id);
    if (!sql_query.exec(strSql))
    {
        qWarning() << sql_query.lastError();
    }
    else
    {
        qInfo() << "DELETE tbname deleteSingleData success";
    }
}

void SqliteUtil::deleteDefectschemes(int id)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);
    sql_query.exec("PRAGMA foreign_keys = ON;");
    if (!sql_query.exec()) {
        qDebug() << "Failed to enable foreign keys:" << sql_query.lastError().text();
    }
    QString strSql = QString("DELETE FROM defect_schemes WHERE scheme_id = %1").arg(id);
    if (!sql_query.exec(strSql))
    {
        qWarning() << sql_query.lastError();
    }
    else
    {
        qInfo() << "DELETE tbname deleteSingleData success";
    }
}

// 插入焊锡检测结果
bool SqliteUtil::insertWeldingResult(const DB_WeldingResult& result)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);

    sql_query.prepare(
        "INSERT INTO welding_results (hxPort, hxId, hxBatch, hxWeight, hxHeight, hxArea, "
        "hxDistance, hxPianyi, hxLTSumArea, hxLTMaxArea, hxHDSumArea, hxHDMaxArea, "
        "classifyStr, fixtureCode, shellCode, imageCode, hxReult, hxReultsDescribe) "
        "VALUES (:hxPort, :hxId, :hxBatch, :hxWeight, :hxHeight, :hxArea, "
        ":hxDistance, :hxPianyi, :hxLTSumArea, :hxLTMaxArea, :hxHDSumArea, :hxHDMaxArea, "
        ":classifyStr, :fixtureCode, :shellCode, :imageCode, :hxReult, :hxReultsDescribe)");
    sql_query.bindValue(":hxPort", result.hxPort);
    sql_query.bindValue(":hxId", result.hxId);
    sql_query.bindValue(":hxBatch", result.hxBatch);
    sql_query.bindValue(":hxWeight", result.hxWeight);
    sql_query.bindValue(":hxHeight", result.hxHeight);
    sql_query.bindValue(":hxArea", result.hxArea);
    sql_query.bindValue(":hxDistance", result.hxDistance);
    sql_query.bindValue(":hxPianyi", result.hxPianyi);
    sql_query.bindValue(":hxLTSumArea", result.hxLTSumArea);
    sql_query.bindValue(":hxLTMaxArea", result.hxLTMaxArea);
    sql_query.bindValue(":hxHDSumArea", result.hxHDSumArea);
    sql_query.bindValue(":hxHDMaxArea", result.hxHDMaxArea);
    sql_query.bindValue(":classifyStr", result.classifyStr);
    sql_query.bindValue(":fixtureCode", result.fixtureCode);
    sql_query.bindValue(":shellCode", result.shellCode);
    sql_query.bindValue(":imageCode", result.imageCode);
    sql_query.bindValue(":hxReult", result.hxReult ? 1 : 0);
    sql_query.bindValue(":hxReultsDescribe", result.hxReultsDescribe);

    if (!sql_query.exec()) {
        qWarning() << "Insert welding result failed:" << sql_query.lastError();
        return false;
    }
    return true;
}

// 删除焊锡检测结果
bool SqliteUtil::deleteWeldingResult(int id)
{
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);

    QString strSql = QString("DELETE FROM welding_results WHERE id = %1").arg(id);
    if (!sql_query.exec(strSql)) {
        qWarning() << "Delete welding result failed:" << sql_query.lastError();
        return false;
    }
    return true;
}

// 批量删除焊锡检测结果
bool SqliteUtil::deleteWeldingResults(const QList<int>& ids)
{
    if (ids.isEmpty()) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);

    // 构建 IN 子句：DELETE FROM welding_results WHERE id IN (1,2,3,...)
    QStringList idStrList;
    for (int id : ids) {
        idStrList << QString::number(id);
    }
    QString strSql = QString("DELETE FROM welding_results WHERE id IN (%1)")
                        .arg(idStrList.join(","));

    if (!sql_query.exec(strSql)) {
        qWarning() << "Batch delete welding results failed:" << sql_query.lastError();
        return false;
    }

    qInfo() << "Batch delete welding results success, count:" << ids.size();
    return true;
}

// 查询焊锡检测结果
QList<DB_WeldingResult> SqliteUtil::queryWeldingResults(int batchId,
                                                        const QDateTime& startTime,
                                                        const QDateTime& endTime,
                                                        int port)
{
    QList<DB_WeldingResult> results;
    QSqlDatabase db = QSqlDatabase::database(m_DB);
    QSqlQuery sql_query(db);

    QString strSql = "SELECT id, hxPort, hxId, hxBatch, hxWeight, hxHeight, hxArea, "
                     "hxDistance, hxPianyi, hxLTSumArea, hxLTMaxArea, hxHDSumArea, "
                     "hxHDMaxArea, classifyStr, fixtureCode, shellCode, imageCode, "
                     "hxReult, hxReultsDescribe, create_time, change_time "
                     "FROM welding_results WHERE 1=1";

    // 添加查询条件
    if (batchId != -1) {
        strSql += QString(" AND hxBatch = %1").arg(batchId);
    }
    if (port != -1) {
        strSql += QString(" AND hxPort = %1").arg(port);
    }
    if (startTime.isValid()) {
        strSql += QString(" AND create_time >= '%1'").arg(startTime.toString("yyyy-MM-dd hh:mm:ss"));
    }
    if (endTime.isValid()) {
        strSql += QString(" AND create_time <= '%1'").arg(endTime.toString("yyyy-MM-dd hh:mm:ss"));
    }

    // 按创建时间降序排列，新的在前面
    strSql += " ORDER BY create_time DESC";

    if (!sql_query.exec(strSql)) {
        qWarning() << "Query welding results failed:" << sql_query.lastError();
        return results;
    }

    while (sql_query.next()) {
        DB_WeldingResult result;
        result.id = sql_query.value("id").toInt();
        result.hxPort = sql_query.value("hxPort").toString();
        result.hxId = sql_query.value("hxId").toInt();
        result.hxBatch = sql_query.value("hxBatch").toInt();
        result.hxWeight = sql_query.value("hxWeight").toFloat();
        result.hxHeight = sql_query.value("hxHeight").toFloat();
        result.hxArea = sql_query.value("hxArea").toDouble();
        result.hxDistance = sql_query.value("hxDistance").toFloat();
        result.hxPianyi = sql_query.value("hxPianyi").toFloat();
        result.hxLTSumArea = sql_query.value("hxLTSumArea").toDouble();
        result.hxLTMaxArea = sql_query.value("hxLTMaxArea").toDouble();
        result.hxHDSumArea = sql_query.value("hxHDSumArea").toDouble();
        result.hxHDMaxArea = sql_query.value("hxHDMaxArea").toDouble();
        result.classifyStr = sql_query.value("classifyStr").toString();
        result.fixtureCode = sql_query.value("fixtureCode").toString();
        result.shellCode = sql_query.value("shellCode").toString();
        result.imageCode = sql_query.value("imageCode").toString();
        result.hxReult = sql_query.value("hxReult").toBool();
        result.hxReultsDescribe = sql_query.value("hxReultsDescribe").toString();
        result.createTime = sql_query.value("create_time").toString();
        result.changeTime = sql_query.value("change_time").toString();

        results.append(result);
    }

    return results;
}

// 按时间范围查询
QList<DB_WeldingResult> SqliteUtil::queryWeldingResultsByTimeRange(const QDateTime& startTime,
                                                                   const QDateTime& endTime)
{
    return queryWeldingResults(-1, startTime, endTime, -1);
}
