#include "Api_Welding.h"
long long count = 1;

WeldingDetector::WeldingDetector() 
{
    m_Detector = std::vector<OVYOLOV8>(WORKNUM);
    m_Consumers = std::vector<std::thread>(WORKNUM);
    conf_thresh = 0.5;
    nms_thresh = 0.5;
    m_initalized = false;
}

bool WeldingDetector::Initialize(Params params)
{
    std::string weldingdetectorfile = params.weldingdetectorfile; 
    conf_thresh = params.conf_thresh;
    nms_thresh = params.nms_thresh;

    weldingdetectorfile = GetProgramDir() + "/weights/" + weldingdetectorfile;

    for (int i = 0; i < WORKNUM; i++) 
    {
        m_Detector[i].LoadOVModel(weldingdetectorfile.c_str(), -1, false);
        cv::Mat image = cv::Mat::zeros(cv::Size(640, 640), CV_8UC3);
        std::vector<BBox> bboxs;
        float confidence = 0.5;
        float nms = 0.5;
        for(int j = 0; j < 10; j++)
            m_Detector[i].Infer(image, bboxs, confidence, nms);
    }

    m_initalized = true;
    return true;
}

// 多光源图像拼接函数
cv::Mat WeldingDetector::MergeMultiLightImages(const std::vector<cv::Mat>& images, int fusionMethod)
{
    if (images.empty()) return cv::Mat();

    // Step 1: 图像对齐（假设已粗略对齐，否则需特征匹配）
    std::vector<cv::Mat> alignedImages = images;

    // Step 2: 光照归一化（CLAHE增强对比度）
    std::vector<cv::Mat> normalizedImages;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(32, 32));
    for (const auto& img : alignedImages) {
        cv::Mat normalized;
        if (img.channels() == 3) {
            std::vector<cv::Mat> channels;
            cv::split(img, channels);
            for (auto& ch : channels) clahe->apply(ch, ch);
            cv::merge(channels, normalized);
        }
        else {
            clahe->apply(img, normalized);
        }
        normalizedImages.push_back(normalized);
    }
    //std::vector<cv::Mat> normalizedImages = images;
    // Step 3: 图像融合（根据需求选择方法）
    cv::Mat result;
    if (fusionMethod == 0) {
        // 方法1: 简单平均（适用于均匀光照补偿）
        result = cv::Mat::zeros(normalizedImages[0].size(), normalizedImages[0].type());
        for (const auto& img : normalizedImages) {
            result += img / static_cast<int>(normalizedImages.size());
        }
    }
    else {
        // 方法2: 最大值合成（突出不同光源的特征）
        result = normalizedImages[0].clone();
        for (size_t i = 1; i < normalizedImages.size(); ++i) {
            //std::max(result, normalizedImages[i], result);
        }
    }

    return result;
}

void WeldingDetector::SetIuputImages(std::vector<cv::Mat>& image, int id)
{
    if (image.empty() || image.size() != 4)
        return;
    else
    {
        //拼接裁剪图像
        cv::Mat src;
        src = MergeMultiLightImages(image);
        m_Images.push(std::pair<cv::Mat, int>(src, id));
    }
}

void WeldingDetector::ProducerFunc() 
{
    while (true)
    {
        if (m_initalized) 
        {
            
            if (m_Images.size() > 0) 
            {
                std::lock_guard<std::mutex> lock(mtx);
                m_ImageTeams.push(m_Images.front());
                m_Images.pop();
                cvr.notify_one(); 
            }          
            
        }      
    }
}

void WeldingDetector::ConsumerFunc(int id)
{
    while (true)
    {
        if (m_initalized) 
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvr.wait(lock, [] {return !Obj<WeldingDetector>::GetInstance()->m_ImageTeams.empty(); }); 

            if (Obj<WeldingDetector>::GetInstance()->m_ImageTeams.size() > 0)
            {
                cv::Mat src = Obj<WeldingDetector>::GetInstance()->m_ImageTeams.front().first;
                int index = Obj<WeldingDetector>::GetInstance()->m_ImageTeams.front().second;
                Obj<WeldingDetector>::GetInstance()->m_ImageTeams.pop();

                WeldingResult result;
                result.id = index;
                Process(id, src, result);
                m_Result.push(result);
            }
            printf("process count: %ld\n", count++);
            lock.unlock();
            Sleep(4);
        }
    }
}

void WeldingDetector::Process(int id, const cv::Mat& src, WeldingResult& result)
{
    clock_t start = clock();
    std::vector<BBox>bboxs;
    cv::Mat image = src;
    m_Detector[id].Infer(image, bboxs, conf_thresh, nms_thresh);

    //添加处理 
    //1.面积计算及尺寸测量
    float area, hx_w, hx_h;
    HXResult hxResult;
    //float left_x, left_y;
    for (int i = 0; i < bboxs.size(); i++)
    {
        if (bboxs[i].label == 0)//焊锡处理
        {
            float bbox_x1 = bboxs[i].x1;
            float bbox_y1 = bboxs[i].y1;
            float bbox_x2 = bboxs[i].x2;
            float bbox_y2 = bboxs[i].y2;
            hx_w = bbox_x2 - bbox_x1;
            hx_h = bbox_y2 - bbox_y1;
            //输出结果
            hxResult.hxid = i;
            //result.distance = 
            //尺寸换算
            hx_w = hx_w * 254 / 96;
            hx_h = hx_h * 254 / 96;
            hx_w = round(hx_w * 10 / 10);
            hx_h = round(hx_h * 10 / 10);
            hxResult.hx_weight = hx_w;
            hxResult.hx_height = hx_h;
            result.hxResult.push_back(hxResult);
            cv::rectangle(image, cv::Point(bbox_x1, bbox_y1), cv::Point(bbox_x2, bbox_y2),cv::Scalar(0,255,0),2);
            cv::putText(image, std::to_string(hx_w), cv::Point((bbox_x1 + bbox_x2) / 2, bbox_y1 - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            cv::putText(image, std::to_string(hx_h), cv::Point(bbox_x2 + 10, (bbox_y1 + bbox_y2) / 2), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            //取检测框ROI区域，适当扩大范围
            //cv::Rect HXroi(bboxs[i].x1, bboxs[i].y1, hx_w, hx_h);
            //cv::Mat HX_Crop = image(HXroi);
            //int i = 1;
        }
        //else if (bboxs[i].label == 1)
        //{
        //    for (int i = 0; i < result; i++)
        //    {

        //    }
        //}
    }
    result.dst = image;
    //
    clock_t end = clock();
    std::cout << "consume : " << static_cast<double>(end - start) / CLOCKS_PER_SEC << " s\n";
}

void WeldingDetector::Report(std::function<void(WeldingResult)> callback)
{
    while (true) 
    {
        if (m_Result.size() > 0)
        {
            callback(m_Result.front());
            m_Result.pop();
        }
        Sleep(4);
    }
}

