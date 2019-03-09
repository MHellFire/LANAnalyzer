// Copyright © 2009 Mariusz Helfajer
//
// This file is part of LANAnalyzer.
//
// LANAnalyzer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LANAnalyzer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LANAnalyzer.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QtGui>

#include "settings.h"
#include "topactivedialog.h"
#include "devicesdialog.h"
#include "filtersdialog.h"
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "capturethread.h"
#include "receivercore.h"
#include "transparencydialog.h"
#include "startcapturedialog.h"
#include "eventsviewermainwindow.h"
#include "portnumbersdialog.h"
#include "netpacketsdialog.h"
#include "nettransferdialog.h"
#include "packetsmainwindow.h"
#include "netpacketsgraphdialog.h"
#include "nettransfergraphdialog.h"
#include "usertransfersgraphdialog.h"
#include "exportdatadialog.h"
#include "summarydialog.h"
#include "myoutputdialog.h"

//#include "WpdPack/Include/pcap.h"
//#include "WpdPack/Include/remote-ext.h"

/*
#ifndef WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
#else
    #include <winsock.h>
#endif
*/

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void moveEvent(QMoveEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindowClass ui;

    // general
    QSize oldSize;
    QPoint oldPosition;

    CaptureData captureData;

    bool capturing;

    // WinPcap's device
    pcap_if_t *device;

    // winsock2.h
    struct in_addr addr;

    // users IPs & names
    QList<QString> usersList;
    QList<QString> usersName;

    // users data
    QList<quint64> usersUp, usersDown,
                   usersUpPrev, usersDownPrev;
    QList<qreal> usersUpSpeed, usersDownSpeed;

    QList<Hosts> usersHosts;
    QList<Apps> usersApps;

    // network data
    quint64 netUpTotal, netDownTotal;
    quint64 netUpTotalPrev, netDownTotalPrev;

    // threads
    CaptureThread *captureThread;
    QThread *receiverThread;

    // objects
    Settings *settings;
    MyOutputDialog *myOutputDlg;
    EventsViewerMainWindow *eventsViewerMainWindow;
    ReceiverCore *receiverCore;
    NetPacketsDialog *netPacketsDlg;
    PacketsMainWindow *packetsMainWindow;
    NetTransferDialog *netTransferDlg;
    NetPacketsGraphDialog *netPacketsGraphDlg;
    NetTransferGraphDialog *netTransferGraphDlg;
    UserTransfersGraphDialog *userTransfersGraphDlg;

    // timers
    QTimer *clockTimer;
    QTimer *startCaptureTimer;

    // status bar
    QLabel *deviceLabel;
    QLabel *filterLabel;
    QLabel *infoLabel;
    QLabel *clockLabel;

    // toolbars
    // file
    QAction *exportDataAct;

    // capture
    QMenu *startMenu;
    QAction *startNowAct;
    QAction *startAct;
    QAction *stopAct;
    QAction *stopCountdownAct;
    QAction *showDeviceAct;
    QAction *showFilterAct;

    // network
    QAction *netTransferAct;
    QAction *netTransferGraphAct;
    QAction *capturedPacketsAct;

    // tools
    QAction *eventsViewerAct;
    QAction *portNumbersAct;

    // options
    QAction *settingsAct;

    // help
    QMenu *helpMenu;
    QAction *helpAct;
    QAction *aboutAct;

    // tray icon
    // actions
    QMenu *trayIconMenu;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMovie *trayIconMovie;

    // menu
    QActionGroup *styleActionGroup;
    QActionGroup *sizeActionGroup;

    //
    bool setLanguage(QString file);

    void createMenu();
    void createToolbars();
    void setToolbarIcons(int size);
    void createStatusBar();
    void createClockTimer();
    void createTrayIcon();
    void createConnections();
    void restoreWindowState();
    void createObjects();
    void clearVariables();

    void setDevice();
    void setFilter();

    void startImmediately();
    void startAt();
    void startAfter();

    void writeSettings(bool writeFile);

    QString bytesToStr(quint64 bytes);

private slots:
    // menu
    // file
    void onExportData();

    // capture
    void startCapture();
    void stopCapture();
    void stopCountdown();
    void showStartCaptureDlg();
    void showSelectDeviceDlg();
    void showSelectFilterDlg();

    // network
    void showNetPacketsDlg();
    void showNetPacketsGraphDlg();
    void showNetTransferDlg();
    void showNetTransferGraphDlg();
    void showPacketsMainWindow();

    // tools
    void showEventsViewerMainWindow();
    void showPortNumbersDlg();

    // options
    void onChangeMovable(bool movable);
    void onToolbarsStyleChanged(QAction *action);
    void onToolbarsSizeChanged(QAction *action);
    void onDefaultToolbars();
    void toggleFullScreen(bool);
    void minimizeToTray();
    void toggleAlwaysOnTop(bool);
    void showWindowOpacityDlg();
    void showSettingsDlg();

    // help
    void showHelp();
    void showAbout();

    // buttons
    // packets
    void toggleIn(bool checked);
    void toggleOut(bool checked);
    void toggleTotal(bool checked);

    // transfers
    void showTopActiveUpDlg();
    void showTopActiveDownDlg();
    void showUsersTransferGraphDlg();

    // applications
    void showTopAppUpDlg();
    void showTopAppDownDlg();
    void showTopAppUsersUpDlg();
    void showTopAppUsersDownDlg();

    // hosts
    void showTopActiveHostsUpDlg();
    void showTopActiveHostsDownDlg();
    void onHostsOpen();

    //
    void onUsersHostsChanged();
    void onUsersAppsChanged();

    // context menu
    void onApplicationsContextMenu(const QPoint &pos);
    void onHostsContextMenu(const QPoint &pos);

    // timers update
    void updateClockTimer();

    // tray icon
    void onTrayIconMovieFrameChanged(int frameNumber);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();

    //
    void setMyWindowOpacity(int value);
    void infoMessage(quint8 type, const QString &title, const QString &message);

    // from receiverCore
    void newUser(const QString &user, const QString &timeOn);
    void newUserName(const QString &user,const QString &name);

    void usersTransfer(QList<quint64> usersUp, QList<quint64> usersDown);
    void usersSpeed(QList<qreal> usersUpSpeed, QList<qreal> usersDownSpeed);

    void netTransfer(quint64 up, quint64 down);

    void netAllPackets(QList<quint32> userArp, QList<quint32> userRarp, QList<quint32> userIcmp, QList<quint32> userIgmp, QList<quint32> userTcp, QList<quint32> userUdp, QList<quint32> userOther, QList<quint32> userTotal);
    void netInPackets(QList<quint32> userArpIn, QList<quint32> userRarpIn, QList<quint32> userIcmpIn, QList<quint32> userIgmpIn, QList<quint32> userTcpIn, QList<quint32> userUdpIn, QList<quint32> userOtherIn, QList<quint32> userTotalIn);
    void netOutPackets(QList<quint32> userArpOut, QList<quint32> userRarpOut, QList<quint32> userIcmpOut, QList<quint32> userIgmpOut, QList<quint32> userTcpOut, QList<quint32> userUdpOut, QList<quint32> userOtherOut, QList<quint32> userTotalOut);

    void newUserApp(quint16 user, Apps app);
    void newUserHost(quint16 user, Hosts host);
    void newHostName(const QString &hostAddress, const QString &hostName);

    void updateUsersApps(QList<Apps> usersApps);
    void updateUsersHosts(QList<Hosts> usersHosts);
};

#endif // MAINWINDOW_H
