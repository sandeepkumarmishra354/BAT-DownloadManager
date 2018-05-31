#ifndef HELP_H
#define HELP_H

#include <QDialog>
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class Help : public QDialog
{
    Q_OBJECT

public:
    Help(QWidget *parent = 0);
    ~Help();

private:
    QVBoxLayout vLayout;
    QLabel detailTextLabel;
    QPushButton cancelButton, qtButton, batdmButton;

public slots:
    void showHelp();
};

#endif // HELP_H
