#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <opencv2/objdetect.hpp>
#include <QString>
#include <QSize>

class FaceDetector {
public:
    FaceDetector();
    bool loadModel(const QString& modelPath, const QSize& inputSize = QSize(320,320));
    cv::Mat detect(const cv::Mat& image);
private:
    cv::Ptr<cv::FaceDetectorYN> detector;
    float scoreThreshold = 0.9f;
    float nmsThreshold = 0.3f;
    int topK = 5000;
};

#endif
