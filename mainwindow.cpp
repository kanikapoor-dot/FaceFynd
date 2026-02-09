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

    ui->progressBar->setRange(0,0);
    ui->progressBar->setValue(0);

    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    ui->statusbar->showMessage("Preparing to scan" + picturesPath);

    PhotoScanner* scanner = new PhotoScanner(this);

    connect(scanner,&PhotoScanner::progessUpdated,this,[this](int count, QString file){
        ui->statusbar->showMessage(tr("Scanning... Found %1 images").arg(count));
        ui->progressBar->setRange(0,0);
    });

    //connect watcher
    connect(&m_watcher,&QFutureWatcher<QStringList>::finished,this, &MainWindow::onScanFinished);

    QFuture<QStringList> future = QtConcurrent::run([scanner,picturesPath](){
        return scanner->scanDirectory(picturesPath);
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
}

MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}
