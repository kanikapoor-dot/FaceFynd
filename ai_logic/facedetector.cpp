#include "facedetector.h"
#include <QDebug>

FaceDetector::FaceDetector() {}

bool FaceDetector::loadModel(const QString &modelPath, const QSize &inputSize)
{
    try{
        detector = cv::FaceDetectorYN::create(
            modelPath.toStdString(),
            "",
            cv::Size(inputSize.width(),inputSize.height()),
            scoreThreshold,
            nmsThreshold,
            topK
        );

        return !detector.empty();
    } catch(const cv::Exception& e)
    {
        qDebug() << "OpenCV Error:" << e.what();
        return false;
    }
}

cv::Mat FaceDetector::detect(const cv::Mat &image)
{
    if(detector.empty() || image.empty()) return cv::Mat();
    detector->setInputSize(image.size());
    cv::Mat faces;
    detector->detect(image,faces);
    return faces;
}
