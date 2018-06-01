#ifndef SDM_NETWORK_H
#define SDM_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkConfigurationManager>
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
    short objectCount() const { return totalObj; }

private:
    QNetworkAccessManager manager;
    QNetworkConfigurationManager confManager;
    QNetworkReply *currentReply = nullptr;
    QNetworkRequest request;
    qint64 prev_bytes = 0, downloadSizeAtPause = 0;
    qint64 rcvBytes = 0, totalBytes = 0;
    qint64 byteSaved = 0;
    QString downloadingFileName;
    QFile mFile;
    QUrl downloadLink;
    short _index_ ;
    bool isHttpRedirected();
    void beginNewDownload(QNetworkRequest &request);
    bool saveToDisk();

    QTimer timer;
    const QString downloadPath = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]+"/";
    bool checkSpeed = false;
    bool paused = false;
    bool firstTymOnly = true;
    bool oncePaused = false;
    bool networkAvailable;
    bool fromNetwork = true;
    bool _atStartup = false;
    bool cancelled = false;

    static short totalObj;

signals:
    void downloadStarted(QString fileName);
    void updateprogress(QString txt);
    void updateSpeed(QString txt);
    void updateprogressBarValue(int);
    void updateprogressBarMax(int);
    void updateDownloadStyle(QString);
    void statusPaused(QString);
    void removeSDM(SDM_network*);

private slots:

    void downloadFinished();
    void progress(qint64 rcv_bytes, qint64 total_bytes);
    void setCheckSpeed();
    void networkStateChanged();

public slots:

    void pause();
    void resume();
    void cancel();
    void remove();
};

#endif // SDM_NETWORK_H
