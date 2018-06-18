/***********************************************************************
 * ------------------------------------------------------------        *
 * | Project created by Sandeep Mishra --2018-05-26T15:11:02-- |       *
 * ------------------------------------------------------------        *
 * This program is a simple download manager                           *
 * Features- pause, resume, cancel, saves incomplete downloads         *
 *                                                                     *
 * This program is developed only for educational purpose              *
 * There is no copyright or any other issues                           *
 *                                                                     *
 * Feel free to contribute or modify the program                       *
 * https://github.com/sandeepkumarmishra354/BAT-DownloadManager        *
 *                                                                     *
 * For any query or bug report drop an email to me                     *
 * email- sandeepkumarmishra354@gmail.com                              *
 ***********************************************************************/

#include "src/sdm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SDM *sdm = new SDM;
    sdm->show();
    return a.exec();
}
