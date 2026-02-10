#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <opencv2/objdetect.hpp>
#include <QString>
#include <QSize>
#include <QObject>
#include "dbmanager.h"

struct FaceResult {
    int photoId;
    QRect rect;
    QVector<float> embedding;
};
Q_DECLARE_METATYPE(FaceResult)

class FaceDetector : public QObject {
    Q_OBJECT
public:
    FaceDetector();
    bool loadModel(const QString& detectionModelPath, const QString &recModelPath,const QSize& inputSize = QSize(320,320));
    cv::Mat detect(const cv::Mat& image);
    void processImages(const QStringList& paths, DbManager* db);
    void stop() {m_abort = true;}
    void resetAbort(){m_abort =false;}
private:
    cv::Ptr<cv::FaceDetectorYN> detector;
    cv::Ptr<cv::FaceRecognizerSF> recognizer;
    float scoreThreshold = 0.9f;
    float nmsThreshold = 0.3f;
    int topK = 5000;
    std::atomic<bool> m_abort{false};
signals:
    void analyzeUpdater(int current,int total);
    void faceDetected(FaceResult result);
};

#endif
