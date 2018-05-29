#ifndef SDM_NETWORK_H
#define SDM_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QIODevice>
#include <QFile>
#include <QTimer>
#include <QUrl>
#include <QSound>
#include <QStandardPaths>

class SDM_network : public QObject
{
    Q_OBJECT

public:
    explicit SDM_network(QObject *parent = nullptr);
    ~SDM_network();
    bool startNewDownload(const QUrl &url);
    QString getFileName(const QUrl &url);
    QString getFile() const { return downloadingFileName; }

private:
    QNetworkAccessManager manager;
    QNetworkReply *currentReply = nullptr;
    QNetworkRequest request;
    qint64 prev_bytes = 0, downloadSizeAtPause = 0;
    qint64 rcvBytes = 0, totalBytes = 0;
    qint64 byteSaved = 0;
    QString downloadingFileName;
    QFile mFile;
    QUrl downloadLink;
    bool isHttpRedirected();
    void beginNewDownload(QNetworkRequest &request);
    bool saveToDisk();

    QTimer timer;
    const QString downloadPath = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]+"/";
    bool checkSpeed = false;
    bool paused = false;
    bool firstTymOnly = true;
    bool oncePaused = false;

signals:
    void downloadStarted(QString fileName);
    void updateprogress(QString txt);
    void updateSpeed(QString txt);
    void updateprogressBarValue(int);
    void updateprogressBarMax(int);
    void updateDownloadStyle(QString);
    void statusPaused(QString);

private slots:

    void downloadFinished();
    void progress(qint64 rcv_bytes, qint64 total_bytes);
    void setCheckSpeed();

public slots:

    void pause();
    void resume();
    void cancel();
};

#endif // SDM_NETWORK_H
