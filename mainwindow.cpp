#include "dialogSettings.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QDebug>
#include <QHostAddress>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QRandomGenerator>
#include <QTime>
#include <QThread>
#include <QFileDialog>
#include <QStateMachine>
#include <QTimer>
#include <QEventTransition>
#include <QSignalTransition>
#include <QStandardPaths>
#include <QDockWidget>
#include <QShortcut>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_tcpSocket(0), r_menu(0)
{

    ui->setupUi(this);
    //ui->treeWidget->setAlternatingRowColors(true);

    /*RightClickMenu*/

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::prepareMenu);

    /*appConfig*/
    QString appConfigDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    appConfig = new QSettings(appConfigDir + "/appConfig.ini", QSettings::IniFormat);

    /*Theme*/
    if(appConfig->value("theme").toString() == "Dark")
    {
        QFile File(":/res/dark.qss");
        File.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(File.readAll());

        qApp->setStyleSheet(StyleSheet);
    }
    else
        ui->treeWidget->setAlternatingRowColors(true);

    /*Initialize*/
    m_tcpSocket = new QTcpSocket();
    sendAlltimer = new QTimer(this);

    disableTest();
    readJsonData();
    createDockWindows();

    /*Two way button implementations*/
    createSendAllStateMachine();
    createConnectionStateMachine();

    /*Enter Pressed*/
    QShortcut *returnShortcut = new QShortcut(QKeySequence(Qt::Key_Return), ui->treeWidget);    //Return
    connect(returnShortcut, &QShortcut::activated, ui->sendButton, &QPushButton::click);

    /*connect*/
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, [=](QAbstractSocket::SocketState state) {
        if (state == QAbstractSocket::ConnectedState) {
            m_TimerPeriod = appConfig->value("timer_period").toInt();

            ui->openButton->setEnabled(true);
            if(appConfig->value("file_path").toString() != QString::null)
            {
               ui->createButton->setEnabled(true);
            }
        }

        if (state == QAbstractSocket::UnconnectedState) {
            disableTest();
        }
    });
    connect(ui->connectButton, &QPushButton::clicked, [=](){
        if(machineConnection.property("state").toString() == "disconnected")
        {
            connectToHost();
        }
        else
        {
            disconnectFromHost();
        }
    });
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::send);
    connect(ui->sendAllButton, &QPushButton::clicked, this, &MainWindow::sendAll);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clear);
    connect(ui->createButton, &QPushButton::clicked, this, &MainWindow::createTestCases);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::fileOpen);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::settingsDialog);
    connect(ui->autoQueryButton, &QPushButton::clicked, [=](bool checked){
        QString key;
        QString icon_path = ":/res/query_start_32.png";

        if(checked)
        {
            key = "otomatik_sorgulama";
            icon_path = ":/res/query_clear_32.png";
        }
        else
        {
            QString key = "otomatik_sorgulama_hepsini_sil";
            icon_path = ":/res/query_start_32.png";
        }

        ui->autoQueryButton->setIcon(QIcon(icon_path));
    });
    connect(this, &MainWindow::nextItem, this, &MainWindow::sendAll);
    connect(this, &MainWindow::next, this, &MainWindow::send);
    connect(sendAlltimer, &QTimer::timeout, this, &MainWindow::handleTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToHost()
{
    QString address = appConfig->value("server_address").toString();
    quint16 port = appConfig->value("server_port").toString().toUShort();

    m_tcpSocket->connectToHost(QHostAddress(address), port);
}

void MainWindow::disconnectFromHost()
{
    m_tcpSocket->disconnectFromHost();
}

void MainWindow::fileOpen()
{
    filePath = QFileDialog::getOpenFileName();

    appConfig->setValue("file_path", filePath);

    ui->createButton->setEnabled(true);
}

void MainWindow::send()
{
    flag = false;
    QByteArray ba;

    QTreeWidgetItem *item = ui->treeWidget->currentItem();

    if(item == nullptr)
        return;

    int state = item->data(SendSetColumn, Qt::UserRole).toInt();
    state  = (state == Idle) ? Set : Get;
    item->setData(SendSetColumn, Qt::UserRole, state);
    ColumnEnum column = (state == Set) ? SendSetColumn : SendGetColumn;

    ba = item->data(column, Qt::DisplayRole).toByteArray();
    m_tcpSocket->write(ba);
    sendAlltimer->start(m_TimerPeriod);
}

void MainWindow::sendAll()
{
    QByteArray ba;
    if (index >= ui->treeWidget->topLevelItemCount()) {
        index = 0;
        sendAlltimer->stop();
        emit finishTest();
        return;
    }

    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(index);
    ui->treeWidget->setCurrentItem(item);
    ui->treeWidget->scrollToItem(item);

    int state = item->data(SendSetColumn, Qt::UserRole).toInt();

    state  = (state == Idle) ? Set : Get;
    item->setData(SendSetColumn, Qt::UserRole, state);
    ColumnEnum column = (state == Set) ? SendSetColumn : SendGetColumn;

    ba = item->data(column, Qt::DisplayRole).toByteArray();
    m_tcpSocket->write(ba);
    sendAlltimer->start(m_TimerPeriod);
}

void MainWindow::readyRead()
{
    flag = true;

    QByteArray recvData = m_tcpSocket->readAll();

    QTreeWidgetItem *item = ui->treeWidget->currentItem();

    if (!item){ return; }

    int state = item->data(SendSetColumn, Qt::UserRole).toInt();

    if(!ui->pinButton->isChecked() && state == Idle)
        showOnDock(QString(recvData));

    QByteArray paramName = item->data(NameColumn, Qt::DisplayRole).toByteArray();
    item->data(ParameterCountColumn, Qt::DisplayRole).toInt();

    if(QString(recvData).contains(paramName) == false)
    {
        qDebug() << "This Message Not Defined in Json :: " << QString(recvData);
        return;
    }

    QString recvFmt;
    QString sendFmt;
    bool bbValid = true;
    bool bValid = false;
    QJsonObject paramObj = m_rootObject[paramName].toObject();
    QJsonArray recv_params = paramObj["recv_params"].toArray();
    QJsonArray recv_format = paramObj["recv_format"].toArray();
    QJsonArray send_params = paramObj["send_params"].toArray();
    QJsonArray send_format = paramObj["send_format"].toArray();

    if(!send_params.isEmpty() && !recv_params.isEmpty())
    {
        if(send_format.count() == recv_format.count())
        {
            responseDetail->clear();
            QTreeWidgetItem *rootItem = new QTreeWidgetItem(responseDetail);
            rootItem->setData(DockName, Qt::DisplayRole, paramName.toUpper());

            QTreeWidgetItem *rootItem1 = new QTreeWidgetItem(rootItem);
            rootItem1->setData(DockName, Qt::DisplayRole, "SEND");

            QTreeWidgetItem *rootItem2 = new QTreeWidgetItem(rootItem);
            rootItem2->setData(DockName, Qt::DisplayRole, "RECV");

            rootItem->setExpanded(true);
            rootItem1->setExpanded(true);
            rootItem2->setExpanded(true);

            for (int var = 0; var < recv_params.count(); ++var)
            {
                recvFmt.append(recv_format.at(var).toString());
                sendFmt.append(send_format.at(var).toString());

                bool bSendValid;
                bool bRecvValid;
                tsParam sendParam;
                tsParam recvParam;
                QString sendMsg = item->data(SendSetColumn, Qt::DisplayRole).toString();

                bSendValid = analizeParameter(sendMsg, send_params.at(var).toObject(), sendFmt, sendParam);
                bRecvValid = analizeParameter(recvData, recv_params.at(var).toObject(), recvFmt, recvParam);

                if(bRecvValid && bSendValid)
                    bValid = Utils::compare(sendParam, recvParam);
                else
                    bValid = false;

                recvParam.status = bValid;

                addNewLine(rootItem1, sendParam);
                addNewLine(rootItem2, recvParam);

                if(bValid == false)
                {
                    bbValid = false;
                    if (state == Set)
                        item->setData(SetDescriptionColumn, Qt::DisplayRole, sendParam.name);
                    else
                        item->setData(GetDescriptionColumn, Qt::DisplayRole, recvParam.name);
                }
            }

            rootItem->setData(DockDescripton, Qt::DisplayRole, bbValid ? "Success" : "Fail");
            rootItem->setData(DockDescripton, Qt::TextColorRole, bbValid ? QColor(Qt::darkGreen) : QColor(Qt::red));

            rootItem1->setData(DockDescripton, Qt::DisplayRole, bbValid ? "Success" : "Fail");
            rootItem1->setData(DockDescripton, Qt::TextColorRole, bbValid ? QColor(Qt::darkGreen) : QColor(Qt::red));
            responseDetail->insertTopLevelItem(responseDetail->topLevelItemCount()+1, rootItem1);

            rootItem2->setData(DockDescripton, Qt::DisplayRole, bbValid ? "Success" : "Fail");
            rootItem2->setData(DockDescripton, Qt::TextColorRole, bbValid ? QColor(Qt::darkGreen) : QColor(Qt::red));
            responseDetail->insertTopLevelItem(responseDetail->topLevelItemCount()+1, rootItem2);
        }
        else
            qDebug() << "recvArray or recvFormat is Empty";
    }
    else
    {
        if(!ui->autoQueryButton->isChecked())
            qDebug() << "count of recvArray not equal to count of recvFormat";
    }

    if (state == Set) {
        item->setData(ReceiveSetColumn, Qt::DisplayRole, recvData);
        item->setData(SetErrorColumn, Qt::DisplayRole, bbValid ? "Success" : "Fail");
        item->setData(SetErrorColumn, Qt::TextColorRole, bbValid ? QColor(Qt::darkGreen) : QColor(Qt::red));
    } else if (state == Get) {
        item->setData(ReceiveGetColumn, Qt::DisplayRole, recvData);
        item->setData(GetErrorColumn, Qt::DisplayRole, bbValid ? "Success" : "Fail");
        item->setData(GetErrorColumn, Qt::TextColorRole, bbValid ? QColor(Qt::darkGreen) : QColor(Qt::red));
    }
}

void MainWindow::handleTimeout()
{
    sendAlltimer->stop();

    if (!flag) {
        QTreeWidgetItem *item = ui->treeWidget->topLevelItem(index);
        item->setData(SetDescriptionColumn, Qt::DisplayRole, "Timeout Error");
    }

    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    int state = item->data(SendSetColumn, Qt::UserRole).toInt();


    if (state == Set)
    {
        if(item->data(SendSetColumn, Qt::UserRole).isNull() == false)
            item->setData(SendSetColumn, Qt::UserRole, Get);
    }


    if (state == Get)
    {
        if(item->data(SendSetColumn, Qt::UserRole).isNull() == false)
            item->setData(SendSetColumn, Qt::UserRole, Idle);

        if(machine.property("state") == "play")
            index++;
    }

    flag = false;

    if(machine.property("state") == "play")
        emit nextItem();

    if(ui->sendButton->isEnabled())
    {
        if (state == Set)
            emit next();
    }
}

void MainWindow::clear()
{
    emit finishTest();
    sendAlltimer->stop();
    ui->treeWidget->clear();
    ui->sendButton->setEnabled(false);
    ui->sendAllButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    ui->createButton->setEnabled(true);
    ui->treeWidget->setEnabled(false);
}

void MainWindow::enableTest()
{
    index = 0;
    ui->sendButton->setEnabled(true);
    ui->sendAllButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
    ui->createButton->setEnabled(false);
    ui->treeWidget->setEnabled(true);
}

void MainWindow::disableTest()
{
    index = 0;
    ui->openButton->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->sendAllButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    ui->createButton->setEnabled(false);
    ui->treeWidget->setEnabled(false);
}

void MainWindow::settingsDialog()
{
    DialogSettings *dialog = new DialogSettings(this);
    dialog->setModal(true);
    //dialog->setSettings();
    dialog->exec();
    //dialog->getSettings();
}

void MainWindow::createSendAllStateMachine()
{
    /*create sendAll states*/
    pause = new QState();
    pause->setObjectName("play");
    pause->assignProperty(ui->sendAllButton, "icon", QIcon(":/res/send_all_play_32.png"));
    pause->assignProperty(&machine, "state",   "pause");

    play = new QState();
    play->setObjectName("pause");
    play->assignProperty(ui->sendAllButton, "icon", QIcon(":/res/send_all_pause_32.png"));
    play->assignProperty(&machine, "state",   "play");

    stop = new QState();
    stop->setObjectName("stop");
    stop->assignProperty(&machine, "state",   "stop");
    stop->assignProperty(ui->sendAllButton, "icon", QIcon(":/res/send_all_play_32.png"));

    /*add transitions*/
    pause->addTransition(ui->sendAllButton, &QPushButton::clicked, play);
    play->addTransition(ui->sendAllButton, &QPushButton::clicked, pause);
    stop->addTransition(ui->sendAllButton, &QPushButton::clicked, play);

    pause->addTransition(this, &MainWindow::finishTest, stop);
    play->addTransition(this, &MainWindow::finishTest, stop);

    connect(pause, &QState::entered, [=](){
        sendAlltimer->stop();
    });

    connect(play, &QState::entered, [=](){
        ui->sendButton->setEnabled(false);
    });

    connect(play, &QState::exited, [=](){
        ui->sendButton->setEnabled(true);
    });

    /*add state*/
    machine.addState(pause);
    machine.addState(play);
    machine.addState(stop);

    machine.setInitialState(stop);
    machine.start();
}

void MainWindow::createConnectionStateMachine()
{
    /*create server connection states*/
    connected = new QState();
    connected->setObjectName("connected");
    connected->assignProperty(ui->connectButton, "icon", QIcon(":/res/disconnect-icon.png"));
    connected->assignProperty(&machineConnection, "state", "connected");

    disconnected = new QState();
    disconnected->setObjectName("disconnected");
    disconnected->assignProperty(ui->connectButton, "icon", QIcon(":/res/connect-icon.png"));
    disconnected->assignProperty(&machineConnection, "state", "disconnected");

    connected->addTransition(m_tcpSocket, &QTcpSocket::disconnected, disconnected);
    disconnected->addTransition(m_tcpSocket, &QTcpSocket::connected, connected);

    machineConnection.addState(connected);
    machineConnection.addState(disconnected);

    machineConnection.setInitialState(disconnected);
    machineConnection.start();
}

void MainWindow::createTestCases()
{
    ui->treeWidget->clear();

    foreach(const QString& key, m_rootObject.keys())
    {
        passNextMsg = false;
        paramTested = 0;
        maxTestCaseCount = 1;

        QJsonObject paramObj = m_rootObject[key].toObject();
        QJsonArray sendArray = paramObj["send_params"].toArray();

        if(!sendArray.isEmpty())
        {
            while(!passNextMsg)
            {
                QString sendMsg;

                createTestMessage(paramObj, sendMsg);

                QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
                item->setData(NameColumn, Qt::DisplayRole, key);
                item->setData(SendSetColumn, Qt::DisplayRole, sendMsg);
                item->setData(SendGetColumn, Qt::DisplayRole, paramObj["get_format"].toString());
                item->setData(ParameterCountColumn, Qt::DisplayRole, sendArray.count());
                item->setData(SendSetColumn, Qt::UserRole, Idle);
            }
        }
        else if(!paramObj["get_format"].toString().isEmpty())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
            item->setData(NameColumn, Qt::DisplayRole, key);
            item->setData(SendSetColumn, Qt::UserRole, Idle);
            item->setData(SendGetColumn, Qt::DisplayRole, paramObj["get_format"].toString());
        }
    }

    enableTest();

    /*Resize columns*/
    ui->treeWidget->resizeColumnToContents(NameColumn);
    ui->treeWidget->resizeColumnToContents(ParameterCountColumn);
    ui->treeWidget->resizeColumnToContents(SendSetColumn);
    ui->treeWidget->resizeColumnToContents(SendGetColumn);
    ui->treeWidget->resizeColumnToContents(SetErrorColumn);
    ui->treeWidget->resizeColumnToContents(GetErrorColumn);
}

void MainWindow::createTestMessage(QJsonObject paramObj, QString &sendMsg)
{
    QString tmp;
    QString tmpParam;

    QJsonArray send_paramList = paramObj["send_params"].toArray();
    QJsonArray send_FormatList = paramObj["send_format"].toArray();

    for (int var = 0; var < send_paramList.count(); var++)
    {
        QJsonObject sendParamObj = send_paramList.at(var).toObject();
        QString send_format = send_FormatList[var].toString();
        QString format = sendParamObj["format"].toString();
        QString type = sendParamObj["type"].toString();

        send_format.replace(QRegExp("%\\d*\\d"), format);

        generateParam(sendParamObj, tmp);

        if(type == "double")
            tmpParam.sprintf(send_format.toLatin1(), tmp.toDouble());
        else if(type == "int")
            tmpParam.sprintf(send_format.toLatin1(), tmp.toInt());
        else if(type == "string")
            tmpParam.sprintf(send_format.toLatin1(), tmp);
        else if(type == "char")
            tmpParam.sprintf(send_format.toLatin1(), tmp.at(0).toLatin1());
        else
            qDebug() << "unknown param type!";

        sendMsg.append(tmpParam);
    }

    paramTested++;

    if((paramTested >= maxTestCaseCount) || paramTested > 30)
    {/*max 30 test case will be created*/
        passNextMsg = true;
    }
}

void MainWindow::generateParam(const QJsonValue &value, QString &msgParams)
{
    QJsonObject obj = value.toObject();
    int min            = obj["min"].toInt();
    int max            = obj["max"].toInt();
    QString type       = obj["type"].toString();
    QJsonArray range   = obj["range"].toArray();

    if(obj["generate"].toString() == "random")
        msgParams = generateParamRandom(min, max);
    else if(obj["generate"].toString() == "range")
        msgParams = generateParamRange(type, range);
    else if(obj["generate"].toString() == "min2max")
        msgParams = generateParamMin2Max(min, max);
}

QString MainWindow::generateParamRandom(int min, int max)
{/*generate random value*/
    return tr("%1").arg((QRandomGenerator::system()->generate() % (max + 1) - min) + min);
}

QString MainWindow::generateParamRange(QString type, QJsonArray &rangeArray)
{/*use in the range*/
    int index;
    QString result;
    int rangeSize = rangeArray.count();

    if(maxTestCaseCount < rangeSize)
        maxTestCaseCount = rangeSize;

    index = (paramTested < rangeArray.count()) ? paramTested : (rangeSize - 1);

    if(type == "int" )
        result = tr("%1").arg(rangeArray[index].toInt());
    else if(type == "string" )
        result = tr("%1").arg(rangeArray[index].toString());
    else if(type == "double" )
        result = tr("%1").arg(rangeArray[index].toDouble());
    else if(type == "char" )
        result = tr("%1").arg(rangeArray[index].toString().at(0).toLatin1());
    else
        qDebug() << "unknown param type!";

    return result;
}

QString MainWindow::generateParamMin2Max(int min, int max)
{/*generate value for all possible case*/
    QString result;

    if(maxTestCaseCount < ((max - min) + 1))
        maxTestCaseCount = ((max - min) + 1);

    if( (min + paramTested ) <= max)
        result = tr("%1").arg(min + paramTested);
    else
        result = generateParamRandom(min, max);

    return result;
}

bool MainWindow::readJsonData()
{
    QFile file(appConfig->value("file_path").toString());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "file can not open!";
        return false;
    }
    QTextStream in(&file);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(in.readAll().toUtf8());

    if(jsonResponse.isEmpty())
    {
        qDebug() << "no json data in file";
        return false;
    }

    m_rootObject = jsonResponse.object();
    return true;
}

void MainWindow::showOnDock(QString recvData)
{
    bool cValid = true;
    QString key = getMsgKey(recvData);

    if(!key.isEmpty())
    {
        QJsonObject paramObj = m_rootObject[key].toObject();
        QJsonArray recvArray = paramObj["recv_params"].toArray();
        QJsonArray recvFormat = paramObj["recv_format"].toArray();

        if(!recvArray.isEmpty())
        {
            /**/
            QString recvFmt;

            responseDetail->clear();
            QTreeWidgetItem *rootItem = new QTreeWidgetItem(responseDetail);
            rootItem->setData(DockName, Qt::DisplayRole, key.toUpper());

            rootItem->setExpanded(true);

            for (int var = 0; var < recvArray.count(); ++var)
            {
                tsParam param;
                QJsonObject pObj = recvArray.at(var).toObject();
                recvFmt.append(recvFormat.at(var).toString());

                param.status = analizeParameter(recvData, pObj, recvFmt, param);

                addNewLine(rootItem, param);

                cValid = param.status ? cValid : param.status;
            }

            rootItem->setData(DockDescripton, Qt::DisplayRole, cValid ? "Success" : "Fail");
            rootItem->setData(DockDescripton, Qt::TextColorRole, cValid ? QColor(Qt::darkGreen) : QColor(Qt::red));
            /**/
            responseDetail->insertTopLevelItem(responseDetail->topLevelItemCount()+1, rootItem);
        }
        else
            qDebug() << "recvArray or recvFormat is Empty";
    }
    else
        qDebug() << "Not Defined Message Received";
}

void MainWindow::createDockWindows()
{
    QStringList headers;
    QStringList ColumnNames;

    headers << tr("Title") << tr("Description");
    ColumnNames << "Name" << "Value" << "Status";

    responseDetail = new QTreeWidget();
    responseDetail->setHeaderLabels(ColumnNames);
    responseDetail->setAlternatingRowColors(true);

    QDockWidget *dock = new QDockWidget(tr("Response Message Detail"), this);
    dock->setWidget(responseDetail);
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    connect(ui->pinButton, &QPushButton::clicked, [=](){
        if(ui->pinButton->isChecked())
            dock->hide();
        else
            dock->show();
    });
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    QString setResponse = current->data(ReceiveSetColumn, Qt::DisplayRole).toString();
    QString getResponse = current->data(ReceiveGetColumn, Qt::DisplayRole).toString();

    if(!setResponse.isEmpty())
        showOnDock(setResponse);
    else if(!getResponse.isEmpty())
        showOnDock(getResponse);
}

QString MainWindow::getMsgKey(QString message)
{
    foreach(const QString& key, m_rootObject.keys())
    {
        if(message.contains(key))
            return key;
    }

    qDebug() << "Message is not defined; msg : " << message;
    return "";
}

bool MainWindow::analizeParameter(QString data, QJsonObject pObj, QString &fmt, tsParam &param)
{
    /*fill parameter detail*/
    param.name      = pObj["name"].toString();
    param.type      = pObj["type"].toString();
    param.format    = pObj["format"].toString();
    param.generate  = pObj["generate"].toString();
    param.factor    = pObj["lsb"].toDouble();
    param.min       = pObj["min"].toInt();
    param.max       = pObj["max"].toInt();
    param.range     = pObj["range"].toArray();
    fmt.replace(QRegExp("%\\d*\\d"), param.format);

    if(param.type == "int"){
        int tmp;

        if(extractData(data.toLatin1(), fmt.toLatin1(), tmp)){
            param.value = tmp;
            param.display = QString::number(tmp);
        }
    }
    else if(param.type == "char"){
        char tmp;

        if(extractData(data.toLatin1(), fmt.toLatin1(), tmp)){
            param.value = tmp;
            param.display = QString(tmp);
        }
    }
    else if(param.type == "double"){
        float tmp;

        if(extractData(data.toLatin1(), fmt.toLatin1(), tmp)){
            param.value = tmp;
            param.display = QString::number(tmp, 'f', 2);
        }
    }
    else if(param.type == "string"){
        char tmp[100];

        if(extractData(data.toLatin1(), fmt.toLatin1(), tmp))
        {
            param.value = QString(tmp);
            param.display = QString(tmp);
        }
    }

    if(param.generate == "range")
        param.display = Utils::getItem(param);

    fmt.replace(QRegExp("%(\?!\\*)"), "%*");     //%(?!\*)
    param.status = Utils::validate(param);
    return param.status;
}

void MainWindow::addNewLine(QTreeWidgetItem *rootItem, tsParam param)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
    item->setData(DockName, Qt::DisplayRole, param.name.toLocal8Bit());
    item->setData(DockValue, Qt::DisplayRole, param.display);
    item->setData(DockDescripton, Qt::DisplayRole, param.status ? "Success" : "Fail");
    item->setData(DockDescripton, Qt::TextColorRole, param.status ? QColor(Qt::darkGreen) : QColor(Qt::red));
}

void MainWindow::prepareMenu( const QPoint & pos )
{
    if(!r_menu)
    {
        r_menu = new QMenu(this);

        for (int var = 0; var < ui->treeWidget->headerItem()->columnCount(); ++var) {

            QString colName = ui->treeWidget->headerItem()->data(var, Qt::DisplayRole).toString();

            if(HiddenableColumns.contains(colName))
            {
                QAction *newAct = new QAction(QIcon(":/res/check_icon_32.png"), colName, this);

                newAct->setCheckable(true);

                r_menu->addAction(newAct);

                connect(newAct, &QAction::triggered, [=](){
                    ui->treeWidget->setColumnHidden(var, newAct->isChecked());
                    QString icon_path = ui->treeWidget->isColumnHidden(var) ? ":/res/uncheck_icon_32.png" : ":/res/check_icon_32.png";
                    newAct->setIcon(QIcon(icon_path));
                });
            }
        }
    }

    QPoint pt(pos);
    r_menu->exec( ui->treeWidget->mapToGlobal(pos) );
}
