#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class Setting : public QDialog
{
    Q_OBJECT

public:
    Setting(QWidget *parent=0);
    ~Setting();
private:
    QPushButton saveBtn, cancelBtn;
    QHBoxLayout hLayout;

private slots:
    void saveSetting();
    void cancelSetting();

};

#endif // SETTING_H
