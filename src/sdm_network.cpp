#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QByteArray>
#include "sdm_network.h"

SDM_network::SDM_network(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, this, &SDM_network::setCheckSpeed);
}

void SDM_network::setCheckSpeed()
{
    checkSpeed = true;
}

bool SDM_network::startNewDownload(const QUrl &url)
{
    request.setUrl(url);
    QString fileName = getFileName(url);
    mFile.setFileName(fileName);
    mFile.open(QIODevice::ReadWrite);
    beginNewDownload(request);
    downloadingFileName = fileName;
    downloadLink = url;
    emit downloadStarted(fileName);
    timer.start(300);
    downloadSizeAtPause = 0;
    return true;
}

void SDM_network::beginNewDownload(QNetworkRequest &request)
{
    currentReply = manager.get(request);
    connect(currentReply, &QNetworkReply::downloadProgress, this, &SDM_network::progress);
    connect(currentReply, &QNetworkReply::finished, this, &SDM_network::downloadFinished);
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
        emit statusPaused("Paused");
    }
}

void SDM_network::resume()
{
    if(paused)
    {
        qDebug()<<"resume";
        byteSaved = downloadSizeAtPause = mFile.size();
        QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(downloadSizeAtPause)+"-";
        request.setRawHeader("Range", rangeHeaderValue);
        beginNewDownload(request);
        paused = false;
    }
}

void SDM_network::cancel()
{
    qDebug()<<"cancel";
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
}

SDM_network::~SDM_network()
{
    if(mFile.isOpen())
        mFile.close();
}
