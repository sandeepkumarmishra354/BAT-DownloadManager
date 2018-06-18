#include <QApplication>
#include "clipboardmanager.h"

ClipBoardManager::ClipBoardManager(QObject *parent) : QObject(parent)
{
    clipboard = QApplication::clipboard();
    oldClipboardText = clipboard->text();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipBoardManager::checkClipboard);
    qDebug()<<"Clipboard constructor";
}

void ClipBoardManager::checkClipboard()
{
    dwnldurl = clipboard->text();
    if(!dwnldurl.isEmpty())
    {
        // if dwnldurl contains any of these then there is chance that dwnldurl is a link
        if(dwnldurl.contains("www",Qt::CaseInsensitive)||dwnldurl.contains("http",Qt::CaseInsensitive)||
           dwnldurl.contains(".com",Qt::CaseInsensitive)||dwnldurl.contains(".in",Qt::CaseInsensitive)||
           dwnldurl.contains(".us",Qt::CaseInsensitive)||dwnldurl.contains(".uk",Qt::CaseInsensitive))
        {
            qDebug()<<"new link copied into clipboard";
            qDebug()<<"link: "<<dwnldurl;
            oldClipboardText = dwnldurl;
            emit newLinkCopied(dwnldurl);
        }
    }
}

ClipBoardManager::~ClipBoardManager()
{
    qDebug()<<"clipboard destructor";
    emit quitThread();
}
