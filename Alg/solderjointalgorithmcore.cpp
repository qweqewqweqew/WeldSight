#include "solderjointalgorithmcore.h"

#ifdef emit
#undef emit
#define MZ_RESTORE_QT_EMIT
#endif
#ifdef signals
#undef signals
#define MZ_RESTORE_QT_SIGNALS
#endif
#include "infer.h"
#ifdef MZ_RESTORE_QT_SIGNALS
#define signals Q_SIGNALS
#undef MZ_RESTORE_QT_SIGNALS
#endif
#ifdef MZ_RESTORE_QT_EMIT
#define emit Q_EMIT
#undef MZ_RESTORE_QT_EMIT
#endif

#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <opencv2/imgproc.hpp>
#include <queue>
#include <sstream>

namespace {

constexpr std::uint16_t kHxAreaError = 0x0080;
constexpr std::uint16_t kHxWidthError = 0x0040;
constexpr std::uint16_t kHxHeightError = 0x0020;
constexpr std::uint16_t kPinError = 0x0010;
constexpr std::uint16_t kHxOffsetError = 0x0008;
constexpr std::uint16_t kPinDistanceError = 0x0004;
constexpr std::uint16_t kCopperError = 0x0002;
constexpr std::uint16_t kBlackDotError = 0x0001;
constexpr std::uint16_t kHxGapError = 0x0100;
constexpr std::uint16_t kAllErrors = 0x01ff;

DetectionRsult primaryResultFromMask(std::uint16_t mask)
{
    if (mask == 0) return DetectionRsult::OK;
    if (mask == kAllErrors) return DetectionRsult::NG;
    if (mask & kHxAreaError) return DetectionRsult::HXAREA_Err;
    if (mask & kHxWidthError) return DetectionRsult::HXSIZE_W_Err;
    if (mask & kHxHeightError) return DetectionRsult::HXSIZE_H_Err;
    if (mask & kPinError) return DetectionRsult::YJ_Err;
    if (mask & kHxOffsetError) return DetectionRsult::HXPY_Err;
    if (mask & kPinDistanceError) return DetectionRsult::TBDIS_Err;
    if (mask & kHxGapError) return DetectionRsult::HXGAP_Err;
    if (mask & kCopperError) return DetectionRsult::LT_Err;
    if (mask & kBlackDotError) return DetectionRsult::HD_Err;
    return DetectionRsult::NG;
}

cv::Mat resizePad(const cv::Mat& input, int targetWidth, int targetHeight,
                  float& scale, int& padX, int& padY)
{
    const float scaleX = static_cast<float>(targetWidth) / input.cols;
    const float scaleY = static_cast<float>(targetHeight) / input.rows;
    scale = (std::min)(scaleX, scaleY);

    const int width = static_cast<int>(input.cols * scale);
    const int height = static_cast<int>(input.rows * scale);
    padX = (targetWidth - width) / 2;
    padY = (targetHeight - height) / 2;

    cv::Mat resized;
    cv::resize(input, resized, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
    cv::Mat output(targetHeight, targetWidth, input.type(), cv::Scalar(114, 114, 114));
    resized.copyTo(output(cv::Rect(padX, padY, width, height)));
    return output;
}

// 从引脚ROI图像中计算梯形短边中心相对于ROI中心的X偏移量
float computeTrapezoidOffset(const cv::Mat& roiImg)
{
    if (roiImg.empty()) return 0.0f;

    cv::Mat gray, binary;
    cv::Mat hsv;
    cv::cvtColor(roiImg, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, cv::Scalar(10, 20, 30), cv::Scalar(40, 255, 255), mask);
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    cv::Mat yellowRegion;
    cv::bitwise_and(roiImg, roiImg, yellowRegion, mask);

    cv::cvtColor(yellowRegion, gray, cv::COLOR_BGR2GRAY);
    cv::medianBlur(gray, gray, 5);
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 顶部凹陷填充（触边即止）
    int leftWhite = -1, rightWhite = -1;
    for (int c = 0; c < binary.cols; ++c) {
        if (binary.at<uchar>(0, c) == 255) {
            leftWhite = c;
            break;
        }
    }
    for (int c = binary.cols - 1; c >= 0; --c) {
        if (binary.at<uchar>(0, c) == 255) {
            rightWhite = c;
            break;
        }
    }
    if (leftWhite != -1 && rightWhite != -1) {
        const int centerCol = (leftWhite + rightWhite) / 2;
        const cv::Point centerPoint(centerCol, 0);
        if (binary.at<uchar>(centerPoint) == 0) {
            std::queue<cv::Point> q;
            q.push(centerPoint);
            binary.at<uchar>(centerPoint) = 255;

            bool stopped = false;
            while (!q.empty() && !stopped) {
                const cv::Point p = q.front();
                q.pop();

                if (p.x == 0 || p.x == binary.cols - 1) {
                    stopped = true;
                    continue;
                }
                if (p.x > 0 && binary.at<uchar>(p.y, p.x - 1) == 0) {
                    const cv::Point left(p.x - 1, p.y);
                    binary.at<uchar>(left) = 255;
                    q.push(left);
                    if (left.x == 0) stopped = true;
                }
                if (!stopped && p.x < binary.cols - 1 && binary.at<uchar>(p.y, p.x + 1) == 0) {
                    const cv::Point right(p.x + 1, p.y);
                    binary.at<uchar>(right) = 255;
                    q.push(right);
                    if (right.x == binary.cols - 1) stopped = true;
                }
                if (!stopped && p.y < binary.rows - 1 && binary.at<uchar>(p.y + 1, p.x) == 0) {
                    const cv::Point down(p.x, p.y + 1);
                    binary.at<uchar>(down) = 255;
                    q.push(down);
                }
            }
        }
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return 0.0f;

    const auto largest = std::max_element(contours.begin(), contours.end(),
        [](const auto& lhs, const auto& rhs) {
            return cv::contourArea(lhs) < cv::contourArea(rhs);
        });

    std::vector<cv::Point> approx;
    const double perimeter = cv::arcLength(*largest, true);
    bool found = false;
    for (double eps = 0.02; eps <= 0.15; eps += 0.01) {
        cv::approxPolyDP(*largest, approx, eps * perimeter, true);
        if (approx.size() == 4) {
            found = true;
            break;
        }
    }
    if (!found) {
        const cv::RotatedRect rect = cv::minAreaRect(*largest);
        cv::Point2f vertices[4];
        rect.points(vertices);
        approx.clear();
        for (int i = 0; i < 4; ++i)
            approx.emplace_back(static_cast<int>(vertices[i].x),
                                static_cast<int>(vertices[i].y));
    }

    // 寻找最底边（平均 y 最大）
    int bottomIdx = 0;
    double maxAvgY = (approx[0].y + approx[1].y) / 2.0;
    for (int i = 1; i < 4; ++i) {
        const double avgY = (approx[i].y + approx[(i + 1) % 4].y) / 2.0;
        if (avgY > maxAvgY) {
            maxAvgY = avgY;
            bottomIdx = i;
        }
    }

    const float midX = (approx[bottomIdx].x + approx[(bottomIdx + 1) % 4].x) / 2.0f;
    const float roiCenterX = roiImg.cols / 2.0f;
    return midX - roiCenterX;
}

bool analyzeSegments(cv::Mat& image,
                     cv::Point2f& center,
                     float radius,
                     WeldingResult& result,
                     float totalScale,
                     float pix2mmScaleX,
                     float pix2mmScaleY,
                     const InThresholdParam& params,
                     OVYOLOV26& segmenter,
                     float confidence)
{
    if (image.empty() || totalScale <= 0.0f) return false;

    const double ratio = (pix2mmScaleX * pix2mmScaleY) /
                         (totalScale * totalScale);
    std::vector<Instance> segments;
    segmenter.SegInfer(image, segments, confidence, 160);
    if (segments.empty()) return false;

    bool foundSolderMask = false;
    cv::Mat solderMask;
    for (const Instance& segment : segments) {
        cv::Mat binaryMask;
        cv::threshold(segment.mask, binaryMask, 0, 255, cv::THRESH_BINARY);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binaryMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if (contours.empty()) continue;

        if (segment.label == 0) {
            foundSolderMask = true;
            const auto largest = std::max_element(
                contours.begin(), contours.end(),
                [](const auto& lhs, const auto& rhs) {
                    return cv::contourArea(lhs) < cv::contourArea(rhs);
                });
            const double maxArea = cv::contourArea(*largest);
            if (maxArea > 0.0) {
                result.HXrect.push_back(cv::boundingRect(*largest));
                result.HX_Area = maxArea * ratio;
                solderMask = binaryMask.clone();
            }
        } else if (segment.label == 1) {
            result.HD_Flag = true;
            double maxArea = 0.0;
            double sumArea = 0.0;
            for (const auto& contour : contours) {
                result.HDrect.push_back(cv::boundingRect(contour));
                const double area = cv::contourArea(contour) * ratio;
                if (area > params.HD_min) {
                    maxArea = (std::max)(maxArea, area);
                    sumArea += area;
                }
            }
            result.HD_MaxArea = maxArea;
            result.HD_SumArea = sumArea;
        } else if (segment.label == 2) {
            result.LT_Flag = true;
            double maxArea = 0.0;
            double sumArea = 0.0;
            for (const auto& contour : contours) {
                result.LTrect.push_back(cv::boundingRect(contour));
                const double area = cv::contourArea(contour) * ratio;
                maxArea = (std::max)(maxArea, area);
                sumArea += area;
            }
            result.LT_MaxArea = maxArea;
            result.LT_SumArea = sumArea;
        } else if (segment.label == 3) {
            result.YJ_Flag = true;
            const auto largest = std::max_element(
                contours.begin(), contours.end(),
                [](const auto& lhs, const auto& rhs) {
                    return cv::contourArea(lhs) < cv::contourArea(rhs);
                });

            result.YJrect.clear();
            result.YJrect.push_back(cv::boundingRect(*largest));

            std::vector<cv::Point> approx;
            const double perimeter = cv::arcLength(*largest, true);
            bool found = false;
            for (double eps = 0.02; eps <= 0.15; eps += 0.01) {
                cv::approxPolyDP(*largest, approx, eps * perimeter, true);
                if (approx.size() == 4) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                const cv::RotatedRect rect = cv::minAreaRect(*largest);
                cv::Point2f vertices[4];
                rect.points(vertices);
                approx.clear();
                for (int i = 0; i < 4; ++i)
                    approx.emplace_back(static_cast<int>(vertices[i].x),
                                        static_cast<int>(vertices[i].y));
            }

            int bottomIdx = 0;
            double maxAvgY = (approx[0].y + approx[1].y) / 2.0;
            for (int i = 1; i < 4; ++i) {
                const double avgY = (approx[i].y + approx[(i + 1) % 4].y) / 2.0;
                if (avgY > maxAvgY) {
                    maxAvgY = avgY;
                    bottomIdx = i;
                }
            }
            center.x = (approx[bottomIdx].x + approx[(bottomIdx + 1) % 4].x) / 2.0f;
        }
    }

    if (foundSolderMask && !result.HXrect.empty() && !solderMask.empty()) {
        center.y = result.HXrect[0].y + result.HXrect[0].height / 2.0f;
        if (center.x == 0.0f)
            center.x = result.HXrect[0].x + result.HXrect[0].width / 2.0f;

        cv::Mat circleMask = cv::Mat::zeros(image.size(), CV_8UC1);
        cv::circle(circleMask, center, static_cast<int>(radius), cv::Scalar(255), -1);
        cv::Mat solderInCircle;
        cv::Mat difference;
        cv::Mat outsideCircle;
        cv::bitwise_and(solderMask, circleMask, solderInCircle);
        cv::bitwise_xor(circleMask, solderInCircle, difference);
        cv::bitwise_xor(solderMask, solderInCircle, outsideCircle);
        if (cv::countNonZero(outsideCircle) == 0) {
            cv::line(difference,
                     cv::Point(static_cast<int>(center.x), static_cast<int>(center.y)),
                     cv::Point(static_cast<int>(center.x), 0),
                     cv::Scalar(0), 2, cv::LINE_8);
        }
        result.HX_DiffArea = cv::countNonZero(difference) * ratio;
        result.HX_Gap = true;

        std::vector<std::vector<cv::Point>> differenceContours;
        cv::findContours(difference, differenceContours,
                         cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if (!differenceContours.empty()) {
            const auto largest = std::max_element(
                differenceContours.begin(), differenceContours.end(),
                [](const auto& lhs, const auto& rhs) {
                    return cv::contourArea(lhs) < cv::contourArea(rhs);
                });
            result.HXGAP_max_contour = {*largest};
        }
    }

    return foundSolderMask;
}

void processWith128Input(int cameraId,
                         const cv::Mat& source,
                         WeldingResult& result,
                         const InThresholdParam& params,
                         OVYOLOV26& detector,
                         OVYOLOV26& segmenter,
                         float confidence,
                         float segmentationConfidence)
{
    QElapsedTimer totalTimer;
    totalTimer.start();

    result.HX_NGReult = 0;
    result.HX_Area = 0.0;
    result.HX_DiffArea = 0.0;
    result.HD_SumArea = 0.0;
    result.HD_MaxArea = 0.0;
    result.LT_SumArea = 0.0;
    result.LT_MaxArea = 0.0;
    result.HD_Flag = false;
    result.LT_Flag = false;
    result.HX_Gap = false;
    result.YJ_Flag = false;
    result.hxResult.clear();
    result.HDrect.clear();
    result.LTrect.clear();
    result.HXrect.clear();
    result.YJrect.clear();
    result.HXGAP_max_contour.clear();

    HXResult solder{};
    result.hxResult.push_back(solder);

    QElapsedTimer stepTimer;
    stepTimer.start();
    cv::Mat display = source.clone();
    result.t_clone = stepTimer.elapsed();
    result.FusionMap = display.clone();

    float detectionScale = 1.0f;
    int detectionPadX = 0;
    int detectionPadY = 0;
    stepTimer.restart();
    cv::Mat detectionImage = resizePad(display, 128, 128,
                                       detectionScale, detectionPadX, detectionPadY);
    result.t_resize = stepTimer.elapsed();
    const int detectionContentWidth = static_cast<int>(display.cols * detectionScale);
    const int detectionContentHeight = static_cast<int>(display.rows * detectionScale);

    const float pix2mmScaleX = cameraId == 0
        ? params.pix2mmScale_F_x : params.pix2mmScale_Z_x;
    const float pix2mmScaleY = cameraId == 0
        ? params.pix2mmScale_F_y : params.pix2mmScale_Z_y;

    std::vector<BBox> boxes;
    stepTimer.restart();
    detector.Infer(detectionImage, boxes, confidence, 50);
    result.t_infer = stepTimer.elapsed();
    result.inferTime = result.t_infer;

    if (boxes.empty()) result.HX_NGReult = kAllErrors;

    TBResult pin{};
    float pinFix = 0.0f;
    bool hasSolder = false;
    bool hasPin = false;
    float solderX1 = 0.0f;
    float solderY1 = 0.0f;
    float solderX2 = 0.0f;
    float solderY2 = 0.0f;
    int sourceRoiX = 0;
    int sourceRoiY = 0;
    cv::Mat solderCrop;
    const cv::Mat imageForPin = detectionImage.clone();

    for (const BBox& box : boxes) {
        if (box.label == 0) {
            hasSolder = true;
            solderX1 = box.x1;
            solderY1 = box.y1;
            solderX2 = box.x2;
            solderY2 = box.y2;
            const float width = solderX2 - solderX1;
            const float height = solderY2 - solderY1;

            constexpr int expansion = 30;
            const int roiX = (std::max)(0, static_cast<int>(solderX1 - expansion));
            const int roiY = (std::max)(0, static_cast<int>(solderY1 - expansion));
            const int roiRight = (std::min)(detectionImage.cols,
                                            static_cast<int>(solderX2 + expansion));
            const int roiBottom = (std::min)(detectionImage.rows,
                                             static_cast<int>(solderY2 + expansion));
            if (roiRight > roiX && roiBottom > roiY &&
                detectionContentWidth > 0 && detectionContentHeight > 0) {
                const float sourceRoiXF = (roiX - detectionPadX) *
                    static_cast<float>(display.cols) / detectionContentWidth;
                const float sourceRoiYF = (roiY - detectionPadY) *
                    static_cast<float>(display.rows) / detectionContentHeight;
                const float sourceRoiWF = (roiRight - roiX) *
                    static_cast<float>(display.cols) / detectionContentWidth;
                const float sourceRoiHF = (roiBottom - roiY) *
                    static_cast<float>(display.rows) / detectionContentHeight;
                sourceRoiX = (std::max)(0, static_cast<int>(std::floor(sourceRoiXF)));
                sourceRoiY = (std::max)(0, static_cast<int>(std::floor(sourceRoiYF)));
                const int sourceRoiW = (std::min)(
                    static_cast<int>(std::ceil(sourceRoiWF)), display.cols - sourceRoiX);
                const int sourceRoiH = (std::min)(
                    static_cast<int>(std::ceil(sourceRoiHF)), display.rows - sourceRoiY);
                if (sourceRoiW > 0 && sourceRoiH > 0) {
                    solderCrop = display(cv::Rect(sourceRoiX, sourceRoiY,
                                                  sourceRoiW, sourceRoiH)).clone();
                }
            }

            if (params.AutoBDswitch) {
                std::ostringstream widthText;
                std::ostringstream heightText;
                widthText << std::fixed << std::setprecision(2) << width / detectionScale;
                heightText << std::fixed << std::setprecision(2) << height / detectionScale;
                cv::putText(display, "BDPix: X:" + widthText.str(), cv::Point(240, 80),
                            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
                cv::putText(display, "Y:" + heightText.str(), cv::Point(360, 110),
                            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            }

            solder.hx_weight = (width * pix2mmScaleX) / detectionScale;
            solder.hx_height = (height * pix2mmScaleY) / detectionScale;
            solder.x1 = solderX1;
            solder.y1 = solderY1;
            solder.x2 = solderX2;
            solder.y2 = solderY2;

            if (solder.hx_height < params.min_h || solder.hx_height > params.max_h)
                result.HX_NGReult |= kHxHeightError;
            if (solder.hx_weight < params.min_w || solder.hx_weight > params.max_w)
                result.HX_NGReult |= kHxWidthError;

            result.hxResult[0] = solder;
        } else if (box.label == 1) {
            hasPin = true;
            pin.x1 = box.x1;
            pin.y1 = box.y1;
            pin.x2 = box.x2;
            pin.y2 = box.y2;
            pin.YJ_H = box.y2 - box.y1;
            pin.YJ_W = box.x2 - box.x1;
            const cv::Rect pinRect =
                cv::Rect(static_cast<int>(pin.x1), static_cast<int>(pin.y1),
                         static_cast<int>(pin.YJ_W), static_cast<int>(pin.YJ_H)) &
                cv::Rect(0, 0, imageForPin.cols, imageForPin.rows);
            if (pinRect.width > 0 && pinRect.height > 0)
                pinFix = computeTrapezoidOffset(imageForPin(pinRect));
        }
    }

    if (hasSolder && !solderCrop.empty() &&
        detectionContentWidth > 0 && detectionContentHeight > 0) {
        cv::Point2f detectionCenter;
        detectionCenter.y = (solderY1 + solderY2) / 2.0f;
        detectionCenter.x = hasPin
            ? (pin.x1 + pin.x2) / 2.0f + pinFix
            : (solderX1 + solderX2) / 2.0f;

        float cropScale = 1.0f;
        int cropPadX = 0;
        int cropPadY = 0;
        cv::Mat crop640 = resizePad(solderCrop, 640, 640,
                                    cropScale, cropPadX, cropPadY);
        const int cropContentWidth = static_cast<int>(solderCrop.cols * cropScale);
        const int cropContentHeight = static_cast<int>(solderCrop.rows * cropScale);
        cv::Point2f sourceCenter(
            (detectionCenter.x - detectionPadX) * static_cast<float>(display.cols) /
                detectionContentWidth - sourceRoiX,
            (detectionCenter.y - detectionPadY) * static_cast<float>(display.rows) /
                detectionContentHeight - sourceRoiY);
        cv::Point2f center640(sourceCenter.x * cropScale + cropPadX,
                              sourceCenter.y * cropScale + cropPadY);

        stepTimer.restart();
        const bool analyzed = analyzeSegments(
            crop640, center640, params.min_radius, result,
            cropScale, pix2mmScaleX, pix2mmScaleY,
            params, segmenter, segmentationConfidence);
        result.t_seg = stepTimer.elapsed();

        if (!analyzed) {
            qWarning() << "Solder area segmentation failed, camera=" << cameraId + 1;
        } else if (result.HX_Area != 0.0 &&
                   (result.HX_Area < params.min_area ||
                    result.HX_Area > params.max_area)) {
            result.HX_NGReult |= kHxAreaError;
        }

        const auto mapPoint = [&](const cv::Point2f& point) {
            const cv::Point2f cropPoint(
                (point.x - cropPadX) * solderCrop.cols / cropContentWidth,
                (point.y - cropPadY) * solderCrop.rows / cropContentHeight);
            return cv::Point2f(cropPoint.x + sourceRoiX, cropPoint.y + sourceRoiY);
        };

        if (!result.YJrect.empty()) {
            hasPin = true;
            for (const cv::Rect& rect : result.YJrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(display,
                              cv::Rect(static_cast<int>(topLeft.x) - 2,
                                       static_cast<int>(topLeft.y) - 2,
                                       static_cast<int>(bottomRight.x - topLeft.x) + 4,
                                       static_cast<int>(bottomRight.y - topLeft.y) + 4),
                              cv::Scalar(255, 0, 0), 2);
            }
        }

        if (result.HX_Gap && result.HX_DiffArea > params.HX_DiffArea_NG) {
            result.HX_NGReult |= kHxGapError;
            const cv::Point2f mappedCenter = mapPoint(center640);
            const int mappedRadius = static_cast<int>(
                params.min_radius * solderCrop.cols / cropContentWidth);
            cv::circle(display, mappedCenter, mappedRadius, cv::Scalar(0, 255, 255), 2);

            if (!result.HXGAP_max_contour.empty()) {
                std::vector<std::vector<cv::Point>> mappedContours(1);
                for (const cv::Point& point : result.HXGAP_max_contour.front()) {
                    const cv::Point2f mapped = mapPoint(point);
                    mappedContours.front().emplace_back(
                        static_cast<int>(mapped.x), static_cast<int>(mapped.y));
                }
                cv::drawContours(display, mappedContours, -1, cv::Scalar(0, 0, 255), -1);
            }
        }

        if (result.HD_Flag &&
            (result.HD_MaxArea > params.HD_MAXArea_max ||
             result.HD_SumArea > params.HD_ALLArea_max)) {
            result.HX_NGReult |= kBlackDotError;
            for (const cv::Rect& rect : result.HDrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(display, cv::Rect(topLeft, bottomRight),
                              cv::Scalar(0, 125, 155), 2);
            }
        }

        if (result.LT_Flag &&
            (result.LT_MaxArea > params.LT_MAXArea_max ||
             result.LT_SumArea > params.LT_ALLArea_max)) {
            result.HX_NGReult |= kCopperError;
            for (const cv::Rect& rect : result.LTrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(display, cv::Rect(topLeft, bottomRight),
                              cv::Scalar(0, 255, 255), 2);
            }
        }

        if (!result.HXrect.empty()) {
            for (const cv::Rect& rect : result.HXrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(display,
                              cv::Rect(static_cast<int>(topLeft.x) - 2,
                                       static_cast<int>(topLeft.y) - 2,
                                       static_cast<int>(bottomRight.x - topLeft.x) + 4,
                                       static_cast<int>(bottomRight.y - topLeft.y) + 4),
                              cv::Scalar(0, 255, 0), 2);
                result.hxResult[0].x1 = topLeft.x;
                result.hxResult[0].y1 = topLeft.y;
                result.hxResult[0].x2 = bottomRight.x;
                result.hxResult[0].y2 = bottomRight.y;
            }
        }

        if (result.YJrect.size() == 1 && result.HXrect.size() == 1) {
            HXResult& measuredSolder = result.hxResult[0];
            const float solderCenter =
                result.HXrect[0].x + result.HXrect[0].width / 2.0f;
            const float pinCenter = center640.x;
            measuredSolder.PixelPY = pinCenter - solderCenter;
            measuredSolder.center_pianyi =
                (pinCenter - result.HXrect[0].x) /
                (result.HXrect[0].x + result.HXrect[0].width - pinCenter + 1e-8f);
            if (measuredSolder.center_pianyi > params.max_PY ||
                measuredSolder.center_pianyi < params.min_PY)
                result.HX_NGReult |= kHxOffsetError;

            const cv::Point2f lineStart = mapPoint(cv::Point2f(
                solderCenter + measuredSolder.PixelPY,
                result.HXrect[0].y + result.HXrect[0].height + 20.0f));
            const cv::Point2f lineEnd = mapPoint(cv::Point2f(
                solderCenter + measuredSolder.PixelPY,
                result.YJrect[0].y - 20.0f));
            cv::line(display, lineStart, lineEnd, cv::Scalar(221, 10, 221), 2);

            const cv::Point2f left = mapPoint(cv::Point2f(
                result.HXrect[0].x - 20.0f, result.YJrect[0].y));
            const cv::Point2f right = mapPoint(cv::Point2f(
                result.HXrect[0].x + result.HXrect[0].width + 20.0f,
                result.YJrect[0].y));
            cv::line(display, left, right, cv::Scalar(255, 255, 0), 2);
        }

        if (hasSolder && result.YJrect.size() == 1) {
            HXResult& measuredSolder = result.hxResult[0];
            measuredSolder.Pixeldis =
                (std::max)(result.YJrect[0].height - 1.0f, 0.0f) *
                solderCrop.cols / cropContentWidth;
            measuredSolder.TBtoHX_distance = measuredSolder.Pixeldis * pix2mmScaleY;
            if (measuredSolder.TBtoHX_distance > params.max_TBDis)
                result.HX_NGReult |= kPinDistanceError;
        }
    }

    if (!boxes.empty() && (!hasSolder || !hasPin))
        result.HX_NGReult |= kPinError;

    result.NO_HX_NGReults = primaryResultFromMask(result.HX_NGReult);
    result.NG_OKflag = result.NO_HX_NGReults != DetectionRsult::OK;
    result.dst = display;
    result.t_total = totalTimer.elapsed();
}

} // namespace

namespace SolderJointAlgorithm {

void process(int cameraId,
             const cv::Mat& source,
             WeldingResult& result,
             const InThresholdParam& params,
             OVYOLOV26& detector,
             OVYOLOV26& segmenter,
             float confidence,
             float segmentationConfidence)
{
    processWith128Input(cameraId, source, result, params, detector, segmenter,
                        confidence, segmentationConfidence);
    return;

    QElapsedTimer totalTimer;
    totalTimer.start();

    result.HX_NGReult = 0;
    result.HX_Area = 0.0;
    result.HX_DiffArea = 0.0;
    result.HD_SumArea = 0.0;
    result.HD_MaxArea = 0.0;
    result.LT_SumArea = 0.0;
    result.LT_MaxArea = 0.0;
    result.HD_Flag = false;
    result.LT_Flag = false;
    result.HX_Gap = false;
    result.hxResult.clear();
    result.HDrect.clear();
    result.LTrect.clear();
    result.HXrect.clear();
    result.HXGAP_max_contour.clear();

    HXResult solder{};
    result.hxResult.push_back(solder);

    QElapsedTimer stepTimer;
    stepTimer.start();
    cv::Mat image = source.clone();
    result.t_clone = stepTimer.elapsed();
    result.FusionMap = image.clone();

    float sourceScale = 1.0f;
    int sourcePadX = 0;
    int sourcePadY = 0;
    stepTimer.restart();
    image = resizePad(image, 640, 640, sourceScale, sourcePadX, sourcePadY);
    result.t_resize = stepTimer.elapsed();

    const float pix2mmScaleX = cameraId == 0
        ? params.pix2mmScale_F_x : params.pix2mmScale_Z_x;
    const float pix2mmScaleY = cameraId == 0
        ? params.pix2mmScale_F_y : params.pix2mmScale_Z_y;

    std::vector<BBox> boxes;
    stepTimer.restart();
    detector.Infer(image, boxes, confidence, 300);
    result.t_infer = stepTimer.elapsed();
    result.inferTime = result.t_infer;

    if (boxes.empty()) result.HX_NGReult = kAllErrors;

    TBResult pin{};
    float pinFix = 0.0f;
    bool hasSolder = false;
    bool hasPin = false;
    float solderX1 = 0.0f;
    float solderY1 = 0.0f;
    float solderX2 = 0.0f;
    float solderY2 = 0.0f;
    int roiX = 0;
    int roiY = 0;
    cv::Mat solderCrop;
    // 画框前的干净图像，用于裁取引脚ROI计算梯形偏移
    const cv::Mat imageForPin = image.clone();

    for (const BBox& box : boxes) {
        if (box.label == 0) {
            hasSolder = true;
            solderX1 = box.x1;
            solderY1 = box.y1;
            solderX2 = box.x2;
            solderY2 = box.y2;
            const float width = solderX2 - solderX1;
            const float height = solderY2 - solderY1;

            constexpr int expansion = 80;
            roiX = (std::max)(0, static_cast<int>(solderX1 - expansion));
            roiY = (std::max)(0, static_cast<int>(solderY1 - expansion));
            const int roiRight = (std::min)(image.cols,
                                          static_cast<int>(solderX2 + expansion));
            const int roiBottom = (std::min)(image.rows,
                                           static_cast<int>(solderY2 + expansion));
            if (roiRight > roiX && roiBottom > roiY) {
                solderCrop = image(cv::Rect(roiX, roiY,
                                            roiRight - roiX, roiBottom - roiY)).clone();
            }

            if (params.AutoBDswitch) {
                std::ostringstream widthText;
                std::ostringstream heightText;
                widthText << std::fixed << std::setprecision(2) << width / sourceScale;
                heightText << std::fixed << std::setprecision(2) << height / sourceScale;
                cv::putText(image, "BDPix: X:" + widthText.str(), cv::Point(240, 80),
                            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
                cv::putText(image, "Y:" + heightText.str(), cv::Point(360, 110),
                            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
            }

            solder.hx_weight = (width * pix2mmScaleX) / sourceScale;
            solder.hx_height = (height * pix2mmScaleY) / sourceScale;
            solder.x1 = solderX1;
            solder.y1 = solderY1;
            solder.x2 = solderX2;
            solder.y2 = solderY2;

            if (solder.hx_height < params.min_h || solder.hx_height > params.max_h)
                result.HX_NGReult |= kHxHeightError;
            if (solder.hx_weight < params.min_w || solder.hx_weight > params.max_w)
                result.HX_NGReult |= kHxWidthError;

            result.hxResult[0] = solder;
            // 焊锡框推迟到分割后绘制：若贴近框唯一则改画贴近框
        } else if (box.label == 1) {
            hasPin = true;
            pin.x1 = box.x1;
            pin.y1 = box.y1;
            pin.x2 = box.x2;
            pin.y2 = box.y2;
            pin.YJ_H = box.y2 - box.y1;
            pin.YJ_W = box.x2 - box.x1;
            cv::rectangle(image, cv::Point(pin.x1, pin.y1),
                          cv::Point(pin.x2, pin.y2), cv::Scalar(255, 0, 0), 2);
            const cv::Rect pinRect =
                cv::Rect(static_cast<int>(pin.x1), static_cast<int>(pin.y1),
                         static_cast<int>(pin.YJ_W), static_cast<int>(pin.YJ_H)) &
                cv::Rect(0, 0, imageForPin.cols, imageForPin.rows);
            if (pinRect.width > 0 && pinRect.height > 0)
                pinFix = computeTrapezoidOffset(imageForPin(pinRect));
        }
    }

    if (!boxes.empty() && (!hasSolder || !hasPin))
        result.HX_NGReult |= kPinError;

    if (hasSolder && !solderCrop.empty()) {
        cv::Point2f center;
        center.y = ((solderY1 + solderY2) / 2.0f) - roiY;
        center.x = hasPin
            ? ((pin.x1 + pin.x2) / 2.0f) - roiX + pinFix
            : ((solderX1 + solderX2) / 2.0f) - roiX;

        float cropScale = 1.0f;
        int cropPadX = 0;
        int cropPadY = 0;
        cv::Mat crop640 = resizePad(solderCrop, 640, 640,
                                    cropScale, cropPadX, cropPadY);
        cv::Point2f center640(center.x * cropScale + cropPadX,
                              center.y * cropScale + cropPadY);

        stepTimer.restart();
        const bool analyzed = analyzeSegments(
            crop640, center640, params.min_radius, result,
            sourceScale * cropScale, pix2mmScaleX, pix2mmScaleY,
            params, segmenter, segmentationConfidence);
        result.t_seg = stepTimer.elapsed();

        if (!analyzed) {
            // 面积判定放宽：分割失败仅告警，不置NG
            qWarning() << "Solder area segmentation failed, camera=" << cameraId + 1;
        } else if (result.HX_Area != 0.0 &&
                   (result.HX_Area < params.min_area ||
                    result.HX_Area > params.max_area)) {
            result.HX_NGReult |= kHxAreaError;
        }

        const auto mapPoint = [&](const cv::Point2f& point) {
            const cv::Point2f cropPoint((point.x - cropPadX) / cropScale,
                                        (point.y - cropPadY) / cropScale);
            return cv::Point2f(cropPoint.x + roiX, cropPoint.y + roiY);
        };

        if (result.HX_Gap && result.HX_DiffArea > params.HX_DiffArea_NG) {
            result.HX_NGReult |= kHxGapError;
            const cv::Point2f mappedCenter = mapPoint(center640);
            const int mappedRadius = static_cast<int>(params.min_radius / cropScale);
            cv::circle(image, mappedCenter, mappedRadius, cv::Scalar(0, 255, 255), 2);

            if (!result.HXGAP_max_contour.empty()) {
                std::vector<std::vector<cv::Point>> mappedContours(1);
                for (const cv::Point& point : result.HXGAP_max_contour.front()) {
                    const cv::Point2f mapped = mapPoint(point);
                    mappedContours.front().emplace_back(
                        static_cast<int>(mapped.x), static_cast<int>(mapped.y));
                }
                cv::drawContours(image, mappedContours, -1, cv::Scalar(0, 0, 255), -1);
            }
        }

        if (result.HD_Flag &&
            (result.HD_MaxArea > params.HD_MAXArea_max ||
             result.HD_SumArea > params.HD_ALLArea_max)) {
            result.HX_NGReult |= kBlackDotError;
            for (const cv::Rect& rect : result.HDrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(image, cv::Rect(topLeft, bottomRight),
                              cv::Scalar(0, 125, 155), 2);
            }
        }

        if (result.LT_Flag &&
            (result.LT_MaxArea > params.LT_MAXArea_max ||
             result.LT_SumArea > params.LT_ALLArea_max)) {
            result.HX_NGReult |= kCopperError;
            for (const cv::Rect& rect : result.LTrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(image, cv::Rect(topLeft, bottomRight),
                              cv::Scalar(0, 255, 255), 2);
            }
        }

        // 焊锡贴近框：分割轮廓唯一时用轮廓外接矩形替代检测框
        if (result.HXrect.size() != 1) {
            cv::rectangle(image, cv::Point(solderX1, solderY1),
                          cv::Point(solderX2, solderY2), cv::Scalar(0, 255, 0), 2);
        } else {
            for (const cv::Rect& rect : result.HXrect) {
                const cv::Point2f topLeft = mapPoint(rect.tl());
                const cv::Point2f bottomRight = mapPoint(rect.br());
                cv::rectangle(image,
                              cv::Rect(static_cast<int>(topLeft.x) - 2,
                                       static_cast<int>(topLeft.y) - 2,
                                       static_cast<int>(bottomRight.x - topLeft.x) + 4,
                                       static_cast<int>(bottomRight.y - topLeft.y) + 4),
                              cv::Scalar(0, 255, 0), 2);
                result.hxResult[0].x1 = topLeft.x;
                result.hxResult[0].y1 = topLeft.y;
                result.hxResult[0].x2 = bottomRight.x;
                result.hxResult[0].y2 = bottomRight.y;
            }
        }
    }

    if (hasPin && hasSolder) {
        HXResult& measuredSolder = result.hxResult[0];
        const float solderCenter = (measuredSolder.x1 + measuredSolder.x2) / 2.0f;
        const float pinCenter = (pin.x1 + pin.x2) / 2.0f + pinFix;
        measuredSolder.PixelPY = pinCenter - solderCenter;
        measuredSolder.center_pianyi =
            (pinCenter - measuredSolder.x1) /
            (measuredSolder.x2 - pinCenter + 1e-8f);
        if (measuredSolder.center_pianyi > params.max_PY ||
            measuredSolder.center_pianyi < params.min_PY)
            result.HX_NGReult |= kHxOffsetError;

        measuredSolder.Pixeldis = (std::max)(pin.YJ_H - 1.0f, 0.0f);
        measuredSolder.TBtoHX_distance =
            (measuredSolder.Pixeldis * pix2mmScaleY) / sourceScale;
        if (measuredSolder.TBtoHX_distance > params.max_TBDis)
            result.HX_NGReult |= kPinDistanceError;

        const int centerX = static_cast<int>(
            (measuredSolder.x1 + measuredSolder.x2) / 2.0f);
        const int centerY = static_cast<int>(measuredSolder.y2 + 20.0f);
        cv::line(image,
                 cv::Point(centerX + static_cast<int>(measuredSolder.PixelPY), centerY),
                 cv::Point(centerX + static_cast<int>(measuredSolder.PixelPY),
                           static_cast<int>(pin.y1 - 20.0f)),
                 cv::Scalar(221, 10, 221), 2);
        cv::line(image,
                 cv::Point(static_cast<int>(measuredSolder.x1 - 20.0f),
                           static_cast<int>(pin.y1)),
                 cv::Point(static_cast<int>(measuredSolder.x2 + 20.0f),
                           static_cast<int>(pin.y1)),
                 cv::Scalar(255, 255, 0), 2);
    }

    result.NO_HX_NGReults = primaryResultFromMask(result.HX_NGReult);
    result.NG_OKflag = result.NO_HX_NGReults != DetectionRsult::OK;
    result.dst = image;
    result.t_total = totalTimer.elapsed();
}

} // namespace SolderJointAlgorithm
