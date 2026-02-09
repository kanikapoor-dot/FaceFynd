#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "photoscanner.h"
#include "dbmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = new DbManager();

    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QStringList foundImages = PhotoScanner::scanDirectory(picturesPath);

    if(!foundImages.isEmpty())
    {
        if(db->saveScannedPaths(foundImages))
        {
            qDebug() << "Successfully Indexed. Total Images No : " << foundImages.size();
        } else
        {
            qDebug() << "Failed to save paths to database";
        }

        //Verification Query
        QSqlQuery query("SELECT COUNT(*) FROM photos");
        if(query.next())
        {
            qDebug() << "Total unique photos in facefynd database:" << query.value(0).toInt();
        }
    }

}

MainWindow::~MainWindow()
{
    delete db;
    delete ui;
}
