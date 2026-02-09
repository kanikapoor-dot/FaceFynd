#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "photoscanner.h"
#include "dbmanager.h"
#include <opencv2/imgcodecs.hpp>

#include <QStandardPaths>
#include <QDebug>
#include <QSqlQuery>
#include <QtConcurrent>
#include <QFileDialog>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db(new DbManager())
    , facedetector(new FaceDetector)
{
    ui->setupUi(this);

    ui->txtSelectedPath->setStyleSheet("background-color: #f0f0f0; color: #333;");
    ui->progressBar->setRange(0,100);
    //ui->progressBar->setValue(0);

    //connect watcher
    connect(&m_watcher,&QFutureWatcher<QStringList>::finished,this, &MainWindow::onScanFinished);

    QString modelPath = "models/face_detection_yunet.onnx";
    if(facedetector->loadModel(modelPath))
    {
        qDebug() << "AI Engine: YuNet Loaded successfully.";
    } else {
        qDebug() << "AI Engine: Failed to load model. Check path:" << modelPath;
    }
}

void MainWindow::on_btnSelectFolder_clicked()
{
    //open directory
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                    tr("Select Folder"),
                                    QDir::homePath(),
                                    QFileDialog::ShowDirsOnly);

    if(selectedDir.isEmpty()) return;

    ui->txtSelectedPath->setText(selectedDir);

    this->setFocus();

    ui->btnSelectFolder->setEnabled(false);
    ui->progressBar->setRange(0,0);
    ui->statusbar->showMessage("Preparing to scan: " + selectedDir);

    PhotoScanner* scanner = new PhotoScanner(this);

    connect(scanner,&PhotoScanner::progessUpdated,this,[this](int count){
        ui->statusbar->showMessage(tr("Scanning... Found %1 images").arg(count));
        ui->progressBar->setRange(0,0);
    });

    QFuture<QStringList> future = QtConcurrent::run([scanner,selectedDir](){
        return scanner->scanDirectory(selectedDir);
    });

    m_watcher.setFuture(future);
}

void MainWindow::onScanFinished()
{
    QStringList foundImages = m_watcher.result();

    if(foundImages.isEmpty())
    {
        ui->statusbar->showMessage("Scan finished. No images found.");
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->btnSelectFolder->setEnabled(true);
        return;
    }

    if(!db->saveScannedPaths(foundImages))
    {
        ui->statusbar->showMessage("Database error: Could not save paths.");
        ui->btnSelectFolder->setEnabled(true);
        return;
    }

    facedetector->resetAbort();

    ui->statusbar->showMessage(tr("Files indexed. Analyzing faces for %1 images...").arg(foundImages.size()));
    ui->progressBar->setRange(0, 0); // Pulse mode for AI work
    ui->btnSelectFolder->setEnabled(false);

    connect(facedetector,&FaceDetector::analyzeUpdater,this, [this](int current, int total) {
        ui->progressBar->setRange(0, total);
        ui->progressBar->setValue(current);
        ui->statusbar->showMessage(tr("Analyzing faces: %1 of %2 images processed...")
                                       .arg(current).arg(total));
    }, Qt::QueuedConnection);

    QFuture<void> aiFuture =  QtConcurrent::run([this,foundImages](){
        facedetector->processImages(foundImages,db);
    });

    connect(&m_aiWatcher,&QFutureWatcher<void>::finished,this,[this](){
        ui->statusbar->showMessage("Face analysis complete! All images processed.");
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(100);
        ui->btnSelectFolder->setEnabled(true);
        m_aiWatcher.disconnect();
    });

    m_aiWatcher.setFuture(aiFuture);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_watcher.isRunning())
    {
        m_watcher.cancel();
        m_watcher.waitForFinished();
    }

    if(m_aiWatcher.isRunning())
    {
        ui->statusbar->showMessage("Stopping AI and saving data... please wait.");
        facedetector->stop();
        qApp->processEvents();
        m_aiWatcher.waitForFinished();
    }
    event->accept();
}

MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}

