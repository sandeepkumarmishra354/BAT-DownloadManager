#include <QDebug>
#include "setting.h"

Setting::Setting(QWidget *parent) : QDialog(parent)
{
    qDebug()<<"setting constructor";
    saveBtn.setText(tr("Save"));
    connect(&saveBtn, &QPushButton::clicked, this, &Setting::saveSetting);
    cancelBtn.setText(tr("Cancel"));
    cancelBtn.setFocus();
    cancelBtn.setDefault(true);
    connect(&cancelBtn, &QPushButton::clicked, this, &Setting::cancelSetting);
    hLayout.addWidget(&saveBtn);
    hLayout.addWidget(&cancelBtn);
    setLayout(&hLayout);
    setStyleSheet("background-color: #333333");
    setWindowTitle("settings");
    setModal(true);
}

void Setting::saveSetting()
{
    qDebug()<<"save setting";
}

void Setting::cancelSetting()
{
    qDebug()<<"cancel setting";
    close();
}

Setting::~Setting()
{
    qDebug()<<"setting destructor";
}
