#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QVector>
#include <QRect>
#include <QStringList>

class DbManager {
public:
    DbManager(const QString& path = "facefynd.db");
    ~DbManager();
    bool saveScannedPaths(const QStringList& paths);
    bool addPhoto(const QString& path,int& photoId);
    bool addFace(int photoId, const QRect& rect,const QVector<float>& embed);
private:
    QSqlDatabase m_db;
    bool createTables();

};

#endif
