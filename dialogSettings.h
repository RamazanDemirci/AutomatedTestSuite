#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = 0);
    ~DialogSettings();
#if 0
    QSettings getSettings();
    void setSettings();
#endif

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::DialogSettings *ui;
#if 0
    QSettings m_settings;
#endif
};

#endif // DIALOGSETTINGS_H
