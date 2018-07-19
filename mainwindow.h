#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qfile.h>
#include <QSettings>
#include <qstatemachine.h>
#include <qjsonobject.h>
#include <QListWidget>
#include <QTreeWidget>
#include <QMenu>
#include "utils.h"

class QTcpSocket;
class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum ColumnEnum {
          NameColumn
        , ParameterCountColumn
        , SendSetColumn
        , ReceiveSetColumn
        , SendGetColumn
        , ReceiveGetColumn
        , SetErrorColumn
        , GetErrorColumn
        , SetDescriptionColumn
        , GetDescriptionColumn
    };

    enum DockColumn {
          DockName
        , DockValue
        , DockDescripton
    };
    enum State { Idle, Set, Get };

    QList<QString> HiddenableColumns {
//          "Name"
          "Param Count"
        , "Send Set Message"
        , "Receive Set Message"
        , "Send Get Message"
        , "Receive Get Message"
//        , "Set Error"
//        , "Get Error"
        , "Set Description"
        , "Get Description"
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QSettings *appConfig;

    bool analizeParameter(QString data, QJsonObject pObj, QString &fmt, tsParam &param);
    QString getMsgKey(QString message);
    void addNewLine(QTreeWidgetItem *rootItem, tsParam param);
    void prepareMenu(const QPoint &pos);
signals:
    void next();
    void nextItem();
    void finishTest();
    void idle();
    void setStart();

private slots:
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    void connectToHost();
    void disconnectFromHost();
    void fileOpen();
    void send();
    void sendAll();
    void readyRead();
    void handleTimeout();
    void getMessageTimeout();
    void clear();
    void enableTest();
    void disableTest();
    void settingsDialog();
    void createSendAllStateMachine();
    void createConnectionStateMachine();
    auto getParam(QString param, QString type);

    void createTestCases();
    bool regexValidation();
    void createTestMessage(QJsonObject paramObj, QString &sendMsg);
    void generateParam(const QJsonValue & value, QString &msgParams);
    bool readJsonData();
    void showOnDock(QString recvData);

private:
    Ui::MainWindow *ui;
    QTcpSocket *m_tcpSocket;
    int index = 0;
    bool setFlag = false;
    bool getFlag = false;

    QMenu *r_menu;

    int m_TimerPeriod;

    QJsonObject m_rootObject;

    bool SetModeOn = true;

    QTimer *sendAlltimer;
    QTimer *sendSingleTimer;

    int paramTested = 0;
    int maxTestCaseCount = 1;
    bool passNextMsg = false;

    QString filePath;

    QStateMachine machine;
    QState *pause;
    QState *play;
    QState *stop;

    QStateMachine machineConnection;
    QState *connected;
    QState *disconnected;

    QStateMachine machineTest;
    QState *set;
    QState *get;

    bool flag = false;

    QTreeWidget *responseDetail;

    void createDockWindows();
    QString generateParamRandom(int min, int max);
    QString generateParamMin2Max(int min, int max);
    QString generateParamRange(QString type, QJsonArray &rangeArray);
};

#endif // MAINWINDOW_H
