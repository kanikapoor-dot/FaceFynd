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

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(100);

    if(!foundImages.isEmpty())
    {
        if(db->saveScannedPaths(foundImages))
        {
           ui->statusbar->showMessage(tr("Index Complete: %1 images saved.").arg(foundImages.size()));
        } else
        {
            ui->statusbar->showMessage("Database error: Could not save paths.");
        }
    } else
    {
        ui->statusbar->showMessage("Scan finished. No images found");
        ui->progressBar->setValue(0);
    }

    ui->btnSelectFolder->setEnabled(true);
}



MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}
