#ifndef PHOTOSCANNER_H
#define PHOTOSCANNER_H

#include <QString>
#include <QStringList>
#include <QDirIterator>

class PhotoScanner {
public:
    static QStringList scanDirectory(const QString& path);
};

#endif
