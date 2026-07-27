#pragma once

#include "Common/common.h"

class OVYOLOV26;

namespace SolderJointAlgorithm {

void process(int cameraId,
             const cv::Mat& source,
             WeldingResult& result,
             const InThresholdParam& params,
             OVYOLOV26& detector,
             OVYOLOV26& segmenter,
             float confidence,
             float segmentationConfidence);

} // namespace SolderJointAlgorithm
