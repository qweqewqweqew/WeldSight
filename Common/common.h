#ifndef COMMON_H
#define COMMON_H
#include <IMVAPI/IMVApi.h>
#include <stdexcept>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <QHash>
#include <QString>
#include <opencv2/core.hpp>

#define WORKNUM 2

enum DetectionRsult
{
    OK,
    HXAREA_Err,
    HXSIZE_W_Err,
    HXSIZE_H_Err,
    YJ_Err,
    HXPY_Err,
    TBDIS_Err,
    LT_Err,
    HD_Err,
    HXGAP_Err,
    NG,
};

//enum HXDetectionRsult
//{
//    HXSIZE_Err,
//    HXAREA_Err,
//    HXPY_Err,
//    TBDIS_Err,
//};

struct Params
{
    std::string weldingdetectorfile;
    float conf_thresh;
    float nms_thresh;
    std::string Seg_weldingdetectorfile;
    float Seg_conf_thresh;
    float Seg_nms_thresh;
};

struct InSolderParam
{
    //融合相关参数
    float gray_param;
    int splitsize;
};

struct InThresholdParam
{
    //尺寸阈值
    float min_w = 5;
    float min_h = 5;
    float max_w = 10;
    float max_h = 10;
    //面积阈值
    float min_area = 15.1;
    float max_area = 100;
    //铜板距离阈值
    float max_TBDis = 3;
    float min_TBDis = 0.01;//最小没要求，目前预留一个参数
    //焊锡偏离程度
    float min_PY = 0.5;
    float max_PY = 2;
    // 月牙缺口判定
    float min_radius = 160;
    float HX_DiffArea_NG = 1.5;
    //相机标定
    float pix2mmScale_F_x = 0.010526;
    float pix2mmScale_F_y = 0.010526;
    float pix2mmScale_Z_x = 0.011765;
    float pix2mmScale_Z_y = 0.011765;
    bool AutoBDswitch = 0;
    //瑕疵检测
    float HD_min = 1;
    //黑点和露铜面积控制
    float LT_ALLArea_max = 0.01;
    float LT_MAXArea_max = 0.01;
    float HD_ALLArea_max = 12;
    float HD_MAXArea_max = 8;
};
struct HXResult
{
    int hxid=1;//焊锡编号
    float hx_weight=0;//焊锡长
    float hx_height=0;//焊锡宽
    float TBtoHX_distance=0;//裸露铜引脚长度
    float Pixeldis=0;//用于显示
    float center_pianyi=0;
    float PixelPY=0;//Y=用于显示
    //焊点检测框坐标
    float x1, y1, x2, y2;
};

struct TBResult
{
    //引脚检测框坐标
    float x1, y1, x2, y2;
    float YJ_H, YJ_W;
};

struct WeldingResult
{
    int id;
    quint64 inspectionGeneration = 0;
    QString fixtureCode;
    QString shellCode;
    DetectionRsult NO_HX_NGReults;
    std::uint16_t HX_NGReult = 0x0000;
    cv::Mat FusionMap;
    cv::Mat dst;
    cv::Rect weldingrect;
    float distance;
    std::vector<HXResult> hxResult;
    bool NG_OKflag = false;
    //瑕疵面积
    double HX_Area = 0;
    double HX_DiffArea = 0;
    double HD_SumArea=0, HD_MaxArea=0;
    double LT_SumArea=0, LT_MaxArea=0;
    bool HD_Flag = false, LT_Flag = false, HX_Gap = false;
    std::vector<cv::Rect> HDrect, LTrect;
    std::vector<std::vector<cv::Point>> HXGAP_max_contour;
    // 推理耗时(ms)
    qint64 inferTime = 0;
    // 关键步骤耗时(ms) - 用于本地循环测试分析
    qint64 t_clone = 0;
    qint64 t_resize = 0;
    qint64 t_infer = 0;
    qint64 t_bbox = 0;
    qint64 t_roi = 0;
    qint64 t_imwrite = 0;
    qint64 t_resizePlus = 0;
    qint64 t_seg = 0;
    qint64 t_post1 = 0;
    qint64 t_post2 = 0;
    qint64 t_post3 = 0;
    qint64 t_post4 = 0;
    qint64 t_draw = 0;
    qint64 t_total = 0;
};
struct DB_WeldingResult{
    int id;//序号
    QString hxPort;//端口
    int hxId;//焊锡编号
    int hxBatch; //焊锡批次
    float hxWeight=0;//焊锡长
    float hxHeight=0;//焊锡宽
    double hxArea = 0; //瑕疵面积
    float hxDistance=0;//裸露铜引脚长度
    float hxPianyi=0;    //焊锡偏移
    double hxLTSumArea=0; //漏铜面积
    double hxLTMaxArea=0; //漏铜最大面积
    double hxHDSumArea=0; //黑点面积
    double hxHDMaxArea=0; //黑点最大面积
    QString classifyStr; //二维码
    QString fixtureCode; //治具码
    QString shellCode; //外壳码
    QString imageCode; //结果图片名称
    bool hxReult = false; //检测结果
    QString hxReultsDescribe; //结果说明
    QString createTime;
    QString changeTime;

    DB_WeldingResult() = default;
};
struct CFrameResult
{
    int sig;
    int camera;
    bool result;
};

struct CameraSw {
    int m_nSource;
    int m_nComposite;
    int m_nOK;
    int m_nNG;
    CameraSw(int ng = 0, int ok = 0, int composite = 0, int source = 0)
        : m_nSource(source), m_nComposite(composite), m_nOK(ok), m_nNG(ng) {}
};

struct SaveSw
{
    CameraSw m_cSave1, m_cSave2, m_cSave3, m_cSave4, m_cSave5, m_cSave6;

    CameraSw& getCameraSw(int index) {
        switch(index) {
        case 1: return m_cSave1;
        case 2: return m_cSave2;
        case 3: return m_cSave3;
        case 4: return m_cSave4;
        case 5: return m_cSave5;
        case 6: return m_cSave6;
        default: throw std::out_of_range("Invalid group index");
        }
    }
};
// 成员指针类型别名
using CameraSwMember = int CameraSw::*;

// 图像保存设置结构
struct ImageSaveSettings {
    bool saveSource = false;    // 保存原图
    bool saveComposite = false; // 保存合成图
    bool saveOK = false;        // 保存OK图像
    bool saveNG = false;        // 保存NG图像

    // 转换为CameraSw
    CameraSw toCameraSw() const {
        CameraSw cs;
        cs.m_nSource = saveSource ? 1 : 0;
        cs.m_nComposite = saveComposite ? 1 : 0;
        cs.m_nOK = saveOK ? 1 : 0;
        cs.m_nNG = saveNG ? 1 : 0;
        return cs;
    }

    // 从CameraSw转换
    static ImageSaveSettings fromCameraSw(const CameraSw& cs) {
        ImageSaveSettings iss;
        iss.saveSource = cs.m_nSource == 1;
        iss.saveComposite = cs.m_nComposite == 1;
        iss.saveOK = cs.m_nOK == 1;
        iss.saveNG = cs.m_nNG == 1;
        return iss;
    }
};
inline constexpr std::array<CameraSwMember, 4> cameraSwMembers = {
    &CameraSw::m_nSource,
    &CameraSw::m_nComposite,
    &CameraSw::m_nOK,
    &CameraSw::m_nNG
};

// 帧信息
// frame imformation
class CFrameInfo
{
public:
    CFrameInfo()
    {
        m_pImageBuf = NULL;
        m_nBufferSize = 0;
        m_nWidth = 0;
        m_nHeight = 0;
        m_ePixelType = gvspPixelMono8;
        m_nPaddingX = 0;
        m_nPaddingY = 0;
        m_nTimeStamp = 0;
    }

    ~CFrameInfo()
    {
    }

public:
    QString cameraKey;
    std::shared_ptr<unsigned char> m_pImageBuf;
    int				m_nBufferSize;
    int				m_nWidth;
    int				m_nHeight;
    IMV_EPixelType	m_ePixelType;
    int				m_nPaddingX;
    int				m_nPaddingY;
    uint64_t		m_nTimeStamp;
};
#endif // COMMON_H
