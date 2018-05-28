#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "sdm_network.h"

SDM_network::SDM_network(QObject *parent) : QObject(parent)
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

bool SDM_network::startNewDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progress(qint64,qint64)));

    QString fileName = getFileName(url);
    downloadingFileName = fileName;
    emit downloadStarted(fileName);
    return true;
}

QString SDM_network::getFileName(const QUrl &url)
{
    QString path = url.path();
    QString fileName = QFileInfo(path).fileName();

    if(fileName.isEmpty())
        fileName = "download";
    if(QFile::exists("/root/Downloads/" + fileName))
    {
        int i = 1;
        while(QFile::exists("/root/Downloads/" + QString::number(i) + fileName))
            i++;

        fileName = QString::number(i) + fileName;
    }

    return "/root/Downloads/" + fileName;
}

bool SDM_network::isHttpRedirected(QNetworkReply *reply)
{
    if(reply != nullptr)
    {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        return statusCode == 301 || statusCode == 302 || statusCode == 303
                   || statusCode == 305 || statusCode == 307 || statusCode == 308;
    }
    return false;
}

bool SDM_network::saveToDisk(const QString &fileName, QIODevice *data)
{
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"file write fail...";
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void SDM_network::progress(qint64 rcv_bytes, qint64 total_bytes)
{
    QString rcvString;
    QString totalString;
    if(rcv_bytes < 1024) // byte
        rcvString = QString::number(rcv_bytes)+" bytes";
    if(rcv_bytes > 1024) // kb
    {
        qreal rcvint = rcv_bytes/1024;
        if(rcvint < 1024)
            rcvString = QString::number(rcvint)+" KB";
        else // mb
        {
            rcvint = rcvint/1024;
            if(rcvint < 1024) // gb
                rcvString = QString::number(rcvint)+" MB";
            else
                rcvString = QString::number(rcvint/1024)+" GB";
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
            qreal tint = total_bytes/1024;
            if(tint < 1024)
                totalString = QString::number(tint)+" KB";
            else // mb
            {
                tint = tint/1024;
                if(tint < 1024)
                    totalString = QString::number(tint)+" MB";
                else
                    totalString = QString::number(tint/1024)+" GB";
            }
        }
    }

    QString status;
    status.sprintf("       %lld kb/s",((rcv_bytes-prev_bytes)/8)/8);
    status.prepend(rcvString+"/"+totalString);
    emit updateprogress(status);
    emit updateprogressBarValue(rcv_bytes/1024);
    emit updateprogressBarMax(total_bytes/1024);
    prev_bytes = rcv_bytes;
}

void SDM_network::downloadFinished(QNetworkReply *ntReply)
{
    QUrl url = ntReply->url();
    if(ntReply->error())
    {
        char cmd[] = "notify-send 'BAT-DM' 'download fail' '-t' 5000";
        qDebug()<<"Network error";
        system(cmd);
    }
    else
    {
        if(isHttpRedirected(ntReply))
        {
            char cmd[] = "notify-send 'BAT-DM' 'request redirected' '-t' 5000";
            qDebug()<<"Request was redirected";
            system(cmd);
        }
        else
        {
            QString fileName = getFileName(url);
            if(saveToDisk(fileName, ntReply))
            {
                char cmd[] = "notify-send 'BAT-DM' 'download completed' '-t' 5000";
                qDebug()<<"Download completed";
                system(cmd);
            }
        }
    }

    ntReply->deleteLater();
}
