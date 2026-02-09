#include "photoscanner.h"
#include <QImageReader>

QStringList PhotoScanner::scanDirectory(const QString &path)
{
    QStringList foundImages;

    //get all image formats support by this OS
    QStringList filters;
    for(const QByteArray &format : QImageReader::supportedImageFormats())
    {
        filters << "*." + QString(format);
    }

    QDirIterator it(path,filters,QDir::Files,QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        foundImages << it.next();
    }
    return foundImages;
}
