#include <QTextCodec>
#include <QApplication>
#include <InitForm/initwidget.h>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <Common/SysEnv.h>
#include "SaveLog/savelog.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MzWeldSight_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // 设置中文字体（简化版）
    QFont font;
    font.setFamily("Microsoft YaHei");  // 微软雅黑，Windows系统通用
    font.setPointSize(9);
    a.setFont(font);

    ///设置是否开启日志上下文打印比如行号、函数等
    SaveLog::Instance()->setUseContext(true);
    ///设置文件存储目录
    SaveLog::Instance()->setPath(qApp->applicationDirPath() + "/log");
    SaveLog::Instance()->deleteLocalLog();
    SaveLog::Instance()->start();
    SaveLog::Instance()->setMaxSize(1024*10);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    SysEnv::instance().setConfigInAppDir();
    InitWidget w;
    w.show();
    SysEnv::instance().save();
    return a.exec();
}
