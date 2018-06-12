#include <QIcon>
#include <QInputDialog>
#include <QUrl>
#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include "sdm.h"

SDM::SDM(QWidget *parent) : QMainWindow(parent)
{
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
    MAIN_WIDGET.setStyleSheet("background-color: #333333");

    appIcon.load(":/resources/app-icon.png");
    appIconLabel.setPixmap(appIcon);
    appNameLabel.setText(tr("BAT-DownloadManager (open-source)"));
    appNameLabel.setStyleSheet("color: darkcyan; font: bold");
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
    helpAction->setIcon(QIcon(":/resources/help.png"));
    connect(helpAction, &QAction::triggered, _help, &Help::show);
    settingAction = new QAction(tr("settings"), this);
    settingAction->setIcon(QIcon(":/resources/setting.png"));
    connect(settingAction, &QAction::triggered, _setting, &Setting::show);
    tool_bar = addToolBar(tr("tool bar"));
    spacerWidget = new QWidget;
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tool_bar->addAction(addTask);
    tool_bar->addAction(quitDM);
    tool_bar->addWidget(spacerWidget);
    tool_bar->addAction(settingAction);
    tool_bar->addAction(helpAction);
}

void SDM::loadTasks()
{
    qDebug()<<"Load tasks...";
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
        itm->setMinimumWidth(width()-80);
}

void SDM::closeEvent(QCloseEvent *e)
{
    bool running = false;
    if(isForceQuit)
    {
        for(auto itm : sdmList)
        {
            running = itm->isRunning();
            if(running)
                break;
        }
        if(running)
        {
            int status = QMessageBox::warning(this, "warning", "Some tasks are running\nAre you sure ?",
                           QMessageBox::No | QMessageBox::Yes);
            switch(status)
            {
                case QMessageBox::Yes:
                    freeMem();
                    e->accept();
                    break;
                case QMessageBox::No:
                    e->ignore();
                    break;
            }
        }
        else
        {
            freeMem();
            e->accept();
        }

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
    bool ok, isLink = false;
    qDebug()<<"Add new task";
    urlText = QInputDialog::getText(this, "Add Url", "URL", QLineEdit::Normal, dwnldurl, &ok);
    if(urlText.contains("www",Qt::CaseInsensitive)||urlText.contains("http",Qt::CaseInsensitive)||
       urlText.contains(".com",Qt::CaseInsensitive)||urlText.contains(".in",Qt::CaseInsensitive)||
       urlText.contains(".us",Qt::CaseInsensitive)||urlText.contains(".uk",Qt::CaseInsensitive))
    {
        isLink = true;
    }
    if(!isLink && ok)
    {
        QMessageBox::warning(this, "url error", "please enter a valid link", QMessageBox::Ok);
        return;
    }
    if(ok && !urlText.isEmpty() && isLink)
    {
        if(centralWidget() != &MAIN_WIDGET)
        {
            centralWidget()->setParent(0);
            setCentralWidget(&MAIN_WIDGET);
        }

        QWidget *dwnldWidget = new QWidget;
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
        SDM_network *sdmnetwork = new SDM_network(dwnldWidget);

        connect(sdmnetwork, &SDM_network::updateprogress, dwnldSizeLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::updateSpeed, dwnldspeedLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::statusPaused, dwnldspeedLabel, &QLabel::setText);
        connect(sdmnetwork, &SDM_network::updateprogressBarValue, dwnldprogress, &QProgressBar::setValue);
        connect(sdmnetwork, &SDM_network::updateprogressBarMax, dwnldprogress, &QProgressBar::setMaximum);
        connect(sdmnetwork, &SDM_network::updateDownloadStyle, dwnldLabel, &QLabel::setStyleSheet);
        connect(sdmnetwork, &SDM_network::setFileName, dwnldLabel, &QLabel::setText);

        sdmList.append(sdmnetwork);
        widgetList.append(dwnldWidget);

        dwnldvLayout->addWidget(dwnldLabel);
        dwnldvLayout->addWidget(dwnldprogress);
        dwnldvLayout->addWidget(dwnldSizeLabel);
        dwnldvLayout->addWidget(dwnldspeedLabel);

        dwnldWidget->setLayout(dwnldvLayout);
        dwnldWidget->setMinimumWidth(width()-80);

        mainVlayout->addWidget(dwnldWidget);
        QThread *mThread = new QThread(sdmnetwork);
        threadList.append(mThread);
        sdmnetwork->moveToThread(mThread);
        connect(mThread, &QThread::started,
               [sdmnetwork, urlText](){sdmnetwork->startNewDownload(QUrl(urlText));});
        connect(sdmnetwork, &SDM_network::quitThread, mThread, &QThread::quit);
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
}

void SDM::removeSDM()
{
    QObject *action_obj = sender();
    QAction *act_cast = qobject_cast<QAction*>(action_obj);
    if(action_obj != 0)
    {
        qDebug()<<"action= "<<act_cast->text();
        act_cast->parentWidget()->hide();
    }
    else
        qDebug()<<"Widget remove Error";
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
    delete settingAction;
    delete helpAction;
    delete _help;
    delete _setting;

    for(auto itm : widgetList)
        delete itm;

    qDebug()<<"free mem";
}

SDM::~SDM()
{
    freeMem();
}
