#include "facedetector.h"
#include <QDebug>
#include <QString>
#include <QRect>
#include <opencv2/imgcodecs.hpp>
#include <QtConcurrent>
#include <QMutex>
#include <QMutexLocker>

static QMutex aiMutex;

FaceDetector::FaceDetector() {}

bool FaceDetector::loadModel(const QString &detectionModelPath,const QString &recModelPath, const QSize &inputSize)
{
    try{
        detector = cv::FaceDetectorYN::create(
            detectionModelPath.toStdString(),
            "",
            cv::Size(inputSize.width(),inputSize.height()),
            scoreThreshold,
            nmsThreshold,
            topK
            );

        recognizer = cv::FaceRecognizerSF::create(recModelPath.toStdString(),"");

        return !detector.empty() && !recognizer.empty();
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

void FaceDetector::processImages(const QStringList &paths, DbManager *dbmanager)
{
    int total = paths.size();
    std::atomic<int> current{0};

    QtConcurrent::blockingMap(paths, [this, dbmanager, total, &current](const QString& path) {
        if (m_abort) return;

        if(detector.empty() || recognizer.empty())
        {
            qDebug() << "AI Models not initialized! Aborting thread.";
            return;
        }

        cv::Mat img = cv::imread(path.toStdString());
        if (img.empty())
        {
            current++;
            emit analyzeUpdater(current.load(),total);
            return;
        }

        // Yunet Detection
        cv::Mat faces;
        {
            QMutexLocker locker(&aiMutex);
            faces = detect(img);
        }

        // Use a temporary connection for the READ-ONLY photoId lookup
        QString threadConn = QString("Thread_Read_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
        int photoId = dbmanager->getPhotoId(path, threadConn);

        if (photoId != -1 && faces.rows > 0) {
            for (int i = 0; i < faces.rows; ++i) {
                // 2. AI Recognition (SFace)
                cv::Mat alignedFace, feature;
                {
                    QMutexLocker locker(&aiMutex);
                    recognizer->alignCrop(img, faces.row(i), alignedFace);
                    recognizer->feature(alignedFace, feature);
                }

                QVector<float> embedding;
                embedding.reserve(feature.cols);
                for(int j = 0; j < feature.cols; ++j) {
                    embedding.append(feature.at<float>(0, j));
                }

                // 3. Instead of saving here, we "Throw the order to the DB Worker"
                FaceResult res;
                res.photoId = photoId;
                res.rect = QRect(faces.at<float>(i,0), faces.at<float>(i,1),
                                 faces.at<float>(i,2), faces.at<float>(i,3));
                res.embedding = embedding;

                emit faceDetected(res);
            }
        }

        current++;
        emit analyzeUpdater(current.load(), total);
    });
}
