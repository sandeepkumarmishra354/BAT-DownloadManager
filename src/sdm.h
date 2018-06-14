#ifndef SDM_H
#define SDM_H

#include <QMainWindow>
#include <QWidget>
#include <QDebug>
#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QProgressBar>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QScrollArea>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QClipboard>
#include <QTimer>
#include <QPixmap>
#include <QPoint>
#include <QThread>
#include "sdm_network.h"
#include "help.h"
#include "setting.h"

class SDM : public QMainWindow
{
    Q_OBJECT

public:
    SDM(QWidget *parent=0);
    ~SDM();

private:
    QWidget mainWidget, MAIN_WIDGET;
    QScrollArea scrollArea;
    QString dwnldurl = "";
    QString oldClipboardText = "";
    QClipboard *clipboard;
    QTimer *timer;
    QLabel appIconLabel, appNameLabel;
    QPixmap appIcon;
    QVBoxLayout appIconLayout;
    QWidget *appIconWidget;
    QToolBar *tool_bar;
    QAction *addTask, *quitDM;
    QAction *helpAction, *settingAction;
    QWidget *spacerWidget;
    QVBoxLayout *mainVlayout, *VLAYOUT;

    Help *_help = new Help(this);
    Setting *_setting = new Setting(this);

    QList <QWidget*> widgetList;
    QList <SDM_network*> sdmList;
    QList <QThread*> threadList;

    const QString downloadPath = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]+"/";

    bool noTasks = true;
    bool isForceQuit = false;
    bool isRestore = false;

    void resizeEvent(QResizeEvent *e = nullptr);
    void closeEvent(QCloseEvent *e);
    void createAction();
    void createDownloadWidgets(QString urlText = "");
    void loadTasks();
    void restoreTasks();
    void addToContainer(QStringList dataList);

    struct taskInfo
    {
        taskInfo() { }
        ~taskInfo()
        {
            if(next != nullptr)
                delete next;
        }
        QString link, fileName;
        qint64 totalByte, rcvByte;
        taskInfo *next = nullptr;
    };

    taskInfo *_tInfo = nullptr;

    qint64 maxValue, minValue;
    QString _link_, _fileName;

private slots:
    void addNewTask();
    void resizeWidgets();
    void checkClipboard();
    void forceQuit();
    void freeMem();
    void removeSDM();
};

#endif // SDM_H
