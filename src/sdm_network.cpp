#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QByteArray>
#include "sdm_network.h"

SDM_network::SDM_network(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &SDM_network::setCheckSpeed);
    connect(&confManager, &QNetworkConfigurationManager::onlineStateChanged,
            [this] (bool status) { networkAvailable = status; networkStateChanged(); });
    networkAvailable = confManager.isOnline();

    ++totalObj;
    timer.start(300);
}

short SDM_network::totalObj = -1;

void SDM_network::setCheckSpeed()
{
    checkSpeed = true;
}

void SDM_network::networkStateChanged()
{
    if(!networkAvailable)
    {
        if(!paused)
            pause();
    }
    if(networkAvailable)
    {
        if(paused && fromNetwork)
            resume();
        if(_atStartup)
        {
            _atStartup = false;
            startNewDownload(downloadLink);
        }

    }
}

bool SDM_network::startNewDownload(const QUrl &url)
{
    QString fileName = getFileName(url);
    downloadingFileName = fileName;
    downloadLink = url;

    if(networkAvailable)
    {
        request.setUrl(url);
        mFile.setFileName(fileName);
        mFile.open(QIODevice::ReadWrite);
        beginNewDownload(request);
        emit downloadStarted(fileName);
        downloadSizeAtPause = 0;
        _atStartup = false;
        return true;
    }

    else
    {
        emit updateSpeed("waiting for network...");
        _atStartup = true;
        mFile.close();
        return false;
    }
}

void SDM_network::beginNewDownload(QNetworkRequest &request)
{
    if(networkAvailable)
    {
        currentReply = manager.get(request);
        connect(currentReply, &QNetworkReply::downloadProgress, this, &SDM_network::progress);
        connect(currentReply, &QNetworkReply::finished, this, &SDM_network::downloadFinished);
    }
    else
        emit updateSpeed("waiting for network...");
}

void SDM_network::pause()
{
    if(!paused)
    {
        qDebug()<<"pause";
        if(currentReply == nullptr)
            return;
        disconnect(currentReply, &QNetworkReply::downloadProgress, this, &SDM_network::progress);
        disconnect(currentReply, &QNetworkReply::finished, this, &SDM_network::downloadFinished);
        mFile.write(currentReply->readAll());
        qDebug()<<currentReply->readAll();
        currentReply->abort();
        currentReply = nullptr;
        paused = true;
        oncePaused = true;
        if(networkAvailable)
        {
            emit statusPaused("Paused");
            fromNetwork = false;
        }
        if(!networkAvailable)
        {
            emit statusPaused("waiting for network...");
            fromNetwork = true;
        }
    }
}

void SDM_network::resume()
{
    //bool __cc = !cancelled;
    if(paused && !cancelled)
    {
        qDebug()<<"resume";
        byteSaved = downloadSizeAtPause = mFile.size();
        QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(downloadSizeAtPause)+"-";
        request.setRawHeader("Range", rangeHeaderValue);
        beginNewDownload(request);
        paused = false;
    }
    if(cancelled)
    {
        cancelled = false;
        startNewDownload(downloadLink);
    }
}

void SDM_network::cancel()
{
    qDebug()<<"cancel";
    if(!_atStartup)
    {
        if(!paused)
        {
            disconnect(currentReply, &QNetworkReply::downloadProgress, this, &SDM_network::progress);
            disconnect(currentReply, &QNetworkReply::finished, this, &SDM_network::downloadFinished);
            currentReply->abort();
            mFile.close();
            mFile.remove();
            prev_bytes = rcvBytes = totalBytes = downloadSizeAtPause = 0;
            cancelled = true;
            emit updateSpeed("cancelled");
            emit updateprogressBarMax(100);
            emit updateprogressBarValue(0);
        }
        if(paused)
        {
            if(mFile.isOpen())
                mFile.close();
            if(mFile.exists())
                mFile.remove();
            cancelled = true;
            emit updateSpeed("cancelled");
            emit updateprogressBarMax(100);
            emit updateprogressBarValue(0);
        }
    }
}

void SDM_network::remove()
{
    qDebug()<<"remove";
    _atStartup = false;
    cancel();
    emit removeSDM(this);
}

QString SDM_network::getFileName(const QUrl &url)
{
    QString path = url.path();
    QString fileName = QFileInfo(path).fileName();

    if(fileName.isEmpty())
        fileName = "download";
    if(QFile::exists(downloadPath + fileName))
    {
        int i = 1;
        while(QFile::exists(downloadPath + QString::number(i) + fileName))
            i++;

        fileName = QString::number(i) + fileName;
    }

    emit setFileName(fileName);
    return downloadPath + fileName;
}

bool SDM_network::isHttpRedirected()
{
    int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

bool SDM_network::saveToDisk()
{
    mFile.write(currentReply->readAll());
    mFile.close();
    return true;
}

void SDM_network::progress(qint64 rcv_bytes, qint64 total_bytes)
{
    if(!firstTymOnly)
        total_bytes = totalBytes;
    if(oncePaused)
        rcv_bytes += byteSaved;
    QString rcvString;
    QString totalString;
    if(rcv_bytes < 1024) // byte
        rcvString = QString::number(rcv_bytes)+" bytes";
    if(rcv_bytes > 1024) // kb
    {
        qreal rcvreal = rcv_bytes/1024;
        if(rcvreal < 1024)
        {
            rcvString = QString::number(rcvreal);
            if(rcvString.contains("."))
            {
                rcvString = rcvString.mid(0,rcvString.indexOf(".")+2);
            }
            rcvString.append(" KB");
        }
        else // mb
        {
            rcvreal = rcvreal/1024;
            if(rcvreal < 1024) // gb
            {
                rcvString = QString::number(rcvreal);
                if(rcvString.contains("."))
                {
                    rcvString = rcvString.mid(0,rcvString.indexOf(".")+2);
                }
                rcvString.append(" MB");
            }
            else
            {
                rcvreal = rcvreal/1024;
                rcvString = QString::number(rcvreal);
                if(rcvString.contains("."))
                {
                    rcvString = rcvString.mid(0,rcvString.indexOf(".")+2);
                }
                rcvString.append(" GB");
            }
        }
    }
    if(total_bytes == -1)
        totalString = "NA";
    else
    {
        if(total_bytes < 1024) // byte
            totalString = QString::number(total_bytes)+" bytes";
        if(total_bytes > 1024) // kb
        {
            qreal treal = total_bytes/1024;
            if(treal < 1024)
            {
                totalString = QString::number(treal);
                if(totalString.contains("."))
                {
                    totalString = totalString.mid(0,totalString.indexOf(".")+2);
                }
                totalString.append(" KB");
            }
            else // mb
            {
                treal = treal/1024;
                if(treal < 1024)
                {
                    totalString = QString::number(treal);
                    if(totalString.contains("."))
                    {
                        totalString = totalString.mid(0,totalString.indexOf(".")+2);
                    }
                    totalString.append(" MB");
                }
                else
                    totalString = QString::number(treal/1024)+" GB";
            }
        }
    }

    QString status = rcvString+"/"+totalString;
    emit updateprogress(status);
    emit updateprogressBarValue(rcv_bytes/1024);
    emit updateprogressBarMax(total_bytes/1024);

    if(checkSpeed)
    {
        QString speed;
        speed.sprintf("%lld kb/s",(rcv_bytes-prev_bytes)/300);
        emit updateSpeed(speed);
        checkSpeed = false;
        prev_bytes = rcv_bytes;
    }

    if(!mFile.isOpen())
        mFile.open(QIODevice::ReadWrite);
    mFile.write(currentReply->readAll());
    if(firstTymOnly)
    {
        totalBytes = total_bytes;
        firstTymOnly = false;
    }
    rcvBytes = rcv_bytes;
}

void SDM_network::downloadFinished()
{
    QUrl url = currentReply->url();
    if(currentReply->error())
    {
        char cmd[] = "notify-send 'BAT-DownloadManager' 'download fail' '-t' 5000";
        qDebug()<<"Network error";
        emit updateDownloadStyle("color: red");
        emit updateSpeed("Failed");
        system(cmd);
        QSound::play(":/icons/soundEffect/fail.wav");
    }
    else
    {
        if(isHttpRedirected())
        {
            char cmd[] = "notify-send 'BAT-DownloadManager' 'request redirected' '-t' 5000";
            qDebug()<<"Request was redirected";
            emit updateDownloadStyle("color: red");
            emit updateSpeed("Failed");
            system(cmd);
            QSound::play(":/icons/soundEffect/fail.wav");
        }
        else
        {
            if(saveToDisk())
            {
                char cmd[] = "notify-send 'BAT-DownloadManager' 'download completed' '-t' 5000";
                qDebug()<<"Download completed";
                system(cmd);
                emit updateDownloadStyle("color: pink");
                emit updateSpeed("Completed");
                QSound::play(":/icons/soundEffect/success.wav");
            }
        }
    }

    currentReply->deleteLater();
    emit quitThread();
}

SDM_network::~SDM_network()
{
    if(mFile.isOpen())
        mFile.close();
    --totalObj;
    emit quitThread();
}
