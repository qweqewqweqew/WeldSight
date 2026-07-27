#include "weldinginitializationworker.h"
#include "weldingalgorithmmanager.h"

#include <Common/SysEnv.h>
#include <QDebug>
#include <exception>

namespace Alg {

WeldingInitializationWorker::WeldingInitializationWorker(QObject *parent)
    : QObject{parent}
{}

void WeldingInitializationWorker::doInitialize()
{
    // 配置参数

    std::vector<Params> vParams;
    Params param1;
    param1.weldingdetectorfile = "YOLO11S_F.onnx";
    param1.conf_thresh = 0.5;
    param1.nms_thresh = 0.3;

    Params param2;
    param2.weldingdetectorfile = "YOLO11S_Z.onnx";
    param2.conf_thresh = 0.5;
    param2.nms_thresh = 0.3;

    Params param3;
    param2.weldingdetectorfile = "yolov11_4.onnx";
    param2.conf_thresh = 0.5;
    param2.nms_thresh = 0.3;


    vParams.push_back(param1);
    vParams.push_back(param2);



    Params loadParam;
    loadParam.weldingdetectorfile = "Bbox640.onnx";
    loadParam.conf_thresh = 0.5;
    loadParam.nms_thresh = 0.3;
    loadParam.Seg_weldingdetectorfile = "YOLOV11S-Seg0724.onnx";
    loadParam.Seg_conf_thresh = 0.5;
    loadParam.Seg_nms_thresh = 0.3;

    bool success = false;
    try {
        success = Obj<WeldingDetector>::GetInstance()->Initialize(loadParam);
    } catch (const std::exception& e) {
        qWarning() << "Welding algorithm initialization exception:" << e.what();
    } catch (...) {
        qWarning() << "Welding algorithm initialization failed with an unknown exception";
    }
    if (success)
        SysEnv::instance().setModuleOk(SysEnv::ALGO);
    else
        SysEnv::instance().setModuleBad(SysEnv::ALGO);
    emit initializationFinished(success);
}
}
