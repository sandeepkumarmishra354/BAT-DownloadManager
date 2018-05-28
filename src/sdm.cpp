#include <QIcon>
#include <QInputDialog>
#include <QUrl>
#include <QApplication>
#include "sdm.h"

SDM::SDM(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("BAT-DownloadManager"));
    setWindowIcon(QIcon(":/icons/dmIcon/app-icon.png"));
    setMinimumHeight(600);
    setMinimumWidth(500);
    addTask = new QAction(tr("Add new"), this);
    addTask->setIcon(QIcon(":/icons/dmIcon/new-task.png"));
    connect(addTask, SIGNAL(triggered()), this, SLOT(addNewTask()));
    quitDM = new QAction(tr("Quit"), this);
    quitDM->setIcon(QIcon(":/icons/dmIcon/quit.png"));
    connect(quitDM, SIGNAL(triggered()), this, SLOT(forceQuit()));
    tool_bar = addToolBar(tr("tool bar"));
    tool_bar->addAction(addTask);
    tool_bar->addAction(quitDM);

    mainVlayout = new QVBoxLayout;
    mainVlayout->setAlignment(Qt::AlignHCenter);
    mainVlayout->setSizeConstraint(QLayout::SetFixedSize);
    mainWidget.setLayout(mainVlayout);
    scrollArea.setWidget(&mainWidget);
    VLAYOUT = new QVBoxLayout;
    VLAYOUT->addWidget(&scrollArea);
    scrollArea.setWidgetResizable(true);
    MAIN_WIDGET.setLayout(VLAYOUT);
    MAIN_WIDGET.setStyleSheet("background-color: #212121");

    appIcon.load(":/icons/dmIcon/app-icon.png");
    appIconLabel.setPixmap(appIcon);
    appNameLabel.setText(tr("BAT-DownloadManager (open-source)"));
    appIconLayout.addWidget(&appIconLabel);
    appIconLayout.addWidget(&appNameLabel);
    appIconLayout.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    appIconWidget = new QWidget;
    appIconWidget->setLayout(&appIconLayout);

    if(noTasks)
        setCentralWidget(appIconWidget);
    else
        setCentralWidget(&MAIN_WIDGET);

    createContextMenu();

    clipboard = QApplication::clipboard();
    oldClipboardText = clipboard->text();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkClipboard()));
    timer->start(500);
}

void SDM::createContextMenu()
{
    contextMenu = new QMenu(this);
    startAction = new QAction(tr("Start"),contextMenu);
    pauseAction = new QAction(tr("Pause"),contextMenu);
    cancelAction = new QAction(tr("Cancel"),contextMenu);
    clearAction = new QAction(tr("Remove"), contextMenu);

    contextMenu->addAction(startAction);
    contextMenu->addAction(pauseAction);
    contextMenu->addAction(cancelAction);
    contextMenu->addAction(clearAction);
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
        itm->setMinimumWidth(width()-50);
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

        connect(sdmnetwork, SIGNAL(updateprogress(QString)), dwnldSizeLabel, SLOT(setText(QString)));
        connect(sdmnetwork, SIGNAL(updateSpeed(QString)), dwnldspeedLabel, SLOT(setText(QString)));
        connect(sdmnetwork, SIGNAL(updateprogressBarValue(int)), dwnldprogress, SLOT(setValue(int)));
        connect(sdmnetwork, SIGNAL(updateprogressBarMax(int)), dwnldprogress, SLOT(setMaximum(int)));
        connect(sdmnetwork, SIGNAL(updateDownloadStyle(QString)), dwnldLabel, SLOT(setStyleSheet(QString)));

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
        dwnldWidget->setMinimumWidth(width()-50);

        mainVlayout->addWidget(dwnldWidget);
        sdmnetwork->startNewDownload(QUrl(urlText));
        fileName = sdmnetwork->getFileName(QUrl(urlText));
        fileName = fileName.mid(fileName.lastIndexOf("/")+1);
        dwnldLabel->setText(fileName);

        dwnldWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(dwnldWidget, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenu(QPoint)));
    }
}

void SDM::showContextMenu(QPoint pos)
{
    qDebug()<<"context menu";
    contextMenu->popup(scrollArea.viewport()->mapToGlobal(pos));
}

void SDM::initContext(SDM_network *sdmnetwork)
{
    qDebug()<<"init context";
    qDebug()<<sdmnetwork->getFile();
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

void SDM::freeMem()
{
    delete tool_bar;
    delete addTask;
    delete mainVlayout;
    delete VLAYOUT;
    delete timer;
    delete contextMenu;

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

    qDebug()<<"free mem";
}

SDM::~SDM()
{
    freeMem();
}
