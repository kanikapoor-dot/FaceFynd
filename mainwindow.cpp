#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "photoscanner.h"
#include "dbmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QSqlQuery>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), db(new DbManager())
{
    ui->setupUi(this);

    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    //connect watcher
    connect(&m_watcher,&QFutureWatcher<QStringList>::finished,this, &MainWindow::onScanFinished);

    qDebug() << "Starting background scan...";
    QFuture<QStringList> future = QtConcurrent::run(&PhotoScanner::scanDirectory,picturesPath);
    m_watcher.setFuture(future);
    ui->statusbar->showMessage("Scanning folders in backgound...");

}

void MainWindow::onScanFinished()
{
    QStringList foundImages = m_watcher.result();

    if(!foundImages.isEmpty())
    {
        if(db->saveScannedPaths(foundImages))
        {
            ui->statusbar->showMessage("Scan Complete. Found images no :"+QString::number(foundImages.size()));
            qDebug() << "Successfully Indexed. Total Images No : " << foundImages.size();
        } else
        {
            qDebug() << "Failed to save paths to database";
        }
    }
}

MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}
