#ifndef HELP_H
#define HELP_H

#include <QDialog>
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QPixmap>

class Help : public QDialog
{
    Q_OBJECT

public:
    Help(QWidget *parent = 0);
    ~Help();

private:
    QVBoxLayout vLayout;
    QLabel detailTextLabel, logoLabel, opensourceLabel;
    QPixmap pixmap;
    QPushButton cancelButton, qtButton;

public slots:
    void showHelp();
    QString detail();
};

#endif // HELP_H
