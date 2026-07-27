#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <QCoreApplication>
#include <QString>
#include <opencv2/core.hpp>
//#include <nlohmann/json.hpp>

#include "Common/common.h"

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
	return QCoreApplication::applicationDirPath().toStdString();
}

struct WeldingInputTask
{
	cv::Mat image;
	int cameraId = -1;
	quint64 inspectionGeneration = 0;
	QString fixtureCode;
	QString shellCode;
};

class WeldingDetector
{
public:
	WeldingDetector();
	~WeldingDetector();
	bool Initialize(Params params);
	void SetIuputImages(std::vector<cv::Mat>& image, int id,
		quint64 inspectionGeneration = 0,
		const QString& fixtureCode = QString(),
		const QString& shellCode = QString());
	void SetIuputImagessignal(cv::Mat& image, int id,
		quint64 inspectionGeneration = 0,
		const QString& fixtureCode = QString(),
		const QString& shellCode = QString());
	void SetIuputImagesForWeldPoint(std::vector<cv::Mat>& image, int id, int weldPoint);
	void SetIuputImagessignalForWeldPoint(cv::Mat& image, int id, int weldPoint);
	void ResetWeldPointCounter(quint64 inspectionGeneration);
	void ForceResetWeldPointCounter(quint64 inspectionGeneration);
	bool SkipWeldPoint(int cameraId, quint64 inspectionGeneration);
	bool IsWeldPointCycleComplete();
	void StartProcessing();
	void StopProcessing();
	void SetThresholdParams(const InThresholdParam& params);
	void SetAutoCalibration(bool enabled);
	void Report(std::function<void(WeldingResult)> callback);
	void ProducerFunc();
	void ConsumerFunc(int id);
	cv::Mat MergeMultiLightImages(const std::vector<cv::Mat>& images, int fusionMethod);
	cv::Mat resize_with_padding(const cv::Mat& input, int target_width, int target_height,
		const cv::Scalar& padding_color);
	cv::Mat resize_with_paddingPlus(const cv::Mat& input, int target_width, int target_height,
		const cv::Scalar& padding_color);
	int findFirstSetBitPlatformIndependent(unsigned int num);
	
	std::vector<std::thread> m_Consumers;
	std::thread m_Producer;

private:
	void Process(int id, const cv::Mat& src, WeldingResult& result);
	void SegandObjDetec(WeldingResult& result);
	std::vector<std::vector<cv::Mat>> loadImageGroups(const std::string& folderPath);

private:
	struct DetectorModels;

	std::mutex mtx;
	std::mutex m_queueMtx;  // 专门用于保护 m_Images 队列的互斥锁
	std::mutex m_resultMtx;
	std::mutex m_paramMtx;
	std::mutex m_detectorMtx;
	std::condition_variable cvr;
	std::unique_ptr<DetectorModels> m_models;
	std::thread m_ConsumerThread;
	std::thread m_ProductorThread;
	std::queue<WeldingInputTask> m_Images;
	std::queue<WeldingInputTask> m_ImageTeams;
	std::queue<WeldingResult> m_Result;
	InThresholdParam m_inParam{};
	float conf_thresh;
	float nms_thresh;
	float Segconf_thresh;
	float Segnms_thresh;
	std::atomic_bool m_initalized{false};
	std::atomic_bool m_running{false};
};
