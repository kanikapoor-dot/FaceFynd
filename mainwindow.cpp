#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "photoscanner.h"
#include <QStandardPaths>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QStringList images = PhotoScanner::scanDirectory(picturesPath);
    qDebug() << "Scanning path: " << picturesPath;
    qDebug() << "found" << images.size() << "Images.";
    if(!images.isEmpty())
    {
        qDebug() << "First image found" << images.first();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
