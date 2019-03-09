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

#include "mainwindow.h"

//=====================================================================================================================================================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    settings = new Settings();
    settings->read();

    bool setLanguageError = false;

    if (!setLanguage(settings->mainWindow.languageFile))
    {
        QMessageBox::critical(0, "Critical", "The selected language could not be set.");

        settings->mainWindow.languageFile = "English (Default)";
        setLanguageError = true;
    }

    ui.setupUi(this);

    eventsViewerMainWindow = new EventsViewerMainWindow();

    switch (settings->getError())
    {
        case 0: break;
        case 1: eventsViewerMainWindow->addEvent(EVENT_WARNING, tr("Reading settings from file"), tr("Settings file does not exist. Default settings loaded."));
                break;
        case 2: eventsViewerMainWindow->addEvent(EVENT_WARNING, tr("Reading settings from file"), tr("An access error occurred (trying to read the file). Default settings loaded."));
                break;
        case 3: eventsViewerMainWindow->addEvent(EVENT_WARNING, tr("Reading settings from file"), tr("A format error occurred (a malformed INI file) while reading the file. Some settings reset to defaults."));
                break;
        default: break;
    }

    if (setLanguageError)
    {
        eventsViewerMainWindow->addEvent(EVENT_CRITICAL, "Setting user language", "The selected language could not be set.");
    }

    qRegisterMetaType<Hosts>("Hosts");
    qRegisterMetaType<hostsList>("QList<Hosts>");

    qRegisterMetaType<Apps>("Apps");
    qRegisterMetaType<appsList>("QList<Apps>");

    qRegisterMetaType<quint32List>("QList<quint32>");
    qRegisterMetaType<quint64List>("QList<quint64>");
    qRegisterMetaType<qrealList>("QList<qreal>");

    createMenu();
    createToolbars();
    createStatusBar();
    createClockTimer();
    createTrayIcon();

    createConnections();

    restoreWindowState();

    createObjects();

    clearVariables();

    setDevice();
    setFilter();

    if (settings->mainWindow.firstRun)
    {
        settings->mainWindow.toolbarsState = saveState(9);
        settings->mainWindow.toolbarsDefault = saveState(9);

        settings->mainWindow.splitterApplication = ui.splitterApplications->saveState();
        settings->mainWindow.splitterHosts = ui.splitterHosts->saveState();

        settings->mainWindow.firstRun = false;
    }

    eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("LANAnalyzer started"), "");

    if (settings->mainWindow.minimizeToTrayOnStart)
        QTimer::singleShot(100, this, SLOT(minimizeToTray()));

    if (settings->mainWindow.autoStart && ui.actionStartNow->isEnabled())
        QTimer::singleShot(150, this, SLOT(startCapture()));
}

MainWindow::~MainWindow()
{
    writeSettings(true);

    delete captureThread;

    if (receiverThread->isRunning())
        receiverThread->quit();
    receiverThread->wait(1000);
    delete receiverThread;

    delete receiverCore;

    delete packetsMainWindow;

    delete netPacketsDlg;
    delete netTransferDlg;
    delete netPacketsGraphDlg;
    delete netTransferGraphDlg;

    delete userTransfersGraphDlg;

    eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("LANAnalyzer closed"), tr(""));
    delete eventsViewerMainWindow;

    delete settings;
}

//=====================================================================================================================================================================================================

void MainWindow::resizeEvent(QResizeEvent *event)
{
    oldSize = event->oldSize();
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    oldPosition = event->oldPos();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (settings->mainWindow.minimizeToTrayOnClose)
    {
        if (QSystemTrayIcon::isSystemTrayAvailable())
        {
            if (trayIcon->isVisible())
            {
                //QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. "
                //                                                 "To terminate the program choose <b>Quit</b> "
                //                                                 "in the context menu of the system tray entry or "
                //                                                 "in \"File\" menu of main window."));

                hide();

                //trayIcon->showMessage(tr("LANAnalyzer tray icon"), tr("Click here to restore application"), QSystemTrayIcon::Information, 3000);

                event->ignore();
            }
            else
            {
                ui.actionTrayIcon->toggle();

                //QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. "
                //                                                 "To terminate the program choose <b>Quit</b> "
                //                                                 "in the context menu of the system tray entry or "
                //                                                 "in \"File\" menu of main window."));

                hide();

                event->ignore();
            }
        }
        else
        {
            QMessageBox::critical(this, tr("Critical"), tr("The system tray is not available."));
            eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Minimize to tray on close"), tr("The system tray is not available."));
        }
    }
    else
    {
        if (capturing)
        {
            QMessageBox msgBox(QMessageBox::Question, tr("Question"),tr("Capturing packets. Are you sure you want to quit?"), 0, this);
            msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
            msgBox.setDefaultButton(msgBox.addButton(tr("&No"), QMessageBox::RejectRole));

            if (msgBox.exec() != QMessageBox::AcceptRole)
            {
                event->ignore();
                return;
            }
        }

        event->accept();
        qApp->quit();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Ctrl + Alt + D
    if ((modifiers & Qt::ControlModifier) && (modifiers & Qt::AltModifier) && (event->key() == Qt::Key_D))
    {
        myOutputDlg->show();
        myOutputDlg->raise();
        myOutputDlg->activateWindow();
    }

    event->accept();
}

//=====================================================================================================================================================================================================

bool MainWindow::setLanguage(QString file)
{
    if (file.compare("English (Default)", Qt::CaseSensitive) != 0)
    {
        // our application translation
        QTranslator *appTranslator = new QTranslator();
        if (!appTranslator->load(QCoreApplication::applicationDirPath() + QString("/languages/") + file))
            return false;
        qApp->installTranslator(appTranslator);

        // qt libraries translation
        QTranslator *qtTranslator = new QTranslator();
        if (qtTranslator->load(QCoreApplication::applicationDirPath() + QString("/languages/qt_") + file))
            qApp->installTranslator(qtTranslator);
    }

    return true;
}

//=====================================================================================================================================================================================================

void MainWindow::createMenu()
{
    // file
    connect(ui.actionExportData, SIGNAL(triggered()), this, SLOT(onExportData()));
    connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

    // capture
    connect(ui.actionStartNow, SIGNAL(triggered()), this, SLOT(startCapture()));
    connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(showStartCaptureDlg()));
    connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(stopCapture()));
    connect(ui.actionStopCountdown, SIGNAL(triggered()), this, SLOT(stopCountdown()));
    connect(ui.actionDevice, SIGNAL(triggered()), this, SLOT(showSelectDeviceDlg()));
    connect(ui.actionFilter, SIGNAL(triggered()), this, SLOT(showSelectFilterDlg()));

    // network
    connect(ui.actionNetworkPacketsStatistics, SIGNAL(triggered()), this, SLOT(showNetPacketsDlg()));
    connect(ui.actionNetworkPacketsGraph, SIGNAL(triggered()), this, SLOT(showNetPacketsGraphDlg()));
    connect(ui.actionNetworkTransferStatistics, SIGNAL(triggered()), this, SLOT(showNetTransferDlg()));
    connect(ui.actionNetworkTransferGraph, SIGNAL(triggered()), this, SLOT(showNetTransferGraphDlg()));
    connect(ui.actionCapturedPackets, SIGNAL(triggered()), this, SLOT(showPacketsMainWindow()));

    // tools
    connect(ui.actionEventsViewer, SIGNAL(triggered()), this, SLOT(showEventsViewerMainWindow()));
    connect(ui.actionPortNumbers, SIGNAL(triggered()), this, SLOT(showPortNumbersDlg()));

    // options
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarFile->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarCapture->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarNetwork->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarTools->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarOptions->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarHelp->toggleViewAction());
    ui.menuToolbars->insertSeparator(ui.actionMovable);

    connect(ui.actionMovable, SIGNAL(toggled(bool)), this, SLOT(onChangeMovable(bool)));

    styleActionGroup = new QActionGroup(ui.menuStyle);
    styleActionGroup->setExclusive(true);
    connect(styleActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onToolbarsStyleChanged(QAction*)));

    ui.actionIconOnly->setActionGroup(styleActionGroup);
    ui.actionTextOnly->setActionGroup(styleActionGroup);
    ui.actionTextBesideIcon->setActionGroup(styleActionGroup);
    ui.actionTextUnderIcon->setActionGroup(styleActionGroup);

    sizeActionGroup = new QActionGroup(ui.menuSize);
    sizeActionGroup->setExclusive(true);
    connect(sizeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onToolbarsSizeChanged(QAction*)));

    ui.action16->setActionGroup(sizeActionGroup);
    ui.action24->setActionGroup(sizeActionGroup);
    ui.action32->setActionGroup(sizeActionGroup);

    connect(ui.actionDefault, SIGNAL(triggered()), this, SLOT(onDefaultToolbars()));

    connect(ui.actionStatusBar, SIGNAL(toggled(bool)), ui.statusbar, SLOT(setVisible(bool)));
    //ui.actionTrayIcon connection -> void createTrayIcon();
    connect(ui.actionFullScreen, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen(bool)));
    connect(ui.actionAlwaysOnTop, SIGNAL(triggered(bool)), this, SLOT(toggleAlwaysOnTop(bool)));
    connect(ui.actionTransparency, SIGNAL(triggered()), this, SLOT(showWindowOpacityDlg()));
    connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(showSettingsDlg()));

    // help
    connect(ui.actionContents, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
}

//=====================================================================================================================================================================================================

void MainWindow::createToolbars()
{
    // file
    exportDataAct = new QAction(tr("Export data"), this);
    ui.toolBarFile->addAction(exportDataAct);
    exportDataAct->setEnabled(true);
    connect(exportDataAct, SIGNAL(triggered()), this, SLOT(onExportData()));

    // capture
    startNowAct = new QAction(tr("Start now"), this);
    startNowAct->setDisabled(true);
    connect(startNowAct, SIGNAL(triggered()), this, SLOT(startCapture()));

    startAct = new QAction(tr("Start"), this);
    startAct->setDisabled(true);
    connect(startAct, SIGNAL(triggered()), this, SLOT(showStartCaptureDlg()));

    startMenu = new QMenu(this);
    startMenu->addAction(startAct);
    startNowAct->setMenu(startMenu);
    ui.toolBarCapture->addAction(startNowAct);

    stopAct = new QAction(tr("Stop"), this);
    ui.toolBarCapture->addAction(stopAct);
    stopAct->setDisabled(true);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stopCapture()));

    stopCountdownAct = new QAction(tr("Stop countdown"), this);
    ui.toolBarCapture->addAction(stopCountdownAct);
    stopCountdownAct->setDisabled(true);
    connect(stopCountdownAct, SIGNAL(triggered()), this, SLOT(stopCountdown()));

    ui.toolBarCapture->addSeparator();

    showDeviceAct = new QAction(tr("Device"), this);
    ui.toolBarCapture->addAction(showDeviceAct);
    showDeviceAct->setEnabled(true);
    connect(showDeviceAct, SIGNAL(triggered()), this, SLOT(showSelectDeviceDlg()));

    showFilterAct = new QAction(tr("Filter"), this);
    ui.toolBarCapture->addAction(showFilterAct);
    showFilterAct->setEnabled(true);
    connect(showFilterAct, SIGNAL(triggered()), this, SLOT(showSelectFilterDlg()));

    // network
    netTransferAct = new QAction(tr("Transfer"), this);
    ui.toolBarNetwork->addAction(netTransferAct);
    netTransferAct->setEnabled(true);
    connect(netTransferAct, SIGNAL(triggered()), this, SLOT(showNetTransferDlg()));

    netTransferGraphAct = new QAction(tr("Transfer graph"), this);
    ui.toolBarNetwork->addAction(netTransferGraphAct);
    netTransferGraphAct->setEnabled(true);
    connect(netTransferGraphAct, SIGNAL(triggered()), this, SLOT(showNetTransferGraphDlg()));

    ui.toolBarNetwork->addSeparator();

    capturedPacketsAct = new QAction(tr("Packets"), this);
    ui.toolBarNetwork->addAction(capturedPacketsAct);
    capturedPacketsAct->setEnabled(true);
    connect(capturedPacketsAct, SIGNAL(triggered()), this, SLOT(showPacketsMainWindow()));

    // tools
    eventsViewerAct = new QAction(tr("Events"), this);
    ui.toolBarTools->addAction(eventsViewerAct);
    eventsViewerAct->setEnabled(true);
    connect(eventsViewerAct, SIGNAL(triggered()), this, SLOT(showEventsViewerMainWindow()));

    ui.toolBarTools->addSeparator();

    portNumbersAct = new QAction(tr("Ports"), this);
    ui.toolBarTools->addAction(portNumbersAct);
    portNumbersAct->setEnabled(true);
    connect(portNumbersAct, SIGNAL(triggered()), this, SLOT(showPortNumbersDlg()));

    // options
    settingsAct = new QAction(tr("Settings"), this);
    ui.toolBarOptions->addAction(settingsAct);
    settingsAct->setEnabled(true);
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(showSettingsDlg()));

    // help
    helpAct = new QAction(tr("Help"), this);
    helpAct->setEnabled(true);
    connect(helpAct, SIGNAL(triggered()), this, SLOT(showHelp()));

    aboutAct = new QAction(tr("About"), this);
    aboutAct->setEnabled(true);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(showAbout()));

    helpMenu = new QMenu(this);
    helpMenu->addAction(aboutAct);
    helpAct->setMenu(helpMenu);
    ui.toolBarHelp->addAction(helpAct);

    restoreState(settings->mainWindow.toolbarsState, 9);

    ui.actionMovable->setChecked(settings->mainWindow.toolbarsMovable);

    switch (settings->mainWindow.toolbarsStyle)
    {
        case 0: ui.actionIconOnly->trigger(); break;
        case 1: ui.actionTextOnly->trigger(); break;
        case 2: ui.actionTextBesideIcon->trigger(); break;
        case 3: ui.actionTextUnderIcon->trigger(); break;
        default: break;
    }

    switch (settings->mainWindow.toolbarsIconSize)
    {
        case 16: ui.action16->trigger(); break;
        case 24: ui.action24->trigger(); break;
        case 32: ui.action32->trigger(); break;
        default: break;
    }
}

void MainWindow::onChangeMovable(bool movable)
{
    ui.toolBarFile->setMovable(movable);
    ui.toolBarCapture->setMovable(movable);
    ui.toolBarNetwork->setMovable(movable);
    ui.toolBarTools->setMovable(movable);
    ui.toolBarOptions->setMovable(movable);
    ui.toolBarHelp->setMovable(movable);
}

void MainWindow::onToolbarsStyleChanged(QAction *action)
{
    if (ui.actionIconOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarCapture->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarNetwork->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarTools->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarOptions->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    if (ui.actionTextOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarCapture->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarNetwork->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarTools->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarOptions->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    if (ui.actionTextBesideIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarCapture->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarNetwork->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarTools->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarOptions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    if (ui.actionTextUnderIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarCapture->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarNetwork->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarTools->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarOptions->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
}

void MainWindow::onToolbarsSizeChanged(QAction *action)
{
    int size = 24;

    if (action->text().contains("16")) size = 16;
    if (action->text().contains("24")) size = 24;
    if (action->text().contains("32")) size = 32;

    ui.toolBarFile->setIconSize(QSize(size, size));
    ui.toolBarCapture->setIconSize(QSize(size, size));
    ui.toolBarNetwork->setIconSize(QSize(size, size));
    ui.toolBarTools->setIconSize(QSize(size, size));
    ui.toolBarOptions->setIconSize(QSize(size, size));
    ui.toolBarHelp->setIconSize(QSize(size, size));

    setToolbarIcons(size);
}

void MainWindow::setToolbarIcons(int size)
{
    // file
    exportDataAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_export.png")));

    // capture
    startNowAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_start_now.png")));
    startAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_start.png")));
    stopAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_stop.png")));
    stopCountdownAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_stop_countdown.png")));
    showDeviceAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_device.png")));
    showFilterAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_filter.png")));

    // network
    netTransferAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_up_down.png")));
    netTransferGraphAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_graph.png")));
    capturedPacketsAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_packet.png")));

    // tools
    eventsViewerAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_viewer.png")));
    portNumbersAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_ports.png")));

    // options
    settingsAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_settings.png")));

    // help
    helpAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_help.png")));
    aboutAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_about.png")));
}

void MainWindow::onDefaultToolbars()
{
    restoreState(settings->mainWindow.toolbarsDefault, 9);
    ui.actionMovable->setChecked(true);
    ui.actionTextUnderIcon->trigger();
    ui.action24->trigger();
}

//=====================================================================================================================================================================================================

void MainWindow::createStatusBar()
{
    ui.statusbar->addPermanentWidget(infoLabel = new QLabel(this), 1);
    ui.statusbar->addPermanentWidget(deviceLabel = new QLabel(this), 1);
    ui.statusbar->addPermanentWidget(filterLabel = new QLabel(this), 1);
    ui.statusbar->addPermanentWidget(clockLabel = new QLabel(this));
}

//=====================================================================================================================================================================================================

void MainWindow::createClockTimer()
{
    clockLabel->setText(QTime::currentTime().toString("hh:mm:ss"));

    clockTimer = new QTimer(this);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(updateClockTimer()));
    clockTimer->start(1000);
}

void MainWindow::updateClockTimer()
{
    clockLabel->setText(QTime::currentTime().toString("hh:mm:ss"));
    clockTimer->start(1000);
}

//=====================================================================================================================================================================================================

void MainWindow::createTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, tr("Critical"), tr("No system tray detected on this system.\n"
                                                    "The tray icon option will be disabled."));

        eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Creating tray icon"), tr("No system tray detected on this system. The tray icon option will be disabled."));

        ui.actionTrayIcon->setDisabled(true);
        ui.actionMinimizeToTray->setDisabled(true);
    }
    else
    {
        trayIcon = new QSystemTrayIcon(this);
        // On Windows, the system tray icon size is 16x16; on X11, the preferred size is 22x22.
        // The icon will be scaled to the appropriate size as necessary.
        trayIcon->setIcon(QIcon(":/images/lananalyzer.png"));
        trayIcon->setToolTip(tr("LANAnalyzer"));

        trayIconMenu = new QMenu(this);

        quitAction = new QAction(tr("Quit"), this);
        trayIconMenu->addAction(quitAction);
        connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
        trayIcon->setContextMenu(trayIconMenu);

        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));

        trayIconMovie = new QMovie(":/images/working.gif");
        connect(trayIconMovie, SIGNAL(frameChanged(int)), this, SLOT(onTrayIconMovieFrameChanged(int)));

        connect(ui.actionTrayIcon, SIGNAL(toggled(bool)), trayIcon, SLOT(setVisible(bool)));
        connect(ui.actionMinimizeToTray, SIGNAL(triggered()), this, SLOT(minimizeToTray()));
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger: isVisible() ? hide() : show(); break;
        case QSystemTrayIcon::DoubleClick: break;
        case QSystemTrayIcon::MiddleClick: break;
        default: break;
    }
}

void MainWindow::messageClicked()
{
    if (!isVisible())
        show();
}

void MainWindow::onTrayIconMovieFrameChanged(int frameNumber)
{
    trayIcon->setIcon(trayIconMovie->currentPixmap());
    //setWindowIcon(trayIconMovie->currentPixmap());
}

void MainWindow::minimizeToTray()
{
    if (!trayIcon->isVisible())
        ui.actionTrayIcon->toggle();

    if (trayIcon->isVisible())
        hide();

    //trayIcon->showMessage(tr("LANAnalyzer tray icon"), tr("Click here to restore application"), QSystemTrayIcon::Information, 3000);

    // QSystemTrayIcon::NoIcon       0   No icon is shown.
    // QSystemTrayIcon::Information  1   An information icon is shown.
    // QSystemTrayIcon::Warning      2   A standard warning icon is shown.
    // QSystemTrayIcon::Critical     3   A critical warning icon is shown.
}

//=====================================================================================================================================================================================================

void MainWindow::createConnections()
{
    connect(ui.pushButtonIn, SIGNAL(toggled(bool)), this, SLOT(toggleIn(bool)));
    connect(ui.pushButtonOut, SIGNAL(toggled(bool)), this, SLOT(toggleOut(bool)));
    connect(ui.pushButtonTotal, SIGNAL(toggled(bool)), this, SLOT(toggleTotal(bool)));

    connect(ui.pushButtonTopActiveUp, SIGNAL(clicked()), this, SLOT(showTopActiveUpDlg()));
    connect(ui.pushButtonTopActiveDown, SIGNAL(clicked()), this, SLOT(showTopActiveDownDlg()));
    connect(ui.pushButtonUsersGraph, SIGNAL(clicked()), this, SLOT(showUsersTransferGraphDlg()));

    connect(ui.pushButtonAppTopUp, SIGNAL(clicked()), this, SLOT(showTopAppUpDlg()));
    connect(ui.pushButtonAppTopDown, SIGNAL(clicked()), this, SLOT(showTopAppDownDlg()));
    connect(ui.pushButtonAppTopUsersUp, SIGNAL(clicked()), this, SLOT(showTopAppUsersUpDlg()));
    connect(ui.pushButtonAppTopUsersDown, SIGNAL(clicked()), this, SLOT(showTopAppUsersDownDlg()));

    connect(ui.pushButtonTopHostsUp, SIGNAL(clicked()), this, SLOT(showTopActiveHostsUpDlg()));
    connect(ui.pushButtonTopHostsDown ,SIGNAL(clicked()), this, SLOT(showTopActiveHostsDownDlg()));
    connect(ui.pushButtonOpen, SIGNAL(clicked()), this, SLOT(onHostsOpen()));

    connect(ui.treeWidgetUsersHosts, SIGNAL(itemSelectionChanged()), this, SLOT(onUsersHostsChanged()));
    connect(ui.treeWidgetUsersApp, SIGNAL(itemSelectionChanged()), this, SLOT(onUsersAppsChanged()));

    connect(ui.treeWidgetApp, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onApplicationsContextMenu(QPoint)));
    connect(ui.treeWidgetHosts, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onHostsContextMenu(QPoint)));
}

//=====================================================================================================================================================================================================

void MainWindow::restoreWindowState()
{
    resize(settings->mainWindow.size);
    move(settings->mainWindow.position);

    if (settings->mainWindow.alwaysOnTop)
    {
        Qt::WindowFlags flags = windowFlags();
        flags.testFlag(Qt::WindowStaysOnTopHint) ? flags ^= Qt::WindowStaysOnTopHint : flags |= Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        ui.actionAlwaysOnTop->setChecked(true);
    }

    if (settings->mainWindow.startMaximized || settings->mainWindow.maximized)
    {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }

    if (settings->mainWindow.fullScreen)
    {
        ui.actionAlwaysOnTop->setDisabled(true);
        setWindowState(windowState() ^ Qt::WindowFullScreen);
        ui.actionFullScreen->setChecked(true);
    }

    ui.actionStatusBar->setChecked(settings->mainWindow.statusBar);
    ui.actionTrayIcon->setChecked(settings->mainWindow.trayIcon);

    ui.tabWidget->setCurrentIndex(settings->mainWindow.currentTab);
    ui.splitterApplications->restoreState(settings->mainWindow.splitterApplication);
    ui.splitterHosts->restoreState(settings->mainWindow.splitterHosts);
}

//=====================================================================================================================================================================================================

void MainWindow::createObjects()
{
    capturing = false;

    captureData.durationChoice = 0;
    captureData.durationValue = 0;
    captureData.startChoice = 0;
    captureData.startValue = 0;

    captureThread = new CaptureThread(this);
    connect(captureThread, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));
    connect(captureThread, SIGNAL(breakThread()), this, SLOT(stopCapture()));

    packetsMainWindow = new PacketsMainWindow(0, captureThread);

    receiverThread = new QThread(this);
    receiverCore = new ReceiverCore(0, captureThread);
    receiverCore->moveToThread(receiverThread);

    connect(receiverCore, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(signalNewUser(QString,QString)), this, SLOT(newUser(QString,QString)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(signalNewUserName(QString,QString)), this, SLOT(newUserName(QString,QString)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(signalUsersTransfer(QList<quint64>,QList<quint64>)), this, SLOT(usersTransfer(QList<quint64>,QList<quint64>)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(signalUsersSpeed(QList<qreal>,QList<qreal>)), this, SLOT(usersSpeed(QList<qreal>,QList<qreal>)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(netAllPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), this, SLOT(netAllPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(netInPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), this, SLOT(netInPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(netOutPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), this, SLOT(netOutPackets(QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>,QList<quint32>)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(signalNewUserApp(quint16,Apps)), this, SLOT(newUserApp(quint16,Apps)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(signalNewUserHost(quint16,Hosts)), this, SLOT(newUserHost(quint16,Hosts)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(signalNewHostName(QString,QString)), this, SLOT(newHostName(QString,QString)), Qt::QueuedConnection);

    connect(receiverCore, SIGNAL(signalUsersApps(QList<Apps>)), this, SLOT(updateUsersApps(QList<Apps>)), Qt::QueuedConnection);
    connect(receiverCore, SIGNAL(signalUsersHosts(QList<Hosts>)), this, SLOT(updateUsersHosts(QList<Hosts>)), Qt::QueuedConnection);

    myOutputDlg = new MyOutputDialog(this, receiverCore);

    netPacketsDlg = new NetPacketsDialog(this, receiverCore);
    netTransferDlg = new NetTransferDialog(this, receiverCore);
    netTransferDlg->setScale(settings->netTransferDialog.up, settings->netTransferDialog.down);

    netPacketsGraphDlg = new NetPacketsGraphDialog(0, receiverCore);
    netTransferGraphDlg = new NetTransferGraphDialog(0,receiverCore);
    userTransfersGraphDlg = new UserTransfersGraphDialog(0, receiverCore);

    ui.treeWidgetPackets->setHeaderHidden(false);
    ui.treeWidgetTransfer->setHeaderHidden(false);
    ui.treeWidgetUsersApp->setHeaderHidden(false);
    ui.treeWidgetApp->setHeaderHidden(false);
    ui.treeWidgetUsersHosts->setHeaderHidden(false);
    ui.treeWidgetHosts->setHeaderHidden(false);

    ui.pushButtonIn->setChecked(false);
    ui.pushButtonOut->setChecked(false);

    receiverThread->start();
}

//=====================================================================================================================================================================================================

void MainWindow::clearVariables()
{
    ui.treeWidgetUsers->clear();
    ui.treeWidgetPackets->clear();
    ui.treeWidgetTransfer->clear();
    ui.treeWidgetUsersApp->clear();
    ui.treeWidgetApp->clear();
    ui.treeWidgetUsersHosts->clear();
    ui.treeWidgetHosts->clear();

    usersList.clear();
    usersName.clear();

    usersUp.clear();
    usersDown.clear();
    usersUpSpeed.clear();
    usersDownSpeed.clear();

    usersHosts.clear();
    usersApps.clear();

    netUpTotal = 0;
    netDownTotal = 0;
}

//=====================================================================================================================================================================================================

void MainWindow::setDevice()
{
    DevicesDialog dlg(this);

    connect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));

    if (dlg.obtainAdapters(settings->mainWindow.deviceName))
    {
        if (dlg.execute())
        {
            device = dlg.device;
            settings->mainWindow.deviceName = device->name;

            quint32 netMask = 0xffffff, pcIP = 0;
            pcap_addr_t *a;
            for (a = device->addresses; a; a = a->next)
            {
                if (a->addr->sa_family == AF_INET)
                {
                    if (a->addr)
                        pcIP = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
                    else
                        pcIP = 0;

                    netMask = ((struct sockaddr_in *)a->netmask)->sin_addr.s_addr;

                    break;
                }
            }
            receiverCore->setData(netMask, pcIP);

            ui.actionStartNow->setEnabled(true);
            startNowAct->setEnabled(true);

            ui.actionStart->setEnabled(true);
            startAct->setEnabled(true);

            infoLabel->setText(tr("Ready to start"));
            deviceLabel->setText(tr("Device: %1").arg(device->description ? device->description : device->name));
            eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Device (automatically from settings) successful set"), tr("Device name: %1").arg(device->description ? device->description : device->name));
        }
        else
        {
            deviceLabel->setText(tr("Device: not selected"));
            infoLabel->setText(tr("Please select device"));
        }
    }
    else
    {
        deviceLabel->setText(tr("Device: not selected"));
        infoLabel->setText(tr("Please select device"));
    }

    disconnect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));
}

void MainWindow::showSelectDeviceDlg()
{
    DevicesDialog dlg(this);

    connect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));

    if (dlg.obtainAdapters(settings->mainWindow.deviceName))
    {
        if (dlg.exec())
        {
            if (!capturing)
            {
                device = dlg.device;
                settings->mainWindow.deviceName = device->name;

                quint32 netMask = 0xffffff, pcIP = 0;
                pcap_addr_t *a;
                for (a = device->addresses; a; a = a->next)
                {
                    if (a->addr->sa_family == AF_INET)
                    {
                        if (a->addr)
                            pcIP = ((struct sockaddr_in *)a->addr)->sin_addr.s_addr;
                        else
                            pcIP = 0;

                        netMask = ((struct sockaddr_in *)a->netmask)->sin_addr.s_addr;

                        break;
                    }
                }
                receiverCore->setData(netMask, pcIP);

                ui.actionStartNow->setEnabled(true);
                startNowAct->setEnabled(true);

                ui.actionStart->setEnabled(true);
                startAct->setEnabled(true);

                infoLabel->setText(tr("Ready to start"));
                deviceLabel->setText(tr("Device: %1").arg(device->description ? device->description : device->name));
                eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Device successful selected by user"), tr("Device name: %1").arg(device->description ? device->description : device->name));
            }
            else
            {
                if (dlg.device->name != settings->mainWindow.deviceName)
                {
                    QMessageBox::information(this, tr("Information"), tr("You can not change the device during capture running."));
                }
            }

        }
    }

    disconnect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));
}

//=====================================================================================================================================================================================================

void MainWindow::setFilter()
{
    filterLabel->setText(tr("Filter: %1").arg(settings->mainWindow.filterName));
    eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Filter (automatically from settings) successful set"), tr("Filter name: %1").arg(settings->mainWindow.filterName));
}

void MainWindow::showSelectFilterDlg()
{
    FiltersDialog dlg(this, settings->mainWindow.filterName);

    connect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));

    dlg.prepareDialog();

    if (dlg.exec())
    {
        if (!capturing)
        {
            settings->mainWindow.filterCode = dlg.getFilterCode();
            settings->mainWindow.filterName = dlg.getFilterName();

            filterLabel->setText(tr("Filter: %1").arg(settings->mainWindow.filterName));
            eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Filter successful selected by user"), tr("Filter name: %1").arg(settings->mainWindow.filterName));
        }
        else
        {
            if (dlg.getFilterCode() != settings->mainWindow.filterCode)
            {
                QMessageBox::information(this, tr("Information"), tr("You can not change the filter during capture running."));
            }
        }
    }

    disconnect(&dlg, SIGNAL(infoMessage(quint8,QString,QString)), this, SLOT(infoMessage(quint8,QString,QString)));
}

//=====================================================================================================================================================================================================

void MainWindow::showStartCaptureDlg()
{
    StartCaptureDialog dlg(this);

    dlg.setCaptureData(captureData);

    if (dlg.exec())
    {
        captureData = dlg.getCaptureData();

        switch (captureData.startChoice)
        {
            case 1: startImmediately();
                    break;
            case 2: startAt();
                    break;
            case 3: startAfter();
                    break;
            default: break;
        }
    }
}

void MainWindow::startImmediately()
{
    startCapture();
}

void MainWindow::startAt()
{
    ushort h, m, s;
    h = captureData.startValue / 60 / 60;
    m = (captureData.startValue - (h * 60 * 60)) / 60;
    s = (captureData.startValue - (h * 60 * 60) - (m * 60));
    QTime startTime(h, m, s, 0);

    int startAfterSec = QTime::currentTime().secsTo(startTime);

    if (startAfterSec < 0)
        startAfterSec += 86400; // 86400 seconds = 24 houres -> next day

    startCaptureTimer = new QTimer(this);
    connect(startCaptureTimer, SIGNAL(timeout()), this, SLOT(startCapture()));
    startCaptureTimer->setSingleShot(true);
    startCaptureTimer->start(startAfterSec * 1000);

    infoLabel->setText(tr("Capture starts at %1").arg(startTime.toString("hh:mm:ss")));

    ui.actionStartNow->setDisabled(true);
    startNowAct->setDisabled(true);
    ui.actionStart->setDisabled(true);
    startAct->setDisabled(true);
    ui.actionStopCountdown->setEnabled(true);
    stopCountdownAct->setEnabled(true);
}

void MainWindow::startAfter()
{
    int startAfterSec = captureData.startValue;

    startCaptureTimer = new QTimer(this);
    connect(startCaptureTimer, SIGNAL(timeout()), this, SLOT(startCapture()));
    startCaptureTimer->setSingleShot(true);
    startCaptureTimer->start(startAfterSec * 1000);

    QTime startTime = QTime::currentTime().addSecs(startAfterSec);
    infoLabel->setText(tr("Capture starts at %1").arg(startTime.toString("hh:mm:ss")));

    ui.actionStartNow->setDisabled(true);
    ui.actionStart->setDisabled(true);
    startNowAct->setDisabled(true);
    startAct->setDisabled(true);
    ui.actionStopCountdown->setEnabled(true);
    stopCountdownAct->setEnabled(true);
}

void MainWindow::startCapture()
{
    if (captureData.startChoice == 2 || captureData.startChoice == 3)
    {
        startCaptureTimer->stop();
        disconnect(startCaptureTimer, SIGNAL(timeout()), this, SLOT(startCapture()));
        delete startCaptureTimer;
    }

    qint32 packetsLimit = -1;

    if (captureData.durationChoice == 1)
        packetsLimit = -1;

    if (captureData.durationChoice == 3)
        packetsLimit = captureData.durationValue;

    if (captureThread->startCapture(device, settings->captureThread.mode, settings->captureThread.bytes, settings->captureThread.timeout, settings->mainWindow.filterCode, packetsLimit))
    {
        myOutputDlg->clear();

        netPacketsGraphDlg->startGraph();
        netTransferGraphDlg->startGraph();
        userTransfersGraphDlg->startGraph();

        clearVariables();

        packetsMainWindow->clearTree();

        // reset scale and data
        netTransferDlg->setScale(settings->netTransferDialog.up, settings->netTransferDialog.down);

        trayIconMovie->start();
        trayIcon->setToolTip(tr("LANAnalyzer\nCapturing packets..."));

        infoLabel->setText(tr("Capturing packets..."));
        eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Capture started"), "");

        ui.actionStartNow->setDisabled(true);
        startNowAct->setDisabled(true);

        ui.actionStart->setDisabled(true);
        startAct->setDisabled(true);

        ui.actionStop->setEnabled(true);
        stopAct->setEnabled(true);

        ui.actionStopCountdown->setDisabled(true);
        stopCountdownAct->setDisabled(true);

        capturing = true;

        if (captureData.durationChoice == 3)
            infoLabel->setText(tr("Capturing packets... (until %1 packets)").arg(captureData.durationValue));

        if (captureData.durationChoice == 2)
        {
            ushort h, m, s;
            h = captureData.durationValue / 60 / 60;
            m = (captureData.durationValue - (h * 60 * 60)) / 60;
            s = (captureData.durationValue - (h * 60 * 60) - (m * 60));
            QTime stopTime(h, m, s, 0);

            int stopAfterSec = QTime::currentTime().secsTo(stopTime);

            if (stopAfterSec < 0)
                stopAfterSec += 86400;  // 86400 seconds = 24 houres -> next day

            QTimer::singleShot(stopAfterSec * 1000, this, SLOT(stopCapture()));

            infoLabel->setText(tr("Capturing packets... (until %1)").arg(stopTime.toString("hh:mm:ss")));
        }

        if (captureData.durationChoice == 4)
        {
            QTimer::singleShot(captureData.durationValue * 1000, this, SLOT(stopCapture()));

            QTime stopTime = QTime::currentTime().addSecs(captureData.durationValue);
            infoLabel->setText(tr("Capturing packets... (until %1)").arg(stopTime.toString("hh:mm:ss")));
        }
    }
    else
    {
        // if started auto after/at time
        if (stopCountdownAct->isEnabled())
        {
            ui.actionStartNow->setEnabled(true);
            startNowAct->setEnabled(true);

            ui.actionStart->setEnabled(true);
            startAct->setEnabled(true);

            ui.actionStop->setDisabled(true);
            stopAct->setDisabled(true);

            ui.actionStopCountdown->setDisabled(true);
            stopCountdownAct->setDisabled(true);
        }

        eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Capture not started"), tr("Capture thread error"));
        infoLabel->setText(tr("Capture not started (error)"));
    }
}

void MainWindow::stopCapture()
{
    if (captureThread->stopCapture())
    {
        netPacketsGraphDlg->stopGraph();
        netTransferGraphDlg->stopGraph();
        userTransfersGraphDlg->stopGraph();

        trayIconMovie->stop();
        trayIcon->setIcon(QIcon(":/images/lananalyzer.png"));
        trayIcon->setToolTip(tr("LANAnalyzer"));

        ui.actionStartNow->setEnabled(true);
        startNowAct->setEnabled(true);

        ui.actionStart->setEnabled(true);
        startAct->setEnabled(true);

        ui.actionStop->setDisabled(true);
        stopAct->setDisabled(true);

        infoLabel->setText(tr("Ready to start"));

        eventsViewerMainWindow->addEvent(EVENT_INFORMATION, tr("Capture stoped"), "");

        capturing = false;
    }
}

void MainWindow::stopCountdown()
{
    if (startCaptureTimer->isActive())
    {
        startCaptureTimer->stop();
        disconnect(startCaptureTimer, SIGNAL(timeout()), this, SLOT(startCapture()));
        delete startCaptureTimer;
    }

    ui.actionStop->setDisabled(true);
    stopAct->setDisabled(true);

    ui.actionStopCountdown->setDisabled(true);
    stopCountdownAct->setDisabled(true);

    ui.actionStartNow->setEnabled(true);
    startNowAct->setEnabled(true);

    ui.actionStart->setEnabled(true);
    startAct->setEnabled(true);

    infoLabel->setText(tr("Ready to start"));
}

//=====================================================================================================================================================================================================

void MainWindow::newUser(const QString &user, const QString &timeOn)
{
    usersList.append(user);
    usersName.append("");

    Hosts host;
    usersHosts.append(host);

    Apps app;
    usersApps.append(app);

    ui.treeWidgetUsers->addTopLevelItem(new QTreeWidgetItem(QStringList() << user << "" << timeOn));
    ui.treeWidgetUsers->topLevelItem(ui.treeWidgetUsers->topLevelItemCount()-1)->setIcon(0, QIcon(":/images/o_user.png"));

    ui.treeWidgetPackets->addTopLevelItem(new QTreeWidgetItem(QStringList() << user));
    ui.treeWidgetPackets->topLevelItem(ui.treeWidgetPackets->topLevelItemCount()-1)->setIcon(0, QIcon(":/images/o_user.png"));

    ui.treeWidgetTransfer->addTopLevelItem(new QTreeWidgetItem(QStringList() << user));
    ui.treeWidgetTransfer->topLevelItem(ui.treeWidgetTransfer->topLevelItemCount()-1)->setIcon(0, QIcon(":/images/o_user.png"));

    ui.treeWidgetUsersHosts->addTopLevelItem(new QTreeWidgetItem(QStringList() << user));
    ui.treeWidgetUsersHosts->topLevelItem(ui.treeWidgetUsersHosts->topLevelItemCount()-1)->setIcon(0, QIcon(":/images/o_user.png"));
    if (ui.treeWidgetUsersHosts->topLevelItemCount() == 1)
    {
        ui.treeWidgetUsersHosts->setCurrentItem(ui.treeWidgetUsersHosts->topLevelItem(0));
        ui.treeWidgetUsersHosts->setFocus();
    }

    ui.treeWidgetUsersApp->addTopLevelItem(new QTreeWidgetItem(QStringList() << user));
    ui.treeWidgetUsersApp->topLevelItem(ui.treeWidgetUsersApp->topLevelItemCount()-1)->setIcon(0, QIcon(":/images/o_user.png"));
    if (ui.treeWidgetUsersApp->topLevelItemCount() == 1)
    {
        ui.treeWidgetUsersApp->setCurrentItem(ui.treeWidgetUsersApp->topLevelItem(0));
        ui.treeWidgetUsersApp->setFocus();
    }
}

void MainWindow::newUserName(const QString &user,const QString &name)
{
    int userIndex = usersList.indexOf(user);

    usersName[userIndex] = name;

    ui.treeWidgetUsers->topLevelItem(userIndex)->setText(1, name);
    ui.treeWidgetPackets->topLevelItem(userIndex)->setText(1, name);
    ui.treeWidgetUsersHosts->topLevelItem(userIndex)->setText(1, name);
    ui.treeWidgetUsersApp->topLevelItem(userIndex)->setText(1, name);
    ui.treeWidgetTransfer->topLevelItem(userIndex)->setText(1, name);
}

void MainWindow::usersTransfer(QList<quint64> usersUp, QList<quint64> usersDown)
{
    this->usersUp = usersUp;
    this->usersDown = usersDown;

    for (int i = 0; i < ui.treeWidgetTransfer->topLevelItemCount(); ++i)
    {
        ui.treeWidgetTransfer->topLevelItem(i)->setText(2, bytesToStr(usersUp.at(i)));
        ui.treeWidgetTransfer->topLevelItem(i)->setText(3, bytesToStr(usersDown.at(i)));
    }
}

void MainWindow::usersSpeed(QList<qreal> usersUpSpeed, QList<qreal> usersDownSpeed)
{
    for (int i = 0; i < ui.treeWidgetTransfer->topLevelItemCount(); ++i)
    {
        ui.treeWidgetTransfer->topLevelItem(i)->setText(4, QString("%1 KB/s").arg(usersUpSpeed.at(i), 0, 'f', 2));
        ui.treeWidgetTransfer->topLevelItem(i)->setText(5, QString("%1 KB/s").arg(usersDownSpeed.at(i), 0, 'f', 2));
    }
}

void MainWindow::netTransfer(quint64 up, quint64 down)
{
    netUpTotal = up;
    netDownTotal = down;
}

void MainWindow::netAllPackets(QList<quint32> userArp, QList<quint32> userRarp, QList<quint32> userIcmp, QList<quint32> userIgmp, QList<quint32> userTcp, QList<quint32> userUdp, QList<quint32> userOther, QList<quint32> userTotal)
{
    for (int i = 0; i < userTotal.count(); ++i)
    {
        ui.treeWidgetPackets->topLevelItem(i)->setText(4, QString::number(userArp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(7, QString::number(userRarp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(10, QString::number(userIcmp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(13, QString::number(userIgmp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(16, QString::number(userTcp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(19, QString::number(userUdp.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(22, QString::number(userOther.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(25, QString::number(userTotal.at(i)));
    }
}

void MainWindow::netInPackets(QList<quint32> userArpIn, QList<quint32> userRarpIn, QList<quint32> userIcmpIn, QList<quint32> userIgmpIn, QList<quint32> userTcpIn, QList<quint32> userUdpIn, QList<quint32> userOtherIn, QList<quint32> userTotalIn)
{
    for (int i = 0; i < userTotalIn.count(); ++i)
    {
        ui.treeWidgetPackets->topLevelItem(i)->setText(2, QString::number(userArpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(5, QString::number(userRarpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(8, QString::number(userIcmpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(11, QString::number(userIgmpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(14, QString::number(userTcpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(17, QString::number(userUdpIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(20, QString::number(userOtherIn.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(23, QString::number(userTotalIn.at(i)));
    }
}

void MainWindow::netOutPackets(QList<quint32> userArpOut, QList<quint32> userRarpOut, QList<quint32> userIcmpOut, QList<quint32> userIgmpOut, QList<quint32> userTcpOut, QList<quint32> userUdpOut, QList<quint32> userOtherOut, QList<quint32> userTotalOut)
{
    for (int i = 0; i < userTotalOut.count(); ++i)
    {
        ui.treeWidgetPackets->topLevelItem(i)->setText(3, QString::number(userArpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(6, QString::number(userRarpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(9, QString::number(userIcmpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(12, QString::number(userIgmpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(15, QString::number(userTcpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(18, QString::number(userUdpOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(21, QString::number(userOtherOut.at(i)));
        ui.treeWidgetPackets->topLevelItem(i)->setText(24, QString::number(userTotalOut.at(i)));
    }
}

void MainWindow::newUserApp(quint16 user, Apps app)
{
    usersApps[user] = app;

    if (ui.treeWidgetUsersApp->indexOfTopLevelItem(ui.treeWidgetUsersApp->currentItem()) == user)
    {
        int i = app.hostPort.count() - 1;

        ui.treeWidgetApp->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetApp, QStringList() << app.hostPort.at(i) << app.hostPortName.at(i) << bytesToStr(app.upBytes.at(i)) << bytesToStr(app.downBytes.at(i))));
    }
}

void MainWindow::newUserHost(quint16 user, Hosts host)
{
    usersHosts[user] = host;

    if (ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem()) == user)
    {
        int i = host.hostIp.count() - 1;
        addr.S_un.S_addr = host.hostIp.at(i);

        ui.treeWidgetHosts->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetHosts, QStringList() << inet_ntoa(addr) << host.hostName.at(i) << host.dPort.at(i) << host.dApp.at(i) << bytesToStr(host.upBytes.at(i)) << bytesToStr(host.downBytes.at(i)) << host.firstVisit.at(i) << host.lastVisit.at(i)));
    }
}

void MainWindow::newHostName(const QString &hostAddress, const QString &hostName)
{
    for (int i = 0; i < usersHosts.count(); ++i)
        for (int j = 0; j < usersHosts.at(i).hostIp.count(); ++j)
            {
                addr.S_un.S_addr = usersHosts.at(i).hostIp.at(j);
                if (inet_ntoa(addr) == hostAddress)
                    usersHosts[i].hostName[j] = hostName;
            }

    int user = ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem());

    for (int i = 0; i < ui.treeWidgetHosts->topLevelItemCount(); ++i)
        ui.treeWidgetHosts->topLevelItem(i)->setText(1, usersHosts.at(user).hostName.at(i));
}

void MainWindow::updateUsersApps(QList<Apps> usersApps)
{
    this->usersApps = usersApps;

    int user = ui.treeWidgetUsersApp->indexOfTopLevelItem(ui.treeWidgetUsersApp->currentItem());

    if (user > -1)
    {
        for (int i = 0; i < ui.treeWidgetApp->topLevelItemCount(); ++i)
        {
            ui.treeWidgetApp->topLevelItem(i)->setText(2, bytesToStr(usersApps.at(user).upBytes.at(i)));
            ui.treeWidgetApp->topLevelItem(i)->setText(3, bytesToStr(usersApps.at(user).downBytes.at(i)));
        }
    }
}

void MainWindow::updateUsersHosts(QList<Hosts> usersHosts)
{
    this->usersHosts = usersHosts;

    int user = ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem());

    if (user > -1)
    {
        for (int i = 0; i < ui.treeWidgetHosts->topLevelItemCount(); ++i)
        {
            ui.treeWidgetHosts->topLevelItem(i)->setText(4, bytesToStr(usersHosts.at(user).upBytes.at(i)));
            ui.treeWidgetHosts->topLevelItem(i)->setText(5, bytesToStr(usersHosts.at(user).downBytes.at(i)));
            ui.treeWidgetHosts->topLevelItem(i)->setText(7, usersHosts.at(user).lastVisit.at(i));
        }
    }
}

//=====================================================================================================================================================================================================

QString MainWindow::bytesToStr(quint64 bytes)
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes >= 1024 && bytes < 1048576) return QString("%1 KB").arg((bytes / 1024.0), 0, 'f', 2);
    if (bytes >= 1048576) return QString("%1 MB").arg((bytes / 1024.0 / 1024.0), 0, 'f', 2);

    return QString("");
}

//=====================================================================================================================================================================================================

void MainWindow::writeSettings(bool writeFile)
{
    settings->mainWindow.maximized = isMaximized();
    settings->mainWindow.fullScreen = isFullScreen();

    if (isMaximized() || isFullScreen())
    {
        settings->mainWindow.size = oldSize;
        settings->mainWindow.position = oldPosition;
    }
    else
    {
        settings->mainWindow.size = size();
        settings->mainWindow.position = pos();
    }

    settings->mainWindow.toolbarsMovable = ui.actionMovable->isChecked();

    if (ui.actionIconOnly->isChecked()) settings->mainWindow.toolbarsStyle = 0;
    if (ui.actionTextOnly->isChecked()) settings->mainWindow.toolbarsStyle = 1;
    if (ui.actionTextBesideIcon->isChecked()) settings->mainWindow.toolbarsStyle = 2;
    if (ui.actionTextUnderIcon->isChecked()) settings->mainWindow.toolbarsStyle = 3;

    if (ui.action16->isChecked()) settings->mainWindow.toolbarsIconSize = 16;
    if (ui.action24->isChecked()) settings->mainWindow.toolbarsIconSize = 24;
    if (ui.action32->isChecked()) settings->mainWindow.toolbarsIconSize = 32;

    settings->mainWindow.toolbarsState = saveState(9);

    settings->mainWindow.statusBar = ui.actionStatusBar->isChecked();
    settings->mainWindow.trayIcon = ui.actionTrayIcon->isChecked();
    settings->mainWindow.alwaysOnTop = ui.actionAlwaysOnTop->isChecked();

    settings->mainWindow.currentTab = ui.tabWidget->currentIndex();
    settings->mainWindow.splitterApplication = ui.splitterApplications->saveState();
    settings->mainWindow.splitterHosts = ui.splitterHosts->saveState();

    eventsViewerMainWindow->writeSettings();
    packetsMainWindow->writeSettings();

    netTransferGraphDlg->writeSettings();
    netPacketsGraphDlg->writeSettings();
    userTransfersGraphDlg->writeSettings();

    if (writeFile)
    {
        settings->write();

        switch (settings->getError())
        {
            case 0: break;
            case 1: QMessageBox::warning(0, tr("Warning"), tr("An access error occurred (trying to write the file). Settings not saved."));
                    eventsViewerMainWindow->addEvent(EVENT_WARNING, tr("Writing settings to file"), tr("An access error occurred (trying to write the file). Settings not saved."));
                    break;
            default: break;
        }
    }
}

//=====================================================================================================================================================================================================

void MainWindow::infoMessage(quint8 type, const QString &title, const QString &message)
{
    if (type == 1)
    {
        QMessageBox::information(this, tr("Information"), title + ":\n" + message);
        eventsViewerMainWindow->addEvent(EVENT_INFORMATION, title, message);
        return;
    }

    if (type == 2)
    {
        QMessageBox::warning(this, tr("Warning"), title + ":\n" + message);
        eventsViewerMainWindow->addEvent(EVENT_WARNING, title, message);
        return;
    }

    if (type == 3)
    {
        QMessageBox::critical(this, tr("Critical"), title + ":\n" + message);
        eventsViewerMainWindow->addEvent(EVENT_CRITICAL, title, message);
        return;
    }
}

//=====================================================================================================================================================================================================

void MainWindow::onUsersAppsChanged()
{
    int user = ui.treeWidgetUsersApp->indexOfTopLevelItem(ui.treeWidgetUsersApp->currentItem());

    if (user >= 0)
    {
        ui.treeWidgetApp->clear();

        for (int i = 0; i < usersApps.at(user).hostPort.count(); ++i)
        {
            ui.treeWidgetApp->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetApp, QStringList() << usersApps.at(user).hostPort.at(i) << usersApps.at(user).hostPortName.at(i) << bytesToStr(usersApps.at(user).upBytes.at(i)) << bytesToStr(usersApps.at(user).downBytes.at(i))));
        }
    }
}

void MainWindow::onUsersHostsChanged()
{
    int user = ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem());

    if (user >= 0)
    {
        ui.treeWidgetHosts->clear();

        for (int i = 0; i < usersHosts.at(user).hostIp.count(); ++i)
        {
            addr.S_un.S_addr = usersHosts.at(user).hostIp.at(i);
            ui.treeWidgetHosts->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetHosts, QStringList() << inet_ntoa(addr) << usersHosts.at(user).hostName.at(i) << usersHosts.at(user).dPort.at(i) << usersHosts.at(user).dApp.at(i) << bytesToStr(usersHosts.at(user).upBytes.at(i)) << bytesToStr(usersHosts.at(user).downBytes.at(i)) << usersHosts.at(user).firstVisit.at(i) << usersHosts.at(user).lastVisit.at(i)));
        }
    }
}

void MainWindow::onApplicationsContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui.treeWidgetApp->itemAt(pos);
    if (!item)
        return;

    QAction *upAct = new QAction(QIcon(":/images/o_bars_up.png"), tr("Top active users on selected port [uploaded]"), this);
    QAction *downAct = new QAction(QIcon(":/images/o_bars_down.png"), tr("Top active users on selected port [downloaded]"), this);

    QMenu menu(tr("Context menu"), this);
    menu.addAction(upAct);
    menu.addAction(downAct);

    connect(upAct, SIGNAL(triggered()), this, SLOT(showTopAppUsersUpDlg()));
    connect(downAct, SIGNAL(triggered()), this, SLOT(showTopAppUsersDownDlg()));

    menu.exec(ui.treeWidgetApp->viewport()->mapToGlobal(pos));
}

void MainWindow::onHostsContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui.treeWidgetHosts->itemAt(pos);
    if (!item)
        return;

    QMenu menu(tr("Context menu"), this);
    menu.addAction(QIcon(":/images/o_globe.png"), tr("Open this host with web browser"));

    QAction *action = menu.exec(ui.treeWidgetHosts->viewport()->mapToGlobal(pos));

    if (action == 0)
        return;

    QDesktopServices::openUrl(QUrl("http://" + item->text(0), QUrl::TolerantMode));
}

//=====================================================================================================================================================================================================

void MainWindow::toggleIn(bool checked)
{
    for (int i = 2; i <= 23; i+=3)
        ui.treeWidgetPackets->setColumnHidden(i, !checked);
}

void MainWindow::toggleOut(bool checked)
{
    for (int i = 3; i <= 24; i+=3)
        ui.treeWidgetPackets->setColumnHidden(i, !checked);
}

void MainWindow::toggleTotal(bool checked)
{
    for (int i = 4; i <= 25; i+=3)
        ui.treeWidgetPackets->setColumnHidden(i, !checked);
}

//=====================================================================================================================================================================================================

void MainWindow::showTopActiveUpDlg()
{
    bool ok;
    int max;
    ui.treeWidgetTransfer->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetTransfer->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active users"), tr("Users:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    disconnect(receiverCore, SIGNAL(signalUsersTransfer(QList<quint64>,QList<quint64>)), this, SLOT(usersTransfer(QList<quint64>,QList<quint64>)));
    disconnect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)));

    quint64 maxValue;
    QList<qint32> topUsersList;

    for (int i = 0; i < maxTop; ++i)
        topUsersList.append(-1);

    // find top users
    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetTransfer->topLevelItemCount(); ++j)
        {
            if (usersUp.at(j) >= maxValue)
            {
                if (!topUsersList.contains(j))
                {
                    maxValue = usersUp.at(j);
                    topUsersList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_up.png"));
    dlg.setWindowTitle(tr("Top %1 active users [uploaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total uploaded on network"), bytesToStr(netUpTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(0) + " " + ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(1)), ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(2), usersUp.at(topUsersList.at(i)), netUpTotal);
    }

    dlg.exec();

    connect(receiverCore, SIGNAL(signalUsersTransfer(QList<quint64>,QList<quint64>)), this, SLOT(usersTransfer(QList<quint64>,QList<quint64>)));
    connect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)));
}

void MainWindow::showTopActiveDownDlg()
{
    bool ok;
    int max;
    ui.treeWidgetTransfer->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetTransfer->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active users"), tr("Users:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    disconnect(receiverCore, SIGNAL(signalUsersTransfer(QList<quint64>,QList<quint64>)), this, SLOT(usersTransfer(QList<quint64>,QList<quint64>)));
    disconnect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)));

    quint64 maxValue;
    QList<qint32> topUsersList;

    for (int i = 0; i < maxTop; ++i)
        topUsersList.append(-1);

    // find top users
    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetTransfer->topLevelItemCount(); ++j)
        {
            if (usersDown.at(j) >= maxValue)
            {
                if (!topUsersList.contains(j))
                {
                    maxValue = usersDown.at(j);
                    topUsersList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_down.png"));
    dlg.setWindowTitle(tr("Top %1 active users [downloaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total downloaded on network"), bytesToStr(netDownTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(0) + " " + ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(1)), ui.treeWidgetTransfer->topLevelItem(topUsersList.at(i))->text(3), usersDown.at(topUsersList.at(i)), netDownTotal);
    }

    dlg.exec();

    connect(receiverCore, SIGNAL(signalUsersTransfer(QList<quint64>,QList<quint64>)), this, SLOT(usersTransfer(QList<quint64>,QList<quint64>)));
    connect(receiverCore, SIGNAL(signalNetTransfer(quint64,quint64)), this, SLOT(netTransfer(quint64,quint64)));
}

void MainWindow::showUsersTransferGraphDlg()
{
    userTransfersGraphDlg->showNormal();
    userTransfersGraphDlg->raise();
    userTransfersGraphDlg->activateWindow();
}

//=====================================================================================================================================================================================================

void MainWindow::showTopAppUpDlg()
{
    bool ok;
    int max;
    ui.treeWidgetApp->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetApp->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active applications"), tr("Applications:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topAppList;

    for (int i = 0; i < maxTop; ++i)
        topAppList.append(-1);

    quint16 user = ui.treeWidgetUsersApp->indexOfTopLevelItem(ui.treeWidgetUsersApp->currentItem());

    quint64 appUpTotal = 0;

    // find top applications
    for (int i = 0; i < ui.treeWidgetApp->topLevelItemCount(); ++i)
    {
        appUpTotal+=usersApps.at(user).upBytes.at(i);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetApp->topLevelItemCount(); ++j)
        {
            if (usersApps.at(user).upBytes.at(j) >= maxValue)
            {
                if (!topAppList.contains(j))
                {
                    maxValue = usersApps.at(user).upBytes.at(j);
                    topAppList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_up.png"));
    dlg.setWindowTitle(tr("Top %1 active applications [uploaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total uploaded by user applications"), bytesToStr(appUpTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(1) + " [" + ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(0) + "]"), ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(2), usersApps.at(user).upBytes.at(topAppList.at(i)), appUpTotal);
    }

    dlg.exec();
}

void MainWindow::showTopAppDownDlg()
{
    bool ok;
    int max;
    ui.treeWidgetApp->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetApp->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active applications"), tr("Applications:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topAppList;

    for (int i = 0; i < maxTop; ++i)
        topAppList.append(-1);

    quint16 user = ui.treeWidgetUsersApp->indexOfTopLevelItem(ui.treeWidgetUsersApp->currentItem());

    quint64 appDownTotal = 0;

    // find top applications
    for (int i = 0; i < ui.treeWidgetApp->topLevelItemCount(); ++i)
    {
        appDownTotal+=usersApps.at(user).downBytes.at(i);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetApp->topLevelItemCount(); ++j)
        {
            if (usersApps.at(user).downBytes.at(j) >= maxValue)
            {
                if (!topAppList.contains(j))
                {
                    maxValue = usersApps.at(user).downBytes.at(j);
                    topAppList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_down.png"));
    dlg.setWindowTitle(tr("Top %1 active applications [downloaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total downloaded by user applications"), bytesToStr(appDownTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(1) + " [" + ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(0) + "]"), ui.treeWidgetApp->topLevelItem(topAppList.at(i))->text(3), usersApps.at(user).downBytes.at(topAppList.at(i)), appDownTotal);
    }

    dlg.exec();
}

void MainWindow::showTopAppUsersUpDlg()
{
    QTreeWidgetItem *item = ui.treeWidgetApp->currentItem();
    if (!item) return;

    bool ok;
    int max;
    ui.treeWidgetUsersApp->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetUsersApp->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active users"), tr("Users:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topUsersList;

    for (int i = 0; i < maxTop; ++i)
        topUsersList.append(-1);

    QString port = item->text(0);

    quint64 appUpTotal = 0;

    // find top users
    for (int i = 0; i < ui.treeWidgetUsersApp->topLevelItemCount(); ++i)
    {
        for (int j = 0; j < usersApps.at(i).hostPort.count(); ++j)
            if (usersApps.at(i).hostPort.at(j) == port)
                appUpTotal+=usersApps.at(i).upBytes.at(j);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetUsersApp->topLevelItemCount(); ++j)
        {
            for (int k = 0; k < usersApps.at(j).hostPort.count(); ++k)
                if (usersApps.at(j).hostPort.at(k) == port)
                    if (usersApps.at(j).upBytes.at(k) >= maxValue)
                    {
                        if (!topUsersList.contains(j))
                        {
                            maxValue = usersApps.at(j).upBytes.at(k);
                            topUsersList[i] = j;
                        }
                    }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_up.png"));
    dlg.setWindowTitle(tr("Top active users on port %1 [uploaded]").arg(port));
    dlg.setFirstItem(tr("Total uploaded by users on port %1").arg(port), bytesToStr(appUpTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        // if user don't use the application
        if (topUsersList.at(i) == -1)
            continue;

        for (int j = 0; j < usersApps.at(topUsersList.at(i)).hostPort.count(); ++j)
            if (usersApps.at(topUsersList.at(i)).hostPort.at(j) == port)
                dlg.insertItem(i, (ui.treeWidgetUsersApp->topLevelItem(topUsersList.at(i))->text(0) + " " + ui.treeWidgetUsersApp->topLevelItem(topUsersList.at(i))->text(1)), bytesToStr(usersApps.at(topUsersList.at(i)).upBytes.at(j)), usersApps.at(topUsersList.at(i)).upBytes.at(j), appUpTotal);
    }

    dlg.exec();
}

void MainWindow::showTopAppUsersDownDlg()
{
    QTreeWidgetItem *item = ui.treeWidgetApp->currentItem();
    if (!item) return;

    bool ok;
    int max;
    ui.treeWidgetUsersApp->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetUsersApp->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active users"), tr("Users:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topUsersList;

    for (int i = 0; i < maxTop; ++i)
        topUsersList.append(-1);

    QString port = item->text(0);

    quint64 appDownTotal = 0;

    // find top users
    for (int i = 0; i < ui.treeWidgetUsersApp->topLevelItemCount(); ++i)
    {
        for (int j = 0; j < usersApps.at(i).hostPort.count(); ++j)
            if (usersApps.at(i).hostPort.at(j) == port)
                appDownTotal+=usersApps.at(i).downBytes.at(j);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetUsersApp->topLevelItemCount(); ++j)
        {
            for (int k = 0; k < usersApps.at(j).hostPort.count(); ++k)
                if (usersApps.at(j).hostPort.at(k) == port)
                    if (usersApps.at(j).downBytes.at(k) >= maxValue)
                    {
                        if (!topUsersList.contains(j))
                        {
                            maxValue = usersApps.at(j).downBytes.at(k);
                            topUsersList[i] = j;
                        }
                    }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_down.png"));
    dlg.setWindowTitle(tr("Top active users on port %1 [downloaded]").arg(port));
    dlg.setFirstItem(tr("Total downloaded by users on port %1").arg(port), bytesToStr(appDownTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        // if user don't use the application
        if (topUsersList.at(i) == -1)
            continue;

        for (int j = 0; j < usersApps.at(topUsersList.at(i)).hostPort.count(); ++j)
            if (usersApps.at(topUsersList.at(i)).hostPort.at(j) == port)
                dlg.insertItem(i, (ui.treeWidgetUsersApp->topLevelItem(topUsersList.at(i))->text(0) + " " + ui.treeWidgetUsersApp->topLevelItem(topUsersList.at(i))->text(1)), bytesToStr(usersApps.at(topUsersList.at(i)).downBytes.at(j)), usersApps.at(topUsersList.at(i)).downBytes.at(j), appDownTotal);
    }

    dlg.exec();
}

//=====================================================================================================================================================================================================

void MainWindow::showTopActiveHostsUpDlg()
{
    bool ok;
    int max;
    ui.treeWidgetHosts->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetHosts->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active hosts"), tr("Hosts:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topHostsList;

    for (int i = 0; i < maxTop; ++i)
        topHostsList.append(-1);

    quint16 user = ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem());

    quint64 hostsUpTotal = 0;

    // find top hosts
    for (int i = 0; i < ui.treeWidgetHosts->topLevelItemCount(); ++i)
    {
        hostsUpTotal+=usersHosts.at(user).upBytes.at(i);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetHosts->topLevelItemCount(); ++j)
        {
            if (usersHosts.at(user).upBytes.at(j) >= maxValue)
            {
                if (!topHostsList.contains(j))
                {
                    maxValue = usersHosts.at(user).upBytes.at(j);
                    topHostsList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_up.png"));
    dlg.setWindowTitle(tr("Top %1 active hosts [uploaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total uploaded to hosts"), bytesToStr(hostsUpTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(1) + " [" + ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(0) + "]"), ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(4), usersHosts.at(user).upBytes.at(topHostsList.at(i)), hostsUpTotal);
    }

    dlg.exec();
}

void MainWindow::showTopActiveHostsDownDlg()
{
    bool ok;
    int max;
    ui.treeWidgetHosts->topLevelItemCount() > 25 ? max = 25 : max = ui.treeWidgetHosts->topLevelItemCount();
    if (max == 0) return;
    int maxTop = QInputDialog::getInteger(this, tr("Select number of top active hosts"), tr("Hosts:"), 10, 1, max, 1, &ok);
    if (!ok) return;

    quint64 maxValue;
    QList<qint32> topHostsList;

    for (int i = 0; i < maxTop; ++i)
        topHostsList.append(-1);

    quint16 user = ui.treeWidgetUsersHosts->indexOfTopLevelItem(ui.treeWidgetUsersHosts->currentItem());

    quint64 hostsDownTotal = 0;

    // find top hosts
    for (int i = 0; i < ui.treeWidgetHosts->topLevelItemCount(); ++i)
    {
        hostsDownTotal+=usersHosts.at(user).downBytes.at(i);
    }

    for (int i = 0; i < maxTop; ++i)
    {
        maxValue = 0;
        for (int j = 0; j < ui.treeWidgetHosts->topLevelItemCount(); ++j)
        {
            if (usersHosts.at(user).downBytes.at(j) >= maxValue)
            {
                if (!topHostsList.contains(j))
                {
                    maxValue = usersHosts.at(user).downBytes.at(j);
                    topHostsList[i] = j;
                }
            }
        }
    }

    TopActiveDialog dlg(this);
    dlg.setWindowIcon(QIcon(":/images/o_bars_down.png"));
    dlg.setWindowTitle(tr("Top %1 active hosts [downloaded]").arg(maxTop));
    dlg.setFirstItem(tr("Total downloaded from hosts"), bytesToStr(hostsDownTotal));

    for (int i = 0; i < maxTop; ++i)
    {
        dlg.insertItem(i, (ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(1) + " [" + ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(0) + "]"), ui.treeWidgetHosts->topLevelItem(topHostsList.at(i))->text(5), usersHosts.at(user).downBytes.at(topHostsList.at(i)), hostsDownTotal);
    }

    dlg.exec();
}

void MainWindow::onHostsOpen()
{
    QTreeWidgetItem *item = ui.treeWidgetHosts->currentItem();
    if (!item)
        return;

    QDesktopServices::openUrl(QUrl("http://" + item->text(0), QUrl::TolerantMode));
}

//=====================================================================================================================================================================================================

void MainWindow::onExportData()
{
    ExportDataDialog dlg(this);

    if (dlg.exec())
    {
        QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss_");
        QString folder = settings->exportDataDialog.folder;

        QString field;
        if (settings->exportDataDialog.fields == 0)
            field = ",";
        else
            field = ";";

        QString line;
        switch (settings->exportDataDialog.lines)
        {
            case 0: line = "\r\n"; break;
            case 1: line = "\r"; break;
            case 2: line = "\n"; break;
            default: line = "\r\n"; break;
        }

        bool insertHeader = settings->exportDataDialog.header;
        QString summaryString;

        if (settings->exportDataDialog.users)
        {
            QString fileName = folder+"/"+dateTime+"users.csv";

            if (fileName.isEmpty())
            {
                summaryString.append(tr("Users page: <b>FAIL</b> (empty file name)<br>"));
                goto endUsers;
            }

            QFile file(fileName);

            if (!file.open(QIODevice::WriteOnly))
            {
                eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Exporting data"), tr("Unable to open user file: %1").arg(fileName));
                summaryString.append(tr("Users page: <b>FAIL</b> (unable to open file)<br>"));
                goto endUsers;
            }

            QTextStream out(&file);
            out.setCodec("UTF-8");

            int columns = ui.treeWidgetUsers->columnCount();

            int i = 0;

            if (insertHeader)
                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetUsers->headerItem()->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }

            for (int j = 0; j < ui.treeWidgetUsers->topLevelItemCount(); ++j)
            {
                i = 0;

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetUsers->topLevelItem(j)->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }
            }

            file.close();

            summaryString.append(tr("Users page: <b>DONE</b><br>"));
        }

        endUsers:

        if (settings->exportDataDialog.packets)
        {
            QString fileName = folder+"/"+dateTime+"packets.csv";

            if (fileName.isEmpty())
            {
                summaryString.append(tr("Packets page: <b>FAIL</b> (empty file name)<br>"));
                goto endPackets;
            }

            QFile file(fileName);

            if (!file.open(QIODevice::WriteOnly))
            {
                eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Exporting data"), tr("Unable to open user file: %1").arg(fileName));
                summaryString.append(tr("Packets page: <b>FAIL</b> (unable to open file)<br>"));
                goto endPackets;
            }

            QTextStream out(&file);
            out.setCodec("UTF-8");

            int columns = ui.treeWidgetPackets->columnCount();

            int i = 0;

            if (insertHeader)
                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetPackets->headerItem()->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }

            for (int j = 0; j < ui.treeWidgetPackets->topLevelItemCount(); ++j)
            {
                i = 0;

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetPackets->topLevelItem(j)->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }
            }

            file.close();

            summaryString.append(tr("Packets page: <b>DONE</b><br>"));
        }

        endPackets:

        if (settings->exportDataDialog.transfers)
        {
            QString fileName = folder+"/"+dateTime+"transfers.csv";

            if (fileName.isEmpty())
            {
                summaryString.append(tr("Transfers page: <b>FAIL</b> (empty file name)<br>"));
                goto endTransfer;
            }

            QFile file(fileName);

            if (!file.open(QIODevice::WriteOnly))
            {
                eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Exporting data"), tr("Unable to open user file: %1").arg(fileName));
                summaryString.append(tr("Transfers page: <b>FAIL</b> (unable to open file)<br>"));
                goto endTransfer;
            }

            QTextStream out(&file);
            out.setCodec("UTF-8");

            int columns = ui.treeWidgetTransfer->columnCount()-2;

            int i = 0;

            if (insertHeader)
                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetTransfer->headerItem()->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }

            for (int j = 0; j < ui.treeWidgetTransfer->topLevelItemCount(); ++j)
            {
                i = 0;

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetTransfer->topLevelItem(j)->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }
            }

            file.close();

            summaryString.append(tr("Transfers page: <b>DONE</b><br>"));
        }

        endTransfer:

        if (settings->exportDataDialog.applications)
        {
            QString fileName = folder+"/"+dateTime+"applications.csv";

            if (fileName.isEmpty())
            {
                summaryString.append(tr("Applications page: <b>FAIL</b> (empty file name)<br>"));
                goto endApplications;
            }

            QFile file(fileName);

            if (!file.open(QIODevice::WriteOnly))
            {
                eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Exporting data"), tr("Unable to open user file: %1").arg(fileName));
                summaryString.append(tr("Applications page: <b>FAIL</b> (unable to open file)<br>"));
                goto endApplications;
            }

            QTextStream out(&file);
            out.setCodec("UTF-8");

            int i;
            int columns;

            if (insertHeader)
            {
                i = 0;
                columns = ui.treeWidgetUsersApp->columnCount();

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetUsersApp->headerItem()->text(i) << "\"" << field;

                    ++i;
                }

                i = 0;
                columns = ui.treeWidgetApp->columnCount();

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetApp->headerItem()->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }
            }

            for (int j = 0; j < ui.treeWidgetUsersApp->topLevelItemCount(); ++j)
            {
                i = 0;

                int x = usersApps.at(j).hostPort.count();

                if (x == 0)
                {
                    out << "\"" << ui.treeWidgetUsersApp->topLevelItem(j)->text(0) << "\"" << field << "\"" << ui.treeWidgetUsersApp->topLevelItem(j)->text(1) << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << line;
                }

                while (i < x)
                {
                    if (i == 0)
                        out << "\"" << ui.treeWidgetUsersApp->topLevelItem(j)->text(0) << "\"" << field << "\"" << ui.treeWidgetUsersApp->topLevelItem(j)->text(1) << "\"" << field;
                    else
                        out << "\"" << "" << "\"" << field << "\"" << "" << "\"" << field;

                    out << "\"" << usersApps.at(j).hostPort.at(i) << "\"" << field;
                    out << "\"" << usersApps.at(j).hostPortName.at(i) << "\"" << field;
                    out << "\"" << bytesToStr(usersApps.at(j).upBytes.at(i)) << "\"" << field;
                    out << "\"" << bytesToStr(usersApps.at(j).downBytes.at(i)) << "\"" << line;

                    ++i;
                }
            }

            file.close();

            summaryString.append(tr("Applications page: <b>DONE</b><br>"));
        }

        endApplications:

        if (settings->exportDataDialog.hosts)
        {
            QString fileName = folder+"/"+dateTime+"hosts.csv";

            if (fileName.isEmpty())
            {
                summaryString.append(tr("Hosts page: <b>FAIL</b> (empty file name)<br>"));
                goto endHosts;
            }

            QFile file(fileName);

            if (!file.open(QIODevice::WriteOnly))
            {
                eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Exporting data"), tr("Unable to open user file: %1").arg(fileName));
                summaryString.append(tr("Hosts page: <b>FAIL</b> (unable to open file)<br>"));
                goto endHosts;
            }

            QTextStream out(&file);
            out.setCodec("UTF-8");

            int i;
            int columns;

            if (insertHeader)
            {
                i = 0;
                columns = ui.treeWidgetUsersHosts->columnCount();

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetUsersHosts->headerItem()->text(i) << "\"" << field;

                    ++i;
                }

                i = 0;
                columns = ui.treeWidgetHosts->columnCount();

                while (i < columns)
                {
                    out << "\"" << ui.treeWidgetHosts->headerItem()->text(i) << "\"";

                    if (++i < columns)
                        out << field;
                    else
                        out << line;
                }
            }

            for (int j = 0; j < ui.treeWidgetUsersHosts->topLevelItemCount(); ++j)
            {
                i = 0;

                int x = usersHosts.at(j).dPort.count();

                if (x == 0)
                {
                    out << "\"" << ui.treeWidgetUsersHosts->topLevelItem(j)->text(0) << "\"" << field << "\"" << ui.treeWidgetUsersHosts->topLevelItem(j)->text(1) << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << field;
                    out << "\"" << "" << "\"" << line;
                }

                while (i < x)
                {
                    if (i == 0)
                        out << "\"" << ui.treeWidgetUsersHosts->topLevelItem(j)->text(0) << "\"" << field << "\"" << ui.treeWidgetUsersHosts->topLevelItem(j)->text(1) << "\"" << field;
                    else
                        out << "\"" << "" << "\"" << field << "\"" << "" << "\"" << field;

                    addr.S_un.S_addr = usersHosts.at(j).hostIp.at(i);
                    QString user = inet_ntoa(addr);
                    out << "\"" << user << "\"" << field;
                    out << "\"" << usersHosts.at(j).hostName.at(i) << "\"" << field;
                    out << "\"" << usersHosts.at(j).dPort.at(i) << "\"" << field;
                    out << "\"" << usersHosts.at(j).dApp.at(i) << "\"" << field;
                    out << "\"" << bytesToStr(usersHosts.at(j).upBytes.at(i)) << "\"" << field;
                    out << "\"" << bytesToStr(usersHosts.at(j).downBytes.at(i)) << "\"" << field;
                    out << "\"" << usersHosts.at(j).firstVisit.at(i) << "\"" << field;
                    out << "\"" << usersHosts.at(j).lastVisit.at(i) << "\"" << line;

                    ++i;
                }
            }

            file.close();

            summaryString.append(tr("Hosts page: <b>DONE</b><br>"));
        }

        endHosts:

        if (settings->exportDataDialog.summary)
        {
            SummaryDialog sDlg(this, summaryString);

            if (sDlg.exec())
            {
                if (!sDlg.showAgain())
                    settings->exportDataDialog.summary = false;
            }
        }

        if (settings->exportDataDialog.openAfter)
            QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    }
}

//=====================================================================================================================================================================================================
void MainWindow::showNetPacketsDlg()
{
    netPacketsDlg->show();
    netPacketsDlg->raise();
    netPacketsDlg->activateWindow();
}

void MainWindow::showNetPacketsGraphDlg()
{
    netPacketsGraphDlg->showNormal();
    netPacketsGraphDlg->raise();
    netPacketsGraphDlg->activateWindow();
}

void MainWindow::showNetTransferDlg()
{
    netTransferDlg->show();
    netTransferDlg->raise();
    netTransferDlg->activateWindow();
}

void MainWindow::showNetTransferGraphDlg()
{
    netTransferGraphDlg->showNormal();
    netTransferGraphDlg->raise();
    netTransferGraphDlg->activateWindow();
}

void MainWindow::showPacketsMainWindow()
{
    if (packetsMainWindow->isMinimized())
        packetsMainWindow->setWindowState(packetsMainWindow->windowState() ^ Qt::WindowMinimized);

    packetsMainWindow->show();
    packetsMainWindow->raise();
    packetsMainWindow->activateWindow();
}

//=====================================================================================================================================================================================================

void MainWindow::showEventsViewerMainWindow()
{
    if (eventsViewerMainWindow->isMinimized())
        eventsViewerMainWindow->setWindowState(eventsViewerMainWindow->windowState() ^ Qt::WindowMinimized);

    eventsViewerMainWindow->show();
    eventsViewerMainWindow->raise();
    eventsViewerMainWindow->activateWindow();
}

void MainWindow::showPortNumbersDlg()
{
    PortNumbersDialog dlg(this);

    dlg.exec();

    if (capturing && dlg.isModified())
        QMessageBox::information(this, tr("Information"), tr("The changes take effect next time you start capture."));
}

//=====================================================================================================================================================================================================

void MainWindow::toggleFullScreen(bool checked)
{
    if (checked)
    {
        ui.actionAlwaysOnTop->setDisabled(true);
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
    else
    {
        ui.actionAlwaysOnTop->setEnabled(true);
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
}

void MainWindow::toggleAlwaysOnTop(bool checked)
{
    setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    show();
}

void MainWindow::showSettingsDlg()
{
    writeSettings(false);

    int up = settings->netTransferDialog.up;
    int down = settings->netTransferDialog.down;

    SettingsDialog dlg(this);

    dlg.exec();

    if (up != settings->netTransferDialog.up || down != settings->netTransferDialog.down)
    {
        netTransferDlg->setScale(settings->netTransferDialog.up, settings->netTransferDialog.down);
    }
}

void MainWindow::showWindowOpacityDlg()
{
    TransparencyDialog dlg(this);

    connect(&dlg, SIGNAL(valueChanged(int)), this, SLOT(setMyWindowOpacity(int)));

    dlg.exec();

    disconnect(&dlg, SIGNAL(valueChanged(int)), this, SLOT(setMyWindowOpacity(int)));
}

void MainWindow::setMyWindowOpacity(int value)
{
    setWindowOpacity(value * 0.01);
}

//=====================================================================================================================================================================================================

void MainWindow::showHelp()
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/help.pdf")))
    {
        QMessageBox::critical(this, tr("Critical"), tr("The help file could not be open. Probably help file does not exist."));
        eventsViewerMainWindow->addEvent(EVENT_CRITICAL, tr("Opening help file"), tr("The help file could not be open. Probably help file does not exist."));
    }
}

void MainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}
