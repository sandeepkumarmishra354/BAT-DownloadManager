#include <QApplication>
#include "help.h"

Help::Help(QWidget *parent) : QDialog(parent)
{
    qDebug()<<"Constructor";
    detailTextLabel.setText("this is BAT-DownloadManager");
    cancelButton.setText("Ok");
    cancelButton.setFocus();
    connect(&cancelButton, &QPushButton::clicked, this, &Help::close);
    qtButton.setText("About Qt");
    connect(&qtButton, &QPushButton::clicked, qApp, &QApplication::aboutQt);
    batdmButton.setText("About BAT-DM");
    vLayout.addWidget(&detailTextLabel);
    vLayout.addSpacing(50);
    vLayout.addWidget(&qtButton);
    vLayout.addWidget(&batdmButton);
    vLayout.addWidget(&cancelButton);
    setLayout(&vLayout);
    setModal(true);
}

void Help::showHelp()
{
    qDebug()<<"show help slot";
    show();
}

Help::~Help()
{
    qDebug()<<"Destructor";
}
