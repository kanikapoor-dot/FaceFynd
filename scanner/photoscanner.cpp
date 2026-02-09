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
    int count = 0;

    while(it.hasNext())
    {
        QString file = it.next();
        foundImages << file;
        count++;
        if(count%10 == 0)
        {
            emit progessUpdated(count,file);
        }
    }
    return foundImages;
}
