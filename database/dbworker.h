#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "facedetector.h"

class DbWorker : public QObject
{
    Q_OBJECT
public:
    explicit DbWorker(const QString &dbPath) : m_dbPath(dbPath) {}

public slots:
    void onSaveFace(FaceResult result)
    {
        if(!m_db.isOpen())
        {
            m_db = QSqlDatabase::addDatabase("QSQLITE","PersistanceThread");
            m_db.setDatabaseName(m_dbPath);
            if(!m_db.open())
            {
                qDebug() << "DB Worker Error: Could not open database" << m_db.lastError().text();
                return;
            }
            QSqlQuery pragmaQuery(m_db);
            pragmaQuery.exec("PRAGMA journal_mode=WAL;");
            pragmaQuery.exec("PRAGMA synchronous=NORMAL;");
            pragmaQuery.exec("PRAGMA busy_timeout=5000;");
        }

        QSqlQuery query(m_db);
        query.prepare("INSERT INTO faces (photo_id, x, y, w, h, embedding) "
                      "VALUES (:pid, :x, :y, :w, :h, :emb)");
        query.bindValue(":pid", result.photoId);
        query.bindValue(":x", result.rect.x());
        query.bindValue(":y", result.rect.y());
        query.bindValue(":w", result.rect.width());
        query.bindValue(":h", result.rect.height());

        QByteArray data(reinterpret_cast<const char*>(result.embedding.constData()),
                        result.embedding.size() * sizeof(float));
        query.bindValue(":emb", data);
        if(!query.exec()) {
            qDebug() << "DB Worker Insert Error:" << query.lastError().text();
        }
    }

private:
    QString m_dbPath;
    QSqlDatabase m_db;
};

