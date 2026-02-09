#include "facedetector.h"
#include <QDebug>
#include <QString>
#include <QRect>
#include <opencv2/imgcodecs.hpp>

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

void FaceDetector::processImages(const QStringList &paths, DbManager *dbmanager)
{
    int total = paths.size();
    int current = 0;
    QString threadConn = "AI_FaceDetect_Conn";
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",threadConn);
        db.setDatabaseName("facefynd.db");
        if(!db.open()) return;
        db.transaction();
        for(const QString& path : paths)
        {
            if(m_abort) break;
            current++;
            emit analyzeUpdater(current,total);
            cv::Mat img = cv::imread(path.toStdString());
            if(img.empty()) continue;

            cv::Mat faces = detect(img);

            int photoId = dbmanager->getPhotoId(path,threadConn);
            if(photoId == -1) continue;
            for(int i = 0;i < faces.rows; ++i)
            {
                int x = static_cast<int>(faces.at<float>(i,0));
                int y = static_cast<int>(faces.at<float>(i,1));
                int w = static_cast<int>(faces.at<float>(i,2));
                int h = static_cast<int>(faces.at<float>(i,3));

                QRect faceRect(x,y,w,h);

                QVector<float> emptyEmbed;
                dbmanager->addFace(photoId,faceRect,emptyEmbed,threadConn);
            }
        }
        db.commit();
    }
    QSqlDatabase::removeDatabase(threadConn);
}
