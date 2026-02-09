#ifndef PHOTOSCANNER_H
#define PHOTOSCANNER_H

#include <QString>
#include <QStringList>
#include <QDirIterator>
#include <QObject>

class PhotoScanner : public QObject {
    Q_OBJECT
public:
    explicit PhotoScanner(QObject *parent = nullptr): QObject(parent) {}

    QStringList scanDirectory(const QString& path);
signals:
    void progessUpdated(int count, QString currentFile);
};

#endif
