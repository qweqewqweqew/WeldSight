#pragma once

#include "Common/common.h"

class OVYOLOV26;
class OVYOLOV8Seg;

namespace SolderJointAlgorithm {

void process(int cameraId,
             const cv::Mat& source,
             WeldingResult& result,
             const InThresholdParam& params,
             OVYOLOV26& detector,
             OVYOLOV8Seg& segmenter,
             float confidence,
             float segmentationConfidence);

} // namespace SolderJointAlgorithm
