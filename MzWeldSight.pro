QT       += core gui
QT += concurrent
QT += network
QT += serialport
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
RC_ICONS = "icon/app.ico"
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Alg/Welding.cpp \
    Alg/imageconvert.cpp \
    Alg/solderjointalgorithmcore.cpp \
    Alg/weldingalgorithmmanager.cpp \
    Alg/weldinginitializationworker.cpp \
    CameraApi/cameracontroller.cpp \
    Common/sysenv.cpp \
    Common/ScannerHook.cpp\
    Form/algconfigform.cpp \
    Form/algorithmparammanager.cpp \
    Form/dataform.cpp \
    Form/ioactionform.cpp \
    Form/saveform.cpp \
    SqliteUtil/sqliteutil.cpp \
    Thread/camerathread.cpp \
    Thread/cameraworker.cpp \
    Thread/imagenamer.cpp \
    Thread/imagesavemanager.cpp \
    main.cpp

HEADERS += \
    Alg/Api_Welding.h \
    Alg/imageconvert.h \
    Alg/solderjointalgorithmcore.h \
    Alg/weldingalgorithmmanager.h \
    Alg/weldinginitializationworker.h \
    CameraApi/cameracontroller.h \
    Common/common.h \
    Common/cvqt_text.hpp \
    Common/sysenv.h \
    Common/ScannerHook.h\
    Form/algconfigform.h \
    Form/algorithmparammanager.h \
    Form/dataform.h \
    Form/ioactionform.h \
    Form/saveform.h \
    SqliteUtil/sqliteutil.h \
    Thread/camerathread.h \
    Thread/cameraworker.h \
    Thread/imagenamer.h \
    Thread/imagesavemanager.h

TRANSLATIONS += \
    MzWeldSight_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


include($$PWD/CapSenseTri/CapSenseTri.pri)
include($$PWD/InitForm/initForm.pri)
include($$PWD/algorithmForm/algorithmForm.pri)
include($$PWD/SaveLog/savelog.pri)

# 支持中文编码
CODECFORTR = UTF-8
CODECFORSRC = UTF-8

# 设置字符集
DEFINES += QT_DEPRECATED_WARNINGS \
           SOURCE_CHARSET=UTF-8 \
           EXECUTION_CHARSET=UTF-8

# 如果使用MSVC编译器，添加以下配置
win32-msvc* {
    QMAKE_CXXFLAGS += /utf-8 /Zm500 /FS
    QMAKE_CFLAGS += /utf-8
}


#--------------------------------------------VC3000HDLL-------------------------------------------
win32 {
    VC3000HDLL_PATH = $$PWD/VC3000HDLL
    INCLUDEPATH += $$VC3000HDLL_PATH/include
    LIBS += User32.lib
}
#--------------------------------------------Alg-------------------------------------------
win32 {
    OPENCV_PATH = $$PWD/lib3rd
    INCLUDEPATH += $$OPENCV_PATH/opencv4.9/include
    INCLUDEPATH += $$OPENCV_PATH/openvino_toolkit_2024/runtime/include
    INCLUDEPATH += $$OPENCV_PATH/tbb/include
    INCLUDEPATH += $$OPENCV_PATH/ovdetector/include

    CONFIG(debug, debug|release) {
        LIBS += -L$$OPENCV_PATH/opencv4.9/lib \
                -lopencv_world490d
    } else {
        LIBS += -L$$OPENCV_PATH/opencv4.9/lib \
                -lopencv_world490
        LIBS += -L$$OPENCV_PATH/openvino_toolkit_2024/runtime/lib/intel64/Release \
                -lopenvino_onnx_frontend
        LIBS += -L$$OPENCV_PATH/openvino_toolkit_2024/runtime/lib/intel64/Release \
                -lopenvino
        LIBS += -L$$OPENCV_PATH/openvino_toolkit_2024/runtime/lib/intel64/Release \
                -lopenvino_c
        LIBS += -L$$OPENCV_PATH/tbb/lib \
                -ltbb12
        LIBS += -L$$OPENCV_PATH/ovdetector/release \
                -lOVDetector
    }
}


#--------------------------------------------Da Hua-------------------------------------------
INCLUDEPATH += $$PWD/DHua/Include

contains(QMAKE_COMPILER_DEFINES, _WIN64) {
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/DHua/Depends/x64/vs2013shared -lMVSDKmdd

    } else {
        LIBS += -L$$PWD/DHua/Depends/x64/vs2013shared -lMVSDKmd

    }
} else {
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/DHua/Depends/win32/vs2013shared -lMVSDKmdd

    } else {
        LIBS += -L$$PWD/DHua/Depends/win32/vs2013shared -lMVSDKmd
    }
}

#--------------------------------------------Hai Kang-------------------------------------------


CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/Bin/x64/Debug
} else {
    DESTDIR = $$PWD/Bin/x64/Release
}

win32 {
    VC3000_RUNTIME = $$PWD/VC3000HDLL/bin
    QMAKE_POST_LINK += $$quote(cmd /c "if not exist $$shell_path($$DESTDIR) mkdir $$shell_path($$DESTDIR) & copy /Y $$shell_path($$VC3000_RUNTIME)\\VC3000HDLL.dll $$shell_path($$DESTDIR)\\ >nul 2>&1")
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$quote(cmd /c "copy /Y $$shell_path($$PWD/lib3rd/ovdetector/release)\\OVDetector.dll $$shell_path($$DESTDIR)\\ >nul 2>&1")
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$quote(cmd /c "if not exist $$shell_path($$DESTDIR)\\weights mkdir $$shell_path($$DESTDIR)\\weights & copy /Y $$shell_path($$PWD/weights)\\Bbox128.onnx $$shell_path($$DESTDIR)\\weights\\ >nul 2>&1")
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$quote(cmd /c "copy /Y $$shell_path($$PWD/weights)\\YOLOV11S-Seg0805.onnx $$shell_path($$DESTDIR)\\weights\\ >nul 2>&1")
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$quote(cmd /c "if not exist $$shell_path($$DESTDIR)\\data mkdir $$shell_path($$DESTDIR)\\data & copy /Y $$shell_path($$PWD/data)\\welding_results.sql $$shell_path($$DESTDIR)\\data\\ >nul 2>&1")
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
}

FORMS += \
    Form/algconfigform.ui \
    Form/dataform.ui \
    Form/ioactionform.ui \
    Form/saveform.ui
