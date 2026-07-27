#include "Api_Welding.h"

void doCallback(WeldingResult result)
{
	printf("id: %d rect[%d,%d,%d,%d]\n",  result.id, result.weldingrect.x, result.weldingrect.y, result.weldingrect.width, result.weldingrect.height);

}

void SendImage()
{
	std::string imagefile = GetProgramDir() + "/images/16.jpg";
	cv::Mat src = cv::imread(imagefile);
	std::vector<cv::Mat> imggroup;
	imggroup.push_back(src);
	imggroup.push_back(src);
	imggroup.push_back(src);
	imggroup.push_back(src);
	for (int i = 0; i < 10; i++) 
	{
		try 
		{
			int index = i % 2;
			Obj<WeldingDetector>::GetInstance()->SetIuputImages(imggroup, index);
		}
		catch (...)
		{
			printf("mistake ...\n");
		}
		Sleep(4);
	}
}

void main() 
{
	Params params;
	params.weldingdetectorfile = "yolov11_1.onnx";
	params.conf_thresh = 0.5;
	params.nms_thresh = 0.3;

	Obj<WeldingDetector>::GetInstance()->Initialize(params);
 
    std::thread ReportThread (&WeldingDetector::Report, Obj<WeldingDetector>::GetInstance(), doCallback);
	std::thread SendThread(SendImage);
	Obj<WeldingDetector>::GetInstance()->m_Producer = std::thread(&WeldingDetector::ProducerFunc, Obj<WeldingDetector>::GetInstance());
	for (int i = 0; i < WORKNUM; ++i)
	{
		Obj<WeldingDetector>::GetInstance()->m_Consumers[i] = std::thread(&WeldingDetector::ConsumerFunc, Obj<WeldingDetector>::GetInstance(), i);
	}
	
	SendThread.join();
	Obj<WeldingDetector>::GetInstance()->m_Producer.join();
	for (int i = 0; i < WORKNUM; i++)
	{	
		Obj<WeldingDetector>::GetInstance()->m_Consumers[i].join();
	}		
	ReportThread.join();
}

