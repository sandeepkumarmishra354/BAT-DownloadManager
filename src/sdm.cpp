#include <QIcon>
#include <QInputDialog>
#include <QUrl>
#include <QApplication>
#include <QCursor>
#include "sdm.h"

SDM::SDM(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("BAT-DownloadManager"));
    setWindowIcon(QIcon(":/icons/dmIcon/app-icon.png"));
    setMinimumHeight(600);
    setMinimumWidth(500);

    createAction();

    mainVlayout = new QVBoxLayout;
    mainVlayout->setAlignment(Qt::AlignHCenter);
    mainVlayout->setSizeConstraint(QLayout::SetFixedSize);
    mainWidget.setLayout(mainVlayout);
    scrollArea.setWidget(&mainWidget);
    VLAYOUT = new QVBoxLayout;
    VLAYOUT->addWidget(&scrollArea);
    scrollArea.setWidgetResizable(true);
    MAIN_WIDGET.setLayout(VLAYOUT);
    MAIN_WIDGET.setStyleSheet("background-color: #333333");

    appIcon.load(":/icons/dmIcon/app-icon.png");
    appIconLabel.setPixmap(appIcon);
    appNameLabel.setText(tr("BAT-DownloadManager (open-source)"));
    appNameLabel.setStyleSheet("color: purple; font: bold");
    appIconLayout.addWidget(&appIconLabel);
    appIconLayout.addWidget(&appNameLabel);
    appIconLayout.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    appIconWidget = new QWidget;
    appIconWidget->setLayout(&appIconLayout);
    appIconWidget->setStyleSheet("background-color: #333333");

    if(noTasks)
        setCentralWidget(appIconWidget);
    else
        setCentralWidget(&MAIN_WIDGET);

    clipboard = QApplication::clipboard();
    oldClipboardText = clipboard->text();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SDM::checkClipboard);
    timer->start(500);
}

void SDM::createAction()
{
    addTask = new QAction(tr("Add new"), this);
    addTask->setIcon(QIcon(":/icons/dmIcon/new-task.png"));
    connect(addTask, SIGNAL(triggered()), this, SLOT(addNewTask()));
    quitDM = new QAction(tr("Quit"), this);
    quitDM->setIcon(QIcon(":/icons/dmIcon/quit.png"));
    connect(quitDM, &QAction::triggered, [this](){isForceQuit = true; close();});
    helpAction = new QAction(tr("help"), this);
    helpAction->setIcon(QIcon(":/icons/dmIcon/help.png"));
    connect(helpAction, &QAction::triggered, _help, &Help::showHelp);
    settingAction = new QAction(tr("settings"), this);
    settingAction->setIcon(QIcon(":/icons/dmIcon/setting.png"));
    tool_bar = addToolBar(tr("tool bar"));
    spacerWidget = new QWidget;
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tool_bar->addAction(addTask);
    tool_bar->addAction(quitDM);
    tool_bar->addWidget(spacerWidget);
    tool_bar->addAction(settingAction);
    tool_bar->addAction(helpAction);
}

void SDM::resizeEvent(QResizeEvent *e)
{
    if(e != nullptr)
    {
        QMainWindow::resizeEvent(e);
        qDebug()<<width();
        resizeWidgets();
    }
}

void SDM::resizeWidgets()
{
    for(auto itm : widgetList)
        itm->setMinimumWidth(width()-100);
}

void SDM::closeEvent(QCloseEvent *e)
{
    if(isForceQuit)
    {
        freeMem();
        e->accept();
    }
    else
    {
        e->ignore();
        hide();
    }
}

void SDM::addNewTask()
{
    QString urlText;
    bool ok;
    qDebug()<<"Add new task";
    urlText = QInputDialog::getText(this, "Add Url", "URL", QLineEdit::Normal, dwnldurl, &ok);
    if(ok && !urlText.isEmpty())
    {
        if(centralWidget() != &MAIN_WIDGET)
        {
            centralWidget()->setParent(0);
            setCentralWidget(&MAIN_WIDGET);
        }
        QString fileName;
        QLabel *dwnldLabel = new QLabel(tr("Downloading file"));
        QProgressBar *dwnldprogress = new QProgressBar;
        QVBoxLayout *dwnldvLayout = new QVBoxLayout;
        QWidget *dwnldWidget = new QWidget;
        QLabel *dwnldSizeLabel = new QLabel(tr("0/0    00 kb/s"));
        dwnldSizeLabel->setStyleSheet("color: pink");
        QLabel *dwnldspeedLabel = new QLabel(tr("0 kb/ps"));
        dwnldspeedLabel->setStyleSheet("color: cyan");
        SDM_network *sdmnetwork = new SDM_network;

        connect(sdmnetwork, &SDM_network::updateprogress, dwnldSizeLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::updateSpeed, dwnldspeedLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::statusPaused, dwnldspeedLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::updateprogressBarValue, dwnldprogress, &QProgressBar::setValue);
        connect(sdmnetwork, &SDM_network::updateprogressBarMax, dwnldprogress, &QProgressBar::setMaximum);
        connect(sdmnetwork, &SDM_network::updateDownloadStyle, dwnldLabel, &QLabel::setStyleSheet);
        connect(sdmnetwork, &SDM_network::setFileName, dwnldLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::removeSDM, this, &SDM::removeSDM);

        sdmList.append(sdmnetwork);
        labelList.append(dwnldLabel);
        progressbarList.append(dwnldprogress);
        vboxList.append(dwnldvLayout);
        widgetList.append(dwnldWidget);
        sizelabelList.append(dwnldSizeLabel);
        speedlabelList.append(dwnldspeedLabel);

        dwnldvLayout->addWidget(dwnldLabel);
        dwnldvLayout->addWidget(dwnldprogress);
        dwnldvLayout->addWidget(dwnldSizeLabel);
        dwnldvLayout->addWidget(dwnldspeedLabel);

        dwnldWidget->setLayout(dwnldvLayout);
        dwnldWidget->setMinimumWidth(width()-100);

        mainVlayout->addWidget(dwnldWidget);
        QThread *mThread = new QThread;
        threadList.append(mThread);
        sdmnetwork->moveToThread(mThread);
        connect(mThread, &QThread::started,
               [sdmnetwork, urlText](){sdmnetwork->startNewDownload(QUrl(urlText));});
        connect(sdmnetwork, &SDM_network::quitThread, mThread, &QThread::quit);
        mThread->start();

        QAction *startAction = new QAction(tr("Start"));
        QAction *pauseAction = new QAction(tr("Pause"));
        QAction *cancelAction = new QAction(tr("Cancel"));
        QAction *removeAction = new QAction(tr("Remove"));

        connect(startAction, &QAction::triggered, sdmnetwork, &SDM_network::resume);
        connect(pauseAction, &QAction::triggered, sdmnetwork, &SDM_network::pause);
        connect(cancelAction, &QAction::triggered, sdmnetwork, &SDM_network::cancel);
        connect(removeAction, &QAction::triggered, sdmnetwork, &SDM_network::remove);

        dwnldWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
        dwnldWidget->addAction(startAction);
        dwnldWidget->addAction(pauseAction);
        dwnldWidget->addAction(cancelAction);
        dwnldWidget->addAction(removeAction);

        actionList.append(startAction);
        actionList.append(pauseAction);
        actionList.append(cancelAction);
        actionList.append(removeAction);
    }
}

void SDM::removeSDM(SDM_network *sdm)
{
    qDebug()<<sdm->getFile();
    short index = sdmList.indexOf(sdm);
    widgetList[index]->hide();
}

void SDM::checkClipboard()
{
    dwnldurl = clipboard->text();
    if(!dwnldurl.isEmpty() && oldClipboardText != dwnldurl)
    {
        if(dwnldurl.contains("www",Qt::CaseInsensitive)||dwnldurl.contains("http",Qt::CaseInsensitive)||
           dwnldurl.contains(".com",Qt::CaseInsensitive)||dwnldurl.contains(".in",Qt::CaseInsensitive)||
           dwnldurl.contains(".us",Qt::CaseInsensitive)||dwnldurl.contains(".uk",Qt::CaseInsensitive))
        {
            if(isHidden())
                show();
            else
                raise();

            addNewTask();
            oldClipboardText = dwnldurl;
        }
    }
}

void SDM::forceQuit()
{
    isForceQuit = true;
    close();
}

void SDM::start()
{
    qDebug()<<"START";
}

void SDM::pause()
{
    qDebug()<<"PAUSE";
}

void SDM::cancel()
{
    qDebug()<<"CANCEL";
}

void SDM::clear()
{
    qDebug()<<"CLEAR";
}

void SDM::freeMem()
{
    delete tool_bar;
    delete addTask;
    delete mainVlayout;
    delete VLAYOUT;
    delete timer;
    delete settingAction;
    delete helpAction;
    delete _help;

    for(auto itm : sizelabelList)
        delete itm;
    for(auto itm : speedlabelList)
        delete itm;
    for(auto itm : labelList)
        delete itm;
    for(auto itm : progressbarList)
        delete itm;
    for(auto itm : vboxList)
        delete itm;
    for(auto itm : widgetList)
        delete itm;
    for(auto itm : sdmList)
        delete itm;
    for(auto itm : actionList)
        delete itm;
    for(auto itm : threadList)
        delete itm;

    qDebug()<<"free mem";
}

SDM::~SDM()
{
    freeMem();
}
