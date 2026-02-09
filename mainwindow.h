#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>
#include "dbmanager.h"
#include "facedetector.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DbManager *db;
    QFutureWatcher<QStringList> m_watcher; //watches img background scan
    void onScanFinished();
    FaceDetector *facedetector;
    QFutureWatcher<void> m_aiWatcher;

private slots:
    void on_btnSelectFolder_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
