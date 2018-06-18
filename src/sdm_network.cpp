#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QByteArray>
#include <QVariant>
#include "sdm_network.h"

SDM_network::SDM_network(QObject *parent) : QObject(parent)
{
    connect(&timer, &QTimer::timeout, [this](){checkSpeed = true;});
    connect(&confManager, &QNetworkConfigurationManager::onlineStateChanged,
            [this] (bool status) { networkAvailable = status; networkStateChanged(); });
    networkAvailable = confManager.isOnline();

    ++totalObj;
    successSound = new QSound(":/resources/success.wav");
    failSound = new QSound(":/resources/fail.wav");
    timer.start(100);
}

short SDM_network::totalObj = 0;
short SDM_network::totalDownloads()
{
    return totalObj;
}

void SDM_network::networkStateChanged()
{
    if(!networkAvailable)
    {
        if(!paused)
            pause();
        qDebug()<<"Network unavailable";
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
        qDebug()<<"Network available";
    }
}

bool SDM_network::startNewDownload(const QUrl &url)
{
    QString fileName = getFileName(url);

    if(networkAvailable)
    {
        request.setUrl(url);
        mFile.setFileName(fileName);
        mFile.open(QIODevice::ReadWrite);
        beginNewDownload(request);
        emit downloadStarted(fileName);
        downloadSizeAtPause = 0;
        _atStartup = false;
        isRestore = false;
        return true;
    }

    else
    {
        emit updateSpeed("waiting for network...");
        qDebug()<<"waiting for network";
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
        currentReply->ignoreSslErrors();
        connect(currentReply, &QNetworkReply::downloadProgress, this, &SDM_network::progress);
        connect(currentReply, &QNetworkReply::finished, this, &SDM_network::downloadFinished);
        connect(currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
        this, &SDM_network::errorOccur);
    }
    else
    {
        emit updateSpeed("waiting for network...");
        qDebug()<<"waiting for network";
    }

   isRestore = false;
}

void SDM_network::pause()
{
    if(!thereIsError)
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
                qDebug()<<"waiting for network";
                fromNetwork = true;
            }
            running = false;
        }
    }
}

void SDM_network::resume()
{
    if(paused && !cancelled)
    {
        qDebug()<<"resume";
        byteSaved = downloadSizeAtPause = mFile.size();
        QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(downloadSizeAtPause)+"-";
        request.setRawHeader("Range", rangeHeaderValue);
        beginNewDownload(request);
        paused = false;
        isRestore = false;
    }
    if(cancelled)
    {
        cancelled = false;
        startNewDownload(downloadLink);
    }

    thereIsError = false;
}

void SDM_network::cancel()
{
    qDebug()<<"cancel";
    if(!thereIsError)
    {
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
                running = false;
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
                running = false;
                emit updateSpeed("cancelled");
                emit updateprogressBarMax(100);
                emit updateprogressBarValue(0);
            }
        }
    }

    removeInfoFile();
}

void SDM_network::remove()
{
    qDebug()<<"remove SDM";
    _atStartup = false;
    fromRemove = true;
    cancel();
    removeInfoFile();
    emit quitThread();
    emit removeSDM();
}

void SDM_network::setSavedByte(qint64 sb, qint64 tb, QString link, QString fileName)
{
    QUrl url(link);
    byteSaved = sb;
    emit updateprogressBarValue(sb/1024);
    emit updateprogressBarMax(tb/1024);
    emit setFileName(fileName);
    downloadingFileName = fileName;
    downloadLink = url;
    downloadLinkString = link;
    request.setUrl(url);
    mFile.setFileName(downloadPath+fileName);
    if(!mFile.isOpen())
        mFile.open(QIODevice::ReadWrite);
    isRestore = true;
    qDebug()<<"restoring...";
}

void SDM_network::errorOccur(QNetworkReply::NetworkError code)
{
    qDebug()<<"DOWNLOAD ERROR...";
    qDebug()<<code;
    thereIsError = true;
    running = false;
}

QString SDM_network::getFileName(const QUrl &url)
{
    QString path = url.path();
    QString fileName = QFileInfo(path).fileName();
    emit setFileName(fileName);

    if(fileName.isEmpty())
        fileName = "download";
    if(QFile::exists(downloadPath + fileName))
    {
        int i = 1;
        while(QFile::exists(downloadPath + QString::number(i) + fileName))
            i++;

        fileName = QString::number(i) + fileName;
    }

    downloadingFileName = fileName;
    downloadLink = url;
    downloadLinkString = url.toString();

    qDebug()<<"downloding file name- "<<fileName;
    return downloadPath + fileName;
}

bool SDM_network::isHttpRedirected()
{
    qDebug()<<"checking redirection...";
    int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool redirected = (statusCode == 301 || statusCode == 302 || statusCode == 303
                       || statusCode == 305 || statusCode == 307 || statusCode == 308);

    if(redirected)
        qDebug()<<"link Redirected";
    else
        qDebug()<<"link not Redirected";

    //QVariant rurl = currentReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    //qDebug()<<rurl.toString();

    return redirected;
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
    QString gSpeed;

    emit updateprogress(status);
    emit updateprogressBarValue(rcv_bytes/1024);
    emit updateprogressBarMax(total_bytes/1024);
    //emit saveInfo(total_bytes, rcv_bytes, downloadLinkString);

    if(checkSpeed)
    {
        QString speed;
        speed.sprintf("%lld kb/s",(rcv_bytes-prev_bytes)/300);
        emit updateSpeed(speed);
        checkSpeed = false;
        prev_bytes = rcv_bytes;
        gSpeed = speed;
    }

    if(!mFile.isOpen())
    {
        mFile.setFileName(downloadingFileName);
        mFile.open(QIODevice::ReadWrite);
    }
    mFile.write(currentReply->readAll());
    if(firstTymOnly)
    {
        totalBytes = total_bytes;
        firstTymOnly = false;
    }
    rcvBytes = rcv_bytes;
    if(rcv_bytes > rcvBytes)
    {
        thereIsError = false;
        running = false;
    }
    else
        running = true;

    qDebug()<<"file: "<<downloadingFileName;
    qDebug()<<"progress: "<<status<<" **** "<<"speed: "<<gSpeed;
}

void SDM_network::saveInfoToDisk()
{
    if(!fromRemove && !completed)
    {
        sFile.setFileName(downloadPath+downloadingFileName+".inf");
        if(sFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QTextStream stream(&sFile);

            stream << "DO NOT MODIFY THE CONTENTS OF THIS FILE" << endl;
            stream << downloadLinkString << endl;
            stream << downloadingFileName << endl;
            stream << totalBytes << endl;
            stream << rcvBytes;
            sFile.close();
            qDebug()<<"link= "<<downloadLinkString;
            qDebug()<<"file name= "<<downloadingFileName;
            qDebug()<<"total bytes= "<<totalBytes;
            qDebug()<<"rcv bytes= "<<rcvBytes;
            qDebug()<<"Info Saved...";
        }
        else
            qDebug()<<"Info save failed...";
    }
}

void SDM_network::removeInfoFile()
{
    if(sFile.isOpen())
        sFile.close();
    sFile.setFileName(downloadingFileName+".inf");
    sFile.remove();
}

void SDM_network::downloadFinished()
{
    if(currentReply->error())
    {
        char cmd[] = "notify-send 'BAT-DownloadManager' 'download fail' '-t' 5000";
        qDebug()<<"Network error (failed)";
        emit updateDownloadStyle("color: red");
        emit updateSpeed("Failed");
        thereIsError = true;
        system(cmd);
        failSound->play();
    }
    else
    {
        if(isHttpRedirected())
        {
            char cmd[] = "notify-send 'BAT-DownloadManager' 'request redirected' '-t' 5000";
            qDebug()<<"Request was redirected (failed)";
            emit updateDownloadStyle("color: red");
            emit updateSpeed("Failed");
            thereIsError = true;
            system(cmd);
            failSound->play();
        }
        else
        {
            if(saveToDisk())
            {
                char cmd[] = "notify-send 'BAT-DownloadManager' 'download completed' '-t' 5000";
                qDebug()<<"Download completed";
                qDebug()<<"saved location: "<<downloadPath+downloadingFileName;
                system(cmd);
                emit updateDownloadStyle("color: pink");
                emit updateSpeed("Completed");
                successSound->play();
                thereIsError = false;
                completed = true;
            }
        }
    }

    currentReply->deleteLater();
    if(sFile.isOpen())
        sFile.close();
    sFile.setFileName(downloadingFileName+".inf");
    sFile.remove();
    running = false;
    emit quitThread();
}

bool SDM_network::saveToDisk()
{
    mFile.write(currentReply->readAll());
    mFile.close();
    qDebug()<<"All data written";
    return true;
}

SDM_network::~SDM_network()
{
    if(!thereIsError)
        saveInfoToDisk();
    if(mFile.isOpen())
        mFile.close();
    if(sFile.isOpen())
        sFile.close();
    --totalObj;
    emit quitThread();
    qDebug()<<"SDM_Network destructor";
}
