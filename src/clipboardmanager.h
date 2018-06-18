#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>
#include <QClipboard>
#include <QString>
#include <QDebug>
#include <QWidget>
#include <QTimer>

class ClipBoardManager : public QObject
{
    Q_OBJECT

public:
    explicit ClipBoardManager(QObject *parent = nullptr);
    ~ClipBoardManager();
    void exitThread() { emit quitThread(); }

private:
    QClipboard *clipboard;
    QString oldClipboardText, dwnldurl;

signals:
    void newLinkCopied(QString);
    void quitThread();

public slots:
    void checkClipboard();
};

#endif // CLIPBOARDMANAGER_H
