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
        createTables();
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

bool DbManager::addFace(int photoId, const QRect& rect,const QVector<float>& embed)
{
    QSqlQuery query;
    query.prepare("INSERT INTO faces (photo_id, x, y, w, h, embedding) "
                  "VALUES (:photoId, :x, :y, :w, :h, :emb)");
    query.bindValue(":photoId", photoId);
    query.bindValue(":x", rect.x());
    query.bindValue(":y", rect.y());
    query.bindValue(":w", rect.width());
    query.bindValue(":h", rect.height());

    // Convert QVector<float> to QByteArray
    QByteArray data(reinterpret_cast<const char*>(embed.constData()),
                    embed.size() * sizeof(float));
    query.bindValue(":emb", data);

    return query.exec();
}
