#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include "help.h"

Help::Help(QWidget *parent) : QDialog(parent)
{
    qDebug()<<"Help Constructor";
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

    pixmap.load(":/resources/app-icon.png");
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
    setWindowTitle("about/help");
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
               "<i style=\"color:cyan\">(Batman/wolverine fan)</i><br><br>"
               "<i style=\"color:darkcyan\">\"It's not who I am underneath "
               "but what I do <br>that defines me :- Batman\"</i><br><br>"
               "<i style=\"color:darkcyan\">\"I'm the best there is at what I do<br>"
               "but what I do best isn't very nice :- Wolverine\"</i><hr>"
               "<i>for any question (or bug report) contact here: </i>"
               "<a href=\"mailto:sandeepkumarmishra354@gmail.com?Subject=BAT-DM question/bug\">E-mail</a><br>"
               "contribute here: "
               "<a href=\"https://github.com/sandeepkumarmishra354/BAT-DownloadManager\">GitHub<a/><br>";

    return _detail_;
}

Help::~Help()
{
    qDebug()<<"Help Destructor";
}
