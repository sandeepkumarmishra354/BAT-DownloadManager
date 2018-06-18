#include <QIcon>
#include <QInputDialog>
#include <QUrl>
#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QDir>
#include "sdm.h"

SDM::SDM(QWidget *parent) : QMainWindow(parent)
{
    qDebug()<<"SDM constructor";
    setWindowTitle(tr("BAT-DownloadManager"));
    setWindowIcon(QIcon(":/resources/app-icon.png"));
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
    MAIN_WIDGET.setStyleSheet("background-color: #263238");

    appIcon.load(":/resources/app-icon.png");
    appIconLabel.setPixmap(appIcon);
    appNameLabel.setText(tr("BAT-DownloadManager (open-source)"));
    appNameLabel.setStyleSheet("color: darkcyan; font: bold");
    appIconLayout.addWidget(&appIconLabel);
    appIconLayout.addWidget(&appNameLabel);
    appIconLayout.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    appIconWidget = new QWidget;
    appIconWidget->setLayout(&appIconLayout);
    appIconWidget->setStyleSheet("background-color: #263238");

    // if there is no tasks then show the app icon as main widget
    if(noTasks)
        setCentralWidget(appIconWidget);
    // otherwise show tasks as main widget
    else
        setCentralWidget(&MAIN_WIDGET);

    // when new link is copied the lambda function executes
    clipboardManager = new ClipBoardManager(this);
    connect(clipboardManager, &ClipBoardManager::newLinkCopied,
           [this](QString link){dwnldurl=link; addNewTask();});

    // checks that any previous tasks are completed or not
    loadTasks();
}

void SDM::createAction()
{
    addTask = new QAction(tr("Add new"), this);
    addTask->setIcon(QIcon(":/resources/new-task.png"));
    addTask->setShortcut(QKeySequence::New);
    connect(addTask, SIGNAL(triggered()), this, SLOT(addNewTask()));
    quitDM = new QAction(tr("Quit"), this);
    quitDM->setIcon(QIcon(":/resources/quit.png"));
    quitDM->setShortcut(tr("CTRL+Q"));
    connect(quitDM, &QAction::triggered, [this](){isForceQuit = true; close();});
    helpAction = new QAction(tr("help"), this);
    helpAction->setShortcut(tr("ALT+H"));
    helpAction->setIcon(QIcon(":/resources/help.png"));
    connect(helpAction, &QAction::triggered, _help, &Help::show);
    settingAction = new QAction(tr("settings"), this);
    settingAction->setIcon(QIcon(":/resources/setting.png"));
    connect(settingAction, &QAction::triggered, _setting, &Setting::show);
    tool_bar = addToolBar(tr("tool bar"));
    spacerWidget = new QWidget;
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tool_bar->setStyleSheet("background-color: #263238");
    tool_bar->addAction(addTask);
    tool_bar->addAction(quitDM);
    tool_bar->addWidget(spacerWidget);
    tool_bar->addAction(settingAction);
    tool_bar->addAction(helpAction);
}

void SDM::loadTasks()
{
    qDebug()<<"Load tasks...";
    QStringList fileList;
    QDir dir(downloadPath);
    // contains a list of detail files
    fileList = dir.entryList(QStringList()<<"*.inf",QDir::Files);
    // iterate through each file
    for(auto itm : fileList)
    {
        qDebug()<<itm;
        QFile file(downloadPath+itm);
        if(file.open(QIODevice::ReadOnly))
        {
            QString data = file.readAll();
            QStringList dataList = data.split("\n", QString::SkipEmptyParts);
            dataList.removeFirst(); // removes the warning message
            // adds the details to container (linked list)
            addToContainer(dataList);
            file.close();
            qDebug()<<dataList;
        }
    }

    // finally restore the tasks
    restoreTasks();
}

void SDM::addToContainer(QStringList dataList)
{
    if(_tInfo == nullptr)
    {
        _tInfo = new taskInfo;
        _tInfo->link = dataList[0]; // downloading file url
        _tInfo->fileName = dataList[1]; // downloading file name
        _tInfo->totalByte = dataList[2].toLong(); // total size of file
        _tInfo->rcvByte = dataList[3].toLong(); // size of downloaded bytes
    }
    else
    {
        taskInfo *tmp = _tInfo;
        while(tmp->next != nullptr)
        {
            tmp = tmp->next;
        }
        taskInfo *tmpNode = new taskInfo;
        tmpNode->link = dataList[0]; // downloading file url
        tmpNode->fileName = dataList[1]; // downloading file name
        tmpNode->totalByte = dataList[2].toLong(); // total size of file
        tmpNode->rcvByte = dataList[3].toLong(); // size of downloaded bytes

        tmp->next = tmpNode;
    }
}

void SDM::restoreTasks()
{
    qDebug()<<"RESTORE";
    taskInfo *tmp = _tInfo;
    while(tmp != nullptr)
    {
        maxValue = tmp->totalByte;
        minValue = tmp->rcvByte;
        _link_ = tmp->link;
        _fileName = tmp->fileName;
        isRestore = true;
        // creates downloading widgets with full details
        createDownloadWidgets(tmp->link);
        tmp = tmp->next;
    }
}

// every time the App size will change the function will also execute
void SDM::resizeEvent(QResizeEvent *e)
{
    if(e != nullptr)
    {
        QMainWindow::resizeEvent(e);
        qDebug()<<"width: "<<width();
        qDebug()<<"height: "<<height();
        // resize the widgets according to new window size
        resizeWidgets();
    }
}

void SDM::resizeWidgets()
{
    for(auto itm : widgetList)
        itm->setMinimumWidth(width()-80);
}

// when user try to close the program this function will executes
void SDM::closeEvent(QCloseEvent *e)
{
    bool running = false;
    if(isForceQuit) // means suspend all progress and close the program
    {
        qDebug()<<"Force quit";
        // checks that there is any ongoing task or not
        for(auto itm : sdmList)
        {
            running = itm->isRunning();
            if(running)
                break;
        }
        if(running) // means there are some tasks that are currently running
        {
            // show a warning message
            qDebug()<<"Some tasks are still running ::: Are you sure ?";
            int status = QMessageBox::warning(this, "warning", "Some tasks are running\nAre you sure ?",
                           QMessageBox::No | QMessageBox::Yes);
            switch(status)
            {
                case QMessageBox::Yes: // suspend all process and close the program
                    qDebug()<<"Yes close";
                    freeMem();
                    clipboardManager->exitThread();
                    e->accept();
                    break;
                case QMessageBox::No: // don't suspend any process and don't close the program
                    qDebug()<<"No don't close";
                    e->ignore();
                    break;
            }
        }
        else // No tasks are running close the program
        {
            qDebug()<<"Exiting...";
            clipboardManager->exitThread();
            freeMem();
            e->accept();
        }

    }
    // if user closes the program from native close button them simply hide the program
    // and let it run in background
    else
    {
        qDebug()<<"Bat-Dm is now hidden (running in background)";
        qDebug()<<"copy any link and it will automatically pop up";
        e->ignore();
        hide();
    }
}

void SDM::addNewTask()
{
    QString urlText;
    bool ok, isLink = false;
    qDebug()<<"Add new task";
    urlText = QInputDialog::getText(this, "Add Url", "URL", QLineEdit::Normal, dwnldurl, &ok);
    // checks that given url is link or not
    if(urlText.contains("www",Qt::CaseInsensitive)||urlText.contains("http",Qt::CaseInsensitive)||
       urlText.contains(".com",Qt::CaseInsensitive)||urlText.contains(".in",Qt::CaseInsensitive)||
       urlText.contains(".us",Qt::CaseInsensitive)||urlText.contains(".uk",Qt::CaseInsensitive))
    {
        isLink = true;
    }
    if(!isLink && ok) // given url is not a link
    {
        qDebug()<<"You entered an invalid link";
        QMessageBox::warning(this, "url error", "please enter a valid link", QMessageBox::Ok);
        return;
    }
    if(ok && !urlText.isEmpty() && isLink) // given url is a link
    {
        qDebug()<<"Link- "<<urlText;
        createDownloadWidgets(urlText);
    }
}

void SDM::createDownloadWidgets(QString urlText)
{
    // remove the appicon as main widget and set tasks as main widget
    if(centralWidget() != &MAIN_WIDGET)
    {
        centralWidget()->setParent(0);
        setCentralWidget(&MAIN_WIDGET);
    }

    QWidget *dwnldWidget = new QWidget;
    dwnldWidget->setCursor(QCursor(Qt::PointingHandCursor));
    QVBoxLayout *dwnldvLayout = new QVBoxLayout(dwnldWidget);
    QLabel *dwnldLabel = new QLabel(dwnldWidget);
    dwnldLabel->setText(tr("Downloading file"));
    QProgressBar *dwnldprogress = new QProgressBar(dwnldWidget);
    QLabel *dwnldSizeLabel = new QLabel(dwnldWidget);
    dwnldSizeLabel->setText(tr("0/0    00 kb/s"));
    dwnldSizeLabel->setStyleSheet("color: pink");
    QLabel *dwnldspeedLabel = new QLabel(dwnldWidget);
    dwnldspeedLabel->setText(tr("0 kb/ps"));
    dwnldspeedLabel->setStyleSheet("color: cyan");
    // responsible for downloading files
    SDM_network *sdmnetwork = new SDM_network;

    connect(sdmnetwork, &SDM_network::updateprogress, dwnldSizeLabel, &QLabel::setText);
    connect(sdmnetwork, &SDM_network::updateSpeed, dwnldspeedLabel, &QLabel::setText);
    connect(sdmnetwork, &SDM_network::statusPaused, dwnldspeedLabel, &QLabel::setText);
    connect(sdmnetwork, &SDM_network::updateprogressBarValue, dwnldprogress, &QProgressBar::setValue);
    connect(sdmnetwork, &SDM_network::updateprogressBarMax, dwnldprogress, &QProgressBar::setMaximum);
    connect(sdmnetwork, &SDM_network::updateDownloadStyle, dwnldLabel, &QLabel::setStyleSheet);
    connect(sdmnetwork, &SDM_network::setFileName, dwnldLabel, &QLabel::setText);

    // sdmnetwork var container
    sdmList.append(sdmnetwork);
    // widget container
    widgetList.append(dwnldWidget);

    dwnldvLayout->addWidget(dwnldLabel);
    dwnldvLayout->addWidget(dwnldprogress);
    dwnldvLayout->addWidget(dwnldSizeLabel);
    dwnldvLayout->addWidget(dwnldspeedLabel);

    dwnldWidget->setLayout(dwnldvLayout);
    dwnldWidget->setMinimumWidth(width()-80);

    mainVlayout->addWidget(dwnldWidget);
    QThread *mThread = new QThread(sdmnetwork);
    // thread container
    threadList.append(mThread);
    // set downloading file as a seperate thread
    sdmnetwork->moveToThread(mThread);

    connect(mThread, &QThread::started,
           [sdmnetwork,dwnldprogress,urlText,this] ()
           {
                if(isRestore) // previous uncompleted tasks
                {
                    sdmnetwork->setSavedByte(minValue, maxValue, urlText, _fileName);
                    dwnldprogress->setMaximum(maxValue);
                    dwnldprogress->setValue(minValue);
                    isRestore = false;
                }
                else // fresh tasks
                    sdmnetwork->startNewDownload(QUrl(urlText));
           });

    // when download finish quit the thread
    connect(sdmnetwork, &SDM_network::quitThread, mThread, &QThread::quit);
    // start downloading in a new thread
    mThread->start();

    QAction *startAction = new QAction(tr("Start"),dwnldWidget);
    QAction *pauseAction = new QAction(tr("Pause"),dwnldWidget);
    QAction *cancelAction = new QAction(tr("Cancel"),dwnldWidget);
    QAction *removeAction = new QAction(tr("Remove"),dwnldWidget);

    connect(startAction, &QAction::triggered, sdmnetwork, &SDM_network::resume);
    connect(pauseAction, &QAction::triggered, sdmnetwork, &SDM_network::pause);
    connect(cancelAction, &QAction::triggered, sdmnetwork, &SDM_network::cancel);
    connect(removeAction, &QAction::triggered, sdmnetwork, &SDM_network::cancel);
    connect(removeAction, &QAction::triggered, this, &SDM::removeSDM);

    dwnldWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    dwnldWidget->addAction(startAction);
    dwnldWidget->addAction(pauseAction);
    dwnldWidget->addAction(cancelAction);
    dwnldWidget->addAction(removeAction);
}

void SDM::removeSDM()
{
    QObject *action_obj = sender();// returns signal sender object
    // cast the QObject into QAction bcz we know that QAction is only responsible for this slot
    QAction *act_cast = qobject_cast<QAction*>(action_obj);
    QWidget *wid;
    SDM_network *sdm;
    if(action_obj != 0)
    {
        qDebug()<<"action= "<<act_cast->text();
        wid = act_cast->parentWidget();
        sdm = sdmList[widgetList.indexOf(wid)];
        // hodes main tasks
        wid->hide();
        // remove and stop download
        sdm->remove();
        delete sdm;
        sdmList.removeOne(sdm);
        if(SDM_network::totalDownloads() == 0)
        {
            if(centralWidget() == &MAIN_WIDGET)
            {
                centralWidget()->setParent(0);
                setCentralWidget(appIconWidget);
            }
        }
        qDebug()<<"widgets removed";
    }
    else
        qDebug()<<"Widget remove Error";
}

// when user clicks on cross icon this function executes
void SDM::forceQuit()
{
    isForceQuit = true;
    qDebug()<<"Force Quit";
    close();
}

// release the memory
void SDM::freeMem()
{
    delete tool_bar;
    delete addTask;
    delete mainVlayout;
    delete VLAYOUT;
    delete settingAction;
    delete helpAction;
    delete clipboardManager;
    delete _help;
    delete _setting;

    for(auto itm : widgetList)
        delete itm;
    for(auto itm : sdmList)
        delete itm;
    if(_tInfo != nullptr)
        delete _tInfo;

    qDebug()<<"SDM destructor";
}

SDM::~SDM()
{
    freeMem();
}
