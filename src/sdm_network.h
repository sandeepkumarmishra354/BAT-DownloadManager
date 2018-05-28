#ifndef SDM_NETWORK_H
#define SDM_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QIODevice>
#include <QFile>
#include <QUrl>

class SDM_network : public QObject
{
    Q_OBJECT
public:
    explicit SDM_network(QObject *parent = nullptr);
    bool startNewDownload(const QUrl &url);
    QString getFileName(const QUrl &url);

private:
    QNetworkAccessManager manager;
    qint64 prev_bytes = 0;
    QString downloadingFileName;
    bool isHttpRedirected(QNetworkReply *reply = nullptr);
    bool saveToDisk(const QString &fileName, QIODevice *data);

signals:
    void downloadStarted(QString fileName);
    void updateprogress(QString txt);
    void updateprogressBarValue(int);
    void updateprogressBarMax(int);
private slots:
    void downloadFinished(QNetworkReply *ntReply = nullptr);
    void progress(qint64 rcv_bytes, qint64 total_bytes);
};

#endif // SDM_NETWORK_H
