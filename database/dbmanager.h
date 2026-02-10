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
    int getPhotoId(const QString& path, const QString &connectionName=QSqlDatabase::defaultConnection);
private:
    QSqlDatabase m_db;
    bool createTables();

};

#endif
