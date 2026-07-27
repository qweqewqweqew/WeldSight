#pragma once
#include "infer.h"
#include <thread>
#include <functional>
#include <vector>
//#include <nlohmann/json.hpp>
#include <direct.h>
#include <regex>
#include <cmath>

#define WORKNUM 2

struct Params
{
	std::string weldingdetectorfile;
	float conf_thresh;
	float nms_thresh;
};

struct InSolderParam
{
	//融合相关参数
	float gray_param;
	int splitsize;
};

struct HXResult
{
	int hxid;
	float hx_weight;
	float hx_height;
	float TBtoHX_distance;
	float center_pianyi;
};

struct WeldingResult 
{
	int id;
	cv::Mat dst;
	cv::Rect weldingrect;
	float distance;
	std::vector<HXResult> hxResult;
};

template<class T>
class Obj
{
private:
	static bool _instance;
public:

	static std::shared_ptr<T> GetInstance()
	{
		static std::shared_ptr<T> instance(new T);
		return instance;
	}
};

inline std::string GetProgramDir() 
{
	char buf[1024] = "";
	std::string path = std::string();
	_getcwd(buf, sizeof(buf));
	std::regex reg("\\\\");
	path = regex_replace(buf, reg, "/");
	return path;
	
}

class WeldingDetector
{
public:
	WeldingDetector();
	bool Initialize(Params params);
	void SetIuputImages(std::vector<cv::Mat>& image, int id);
	void Report(std::function<void(WeldingResult)> callback);
	void ProducerFunc();
	void ConsumerFunc(int id);
	cv::Mat MergeMultiLightImages(const std::vector<cv::Mat>& images, int fusionMethod = 0);
	
	std::vector<std::thread> m_Consumers;
	std::thread m_Producer;

private:
	void Process(int id, const cv::Mat& src, WeldingResult& result);

private:
	std::mutex mtx;
	std::condition_variable cvr;
	std::vector<OVYOLOV8> m_Detector;
	std::thread m_ConsumerThread;
	std::thread m_ProductorThread;
	std::queue<std::pair<cv::Mat, int>> m_Images;
	std::queue<std::pair<cv::Mat, int>> m_ImageTeams;
	std::queue<WeldingResult> m_Result;
	float conf_thresh;
	float nms_thresh;
	bool m_initalized;
};

