#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QByteArray>
#include <QDebug>
#include <QRect>

DbManager::DbManager(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if(!m_db.open())
    {
        qDebug() << "Error: Connection with database failed" << m_db.lastError();
    } else
    {
        QSqlQuery query(m_db);
        query.exec("PRAGMA journal_mode=WAL;");
        query.exec("PRAGMA synchronous=NORMAL;");
        createTables();
    }
}

DbManager::~DbManager()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
}

bool DbManager::createTables()
{
    QSqlQuery query;
    bool success = true;

    // 1. Photos table (Must be first)
    if (!query.exec("CREATE TABLE IF NOT EXISTS photos ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "path TEXT UNIQUE)")) {
        qDebug() << "Photos table error:" << query.lastError().text();
        success = false;
    }

    // 2. People table
    if (!query.exec("CREATE TABLE IF NOT EXISTS people ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "name TEXT DEFAULT 'Unknown')")) {
        qDebug() << "People table error:" << query.lastError().text();
        success = false;
    }

    // 3. Faces table (Depends on 1 and 2)
    if (!query.exec("CREATE TABLE IF NOT EXISTS faces ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "photo_id INTEGER, "
                    "person_id INTEGER, "
                    "x INTEGER, y INTEGER, w INTEGER, h INTEGER, "
                    "embedding BLOB, "
                    "FOREIGN KEY(photo_id) REFERENCES photos(id) ON DELETE CASCADE, "
                    "FOREIGN KEY(person_id) REFERENCES people(id))")) {
        qDebug() << "Faces table error:" << query.lastError().text();
        success = false;
    }

    return success;
}

bool DbManager::saveScannedPaths(const QStringList &paths)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO photos (path) VALUES (:path)");
    for(const QString& path : paths)
    {
        query.bindValue(":path",path);
        query.exec();
    }

    return QSqlDatabase::database().commit();
}

int DbManager::getPhotoId(const QString &path, const QString &connectionName)
{
    if(!QSqlDatabase::contains(connectionName))
    {
        QSqlDatabase threadDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        threadDb.setDatabaseName(m_db.databaseName());
        if(!threadDb.open()) return -1;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);
    query.prepare("SELECT id FROM photos WHERE path = :path");
    query.bindValue(":path",path);

    if(query.exec() && query.next())
    {
        return query.value(0).toInt();
    }
    return -1;
}
