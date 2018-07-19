#include "dialogSettings.h"
#include "mainwindow.h"
#include "ui_dialogSettings.h"

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
#if 1
    if(parent != nullptr)
    {
        MainWindow * main = qobject_cast<MainWindow *>(parent);
        main->appConfig->sync();
        ui->ipAddressEdit->setText(main->appConfig->value("server_address").toString());
        ui->portEdit->setText(main->appConfig->value("server_port").toString());
        ui->timerPeriodEdit->setText(main->appConfig->value("timer_period").toString());
        ui->themeCombo->setCurrentText(main->appConfig->value("theme").toString());
    }
#endif
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

#if 0
QSettings DialogSettings::getSettings()
{
    return m_settings;
}

void DialogSettings::setSettings(QSettings settings)
{
    m_settings = settings;
    ui->ipAddressEdit->setText(m_settings->value("server_address").toString());
    ui->portEdit->setText(m_settings->value("server_port").toString());
}
#endif

void DialogSettings::on_buttonBox_accepted()
{
#if 0
    m_settings.setValue("server_address", ui->ipAddressEdit->text());
    m_settings.setValue("server_port", ui->portEdit->text());
#endif

#if 1
    if(parent() != nullptr)
    {
        MainWindow * main = qobject_cast<MainWindow *>(parent());
        main->appConfig->setValue("server_address", ui->ipAddressEdit->text());
        main->appConfig->setValue("server_port", ui->portEdit->text().toUShort());
        main->appConfig->setValue("timer_period", ui->timerPeriodEdit->text().toUShort());
        main->appConfig->setValue("theme", ui->themeCombo->currentText());
        main->appConfig->sync();
    }
#endif
}

void DialogSettings::on_buttonBox_rejected()
{

}
