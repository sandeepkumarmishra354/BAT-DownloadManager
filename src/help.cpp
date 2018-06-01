#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include "help.h"

Help::Help(QWidget *parent) : QDialog(parent)
{
    qDebug()<<"Constructor";
    detailTextLabel.setText(detail());
    detailTextLabel.setStyleSheet("color:white");
    connect(&detailTextLabel, &QLabel::linkActivated,
           [](QString link){QDesktopServices::openUrl(QUrl(link));});
    cancelButton.setText("Ok");
    cancelButton.setFocus();
    cancelButton.setDefault(true);
    connect(&cancelButton, &QPushButton::clicked, this, &Help::close);
    qtButton.setText("About Qt");
    connect(&qtButton, &QPushButton::clicked, qApp, &QApplication::aboutQt);
    pixmap.load(":/icons/dmIcon/app-icon.png");
    pixmap = pixmap.scaled(70,70,Qt::KeepAspectRatio);
    logoLabel.setPixmap(pixmap);
    logoLabel.setAlignment(Qt::AlignCenter);
    opensourceLabel.setText("(open-source)");
    opensourceLabel.setAlignment(Qt::AlignCenter);
    opensourceLabel.setStyleSheet("color: cyan");
    vLayout.addWidget(&detailTextLabel);
    vLayout.addWidget(&logoLabel);
    vLayout.addWidget(&opensourceLabel);
    vLayout.addSpacing(30);
    vLayout.addWidget(&qtButton);
    vLayout.addWidget(&cancelButton);
    setLayout(&vLayout);
    setStyleSheet("background-color: #333333");
    setMinimumHeight(400);
    setMinimumWidth(350);
    setModal(true);
}

QString Help::detail()
{
    QString _detail_;

    _detail_ = "<b style=\"color:cyan\">BAT-DownloadManager</b> v1.0<hr>"
               "This program is written with the help of Qt (A c++ framework)<br>"
               "for educational purpose only.<br>"
               "<span style=\"color:lightcyan\">Feel free to contribute in this project...</span><br>"
               "(there is no copyright or any other issue)<br>"
               "<b style=\"color:cyan\">Author</b>: <i style=\"color:cyan\">Sandeep Mishra</i><br>"
               "<i style=\"color:cyan\">(Batman/wolverine fan)</i><br>"
               "<i style=\"color:lightcyan\">\"It's not who I am underneath "
               "but what I do <br>that defines me :- Batman begins\"</i><hr>"
               "<i>for any question (or bug report) contact here: </i>"
               "<a href=\"mailto:sandeepkumarmishra354@gmail.com?Subject=BAT-DM question/bug\">E-mail</a><br>"
               "contribute here: "
               "<a href=\"https://github.com/sandeepkumarmishra354/BAT-DownloadManager\">GitHub<a/><br>";

    return _detail_;
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
