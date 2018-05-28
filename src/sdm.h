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
#include "sdm_network.h"

class SDM : public QMainWindow
{
    Q_OBJECT

public:
    SDM(QWidget *parent=0);
    ~SDM();
    void resizeEvent(QResizeEvent *e = nullptr);
    void closeEvent(QCloseEvent *e);

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
    QVBoxLayout *mainVlayout, *VLAYOUT;
    QList <QLabel*> labelList;
    QList <QProgressBar*> progressbarList;
    QList <QWidget*> widgetList;
    QList <QVBoxLayout*> vboxList;
    QList <QLabel*> speedlabelList;
    QList <SDM_network*> sdmList;

    bool noTasks = true;
    bool isForceQuit = false;

private slots:
    void addNewTask();
    void resizeWidgets();
    void checkClipboard();
    void forceQuit();
    void freeMem();
};

#endif // SDM_H