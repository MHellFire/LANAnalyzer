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

#include "packetsmainwindow.h"

PacketsMainWindow::PacketsMainWindow(QWidget *parent, CaptureThread *thread)
    : QMainWindow(parent), thread(thread)
{
    ui.setupUi(this);

    ui.treeWidget->resizeColumnToContents(0);
    ui.treeWidget->resizeColumnToContents(2);

    createMenu();
    createToolbars();
    createStatusBar();

    restoreWindowState();

    if (Settings::mainWindow.firstRun)
    {
        Settings::packetsMainWindow.toolbarsState = saveState(9);
        Settings::packetsMainWindow.toolbarsDefault = saveState(9);
    }

    clearTree();
}

PacketsMainWindow::~PacketsMainWindow()
{
}

void PacketsMainWindow::resizeEvent(QResizeEvent *event)
{
    oldSize = event->oldSize();
}

void PacketsMainWindow::moveEvent(QMoveEvent *event)
{
    oldPosition = event->oldPos();
}

void PacketsMainWindow::createMenu()
{
    // menu
    // file
    connect(ui.actionExportData, SIGNAL(triggered()), this, SLOT(onExportData()));
    connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));

    // view
    connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(ui.actionClear, SIGNAL(triggered()), this, SLOT(onClear()));
    ui.actionAutoScrollToBottom->setChecked(Settings::packetsMainWindow.autoScroll);
    connect(ui.actionAutoScrollToBottom, SIGNAL(triggered(bool)), this, SLOT(onAutoScrollTriggered(bool)));

    // options
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarFile->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarView->toggleViewAction());
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
    connect(ui.actionAlwaysOnTop, SIGNAL(triggered(bool)), this, SLOT(toggleAlwaysOnTop(bool)));

    // help
    connect(ui.actionContents, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void PacketsMainWindow::createToolbars()
{
    // file
    exportDataAct = new QAction(tr("Export"), this);
    ui.toolBarFile->addAction(exportDataAct);
    exportDataAct->setEnabled(true);
    connect(exportDataAct, SIGNAL(triggered()), this, SLOT(onExportData()));

    // view
    startAct = new QAction(tr("Start"), this);
    ui.toolBarView->addAction(startAct);
    startAct->setEnabled(true);
    connect(startAct, SIGNAL(triggered()), this, SLOT(onStart()));

    stopAct = new QAction(tr("Stop"), this);
    ui.toolBarView->addAction(stopAct);
    stopAct->setDisabled(true);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(onStop()));

    ui.toolBarView->addSeparator();

    clearAct = new QAction(tr("Clear"), this);
    ui.toolBarView->addAction(clearAct);
    clearAct->setEnabled(true);
    connect(clearAct, SIGNAL(triggered()), this, SLOT(onClear()));

    ui.toolBarView->addSeparator();

    autoScrollAct = new QAction(tr("Auto scroll"), this);
    ui.toolBarView->addAction(autoScrollAct);
    autoScrollAct->setCheckable(true);
    autoScrollAct->setChecked(Settings::packetsMainWindow.autoScroll);
    onAutoScrollTriggered(Settings::packetsMainWindow.autoScroll);
    connect(autoScrollAct, SIGNAL(triggered(bool)), this, SLOT(onAutoScrollTriggered(bool)));

    // help
    helpAct = new QAction(tr("Help"), this);
    helpAct->setEnabled(true);
    connect(helpAct, SIGNAL(triggered()), this, SLOT(showHelp()));

    aboutAct = new QAction(tr("About"), this);
    aboutAct->setEnabled(true);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(showAbout()));

    helpActMenu = new QMenu(this);
    helpActMenu->addAction(aboutAct);
    helpAct->setMenu(helpActMenu);
    ui.toolBarHelp->addAction(helpAct);

    restoreState(Settings::packetsMainWindow.toolbarsState, 9);

    ui.actionMovable->setChecked(Settings::packetsMainWindow.toolbarsMovable);

    switch (Settings::packetsMainWindow.toolbarsStyle)
    {
        case 0: ui.actionIconOnly->trigger(); break;
        case 1: ui.actionTextOnly->trigger(); break;
        case 2: ui.actionTextBesideIcon->trigger(); break;
        case 3: ui.actionTextUnderIcon->trigger(); break;
        default: break;
    }

    switch (Settings::packetsMainWindow.toolbarsIconSize)
    {
        case 16: ui.action16->trigger(); break;
        case 24: ui.action24->trigger(); break;
        case 32: ui.action32->trigger(); break;
        default: break;
    }
}

void PacketsMainWindow::onChangeMovable(bool movable)
{
    ui.toolBarFile->setMovable(movable);
    ui.toolBarView->setMovable(movable);
    ui.toolBarHelp->setMovable(movable);
}

void PacketsMainWindow::onToolbarsStyleChanged(QAction *action)
{
    if (ui.actionIconOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarView->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    if (ui.actionTextOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarView->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    if (ui.actionTextBesideIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarView->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    if (ui.actionTextUnderIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarView->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
}

void PacketsMainWindow::onToolbarsSizeChanged(QAction *action)
{
    int size = 24;

    if (action->text().contains("16")) size = 16;
    if (action->text().contains("24")) size = 24;
    if (action->text().contains("32")) size = 32;

    ui.toolBarFile->setIconSize(QSize(size, size));
    ui.toolBarView->setIconSize(QSize(size, size));
    ui.toolBarHelp->setIconSize(QSize(size, size));

    setToolbarIcons(size);
}

void PacketsMainWindow::setToolbarIcons(int size)
{
    // file
    exportDataAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_export.png")));

    // view
    startAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_start_now.png")));
    stopAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_stop.png")));
    clearAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_clear.png")));
    autoScrollAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_bottom.png")));

    // help
    helpAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_help.png")));
    aboutAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_about.png")));
}

void PacketsMainWindow::onDefaultToolbars()
{
    restoreState(Settings::packetsMainWindow.toolbarsDefault, 9);
    ui.actionMovable->setChecked(true);
    ui.actionTextUnderIcon->trigger();
    ui.action24->trigger();
}

void PacketsMainWindow::createStatusBar()
{
    ui.statusbar->addPermanentWidget(infoLabel = new QLabel(tr("Packets: 0")), 1);
}

void PacketsMainWindow::restoreWindowState()
{
    resize(Settings::packetsMainWindow.size);
    move(Settings::packetsMainWindow.position);

    if (Settings::packetsMainWindow.alwaysOnTop)
    {
        Qt::WindowFlags flags = windowFlags();
        flags.testFlag(Qt::WindowStaysOnTopHint) ? flags ^= Qt::WindowStaysOnTopHint : flags |= Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        ui.actionAlwaysOnTop->setChecked(true);
    }

    if (Settings::packetsMainWindow.maximized)
    {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }

    ui.actionStatusBar->setChecked(Settings::packetsMainWindow.statusBar);
}

void PacketsMainWindow::writeSettings()
{
    Settings::packetsMainWindow.maximized = isMaximized();
    if (isMaximized())
    {
        Settings::packetsMainWindow.size = oldSize;
        Settings::packetsMainWindow.position = oldPosition;
    }
    else
    {
        Settings::packetsMainWindow.size = size();
        Settings::packetsMainWindow.position = pos();
    }

    Settings::packetsMainWindow.toolbarsMovable = ui.actionMovable->isChecked();
    if (ui.actionIconOnly->isChecked()) Settings::packetsMainWindow.toolbarsStyle = 0;
    if (ui.actionTextOnly->isChecked()) Settings::packetsMainWindow.toolbarsStyle = 1;
    if (ui.actionTextBesideIcon->isChecked()) Settings::packetsMainWindow.toolbarsStyle = 2;
    if (ui.actionTextUnderIcon->isChecked()) Settings::packetsMainWindow.toolbarsStyle = 3;

    if (ui.action16->isChecked()) Settings::packetsMainWindow.toolbarsIconSize = 16;
    if (ui.action24->isChecked()) Settings::packetsMainWindow.toolbarsIconSize = 24;
    if (ui.action32->isChecked()) Settings::packetsMainWindow.toolbarsIconSize = 32;

    Settings::packetsMainWindow.toolbarsState = saveState(9);
    Settings::packetsMainWindow.statusBar = ui.actionStatusBar->isChecked();
    Settings::packetsMainWindow.alwaysOnTop = ui.actionAlwaysOnTop->isChecked();

    Settings::packetsMainWindow.autoScroll = ui.actionAutoScrollToBottom->isChecked();
}

void PacketsMainWindow::onExportData()
{
}

void PacketsMainWindow::onStart()
{
    connect(thread, SIGNAL(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)), this, SLOT(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)), Qt::QueuedConnection);
    startAct->setDisabled(true);
    ui.actionStart->setDisabled(true);
    stopAct->setEnabled(true);
    ui.actionStop->setEnabled(true);
}

void PacketsMainWindow::onStop()
{
    disconnect(thread, SIGNAL(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)), this, SLOT(receivedPacket(QString,quint32,QString,QString,quint16,quint32,quint32,quint32,quint32,QString)));
    startAct->setEnabled(true);
    ui.actionStart->setEnabled(true);
    stopAct->setDisabled(true);
    ui.actionStop->setDisabled(true);
}

void PacketsMainWindow::onClear()
{
    clearTree();
}

void PacketsMainWindow::onAutoScrollTriggered(bool checked)
{
    autoScroll = checked;

    ui.actionAutoScrollToBottom->setChecked(checked);
    autoScrollAct->setChecked(checked);

    if (checked)
        ui.treeWidget->scrollToBottom();
}

void PacketsMainWindow::toggleAlwaysOnTop(bool checked)
{
    setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    show();
}

void PacketsMainWindow::receivedPacket(const QString &time, quint32 length, const QString &sMac, const QString &dMac, quint16 type, quint32 sIP, quint32 dIP, quint32 sPort, quint32 dPort, const QString &info)
{
    if (type == 0x0806) { typeStr = "ARP"; goto end; }
    if (type == 0x8035) { typeStr = "RARP"; goto end; }
    if (type == 6) { typeStr = "IPv4 TCP"; goto end; }
    if (type == 17) { typeStr = "IPv4 UDP"; goto end; }
    if (type == 1) { typeStr = "IPv4 ICMP"; goto end; }
    if (type == 2) { typeStr = "IPv4 IGMP"; goto end; }

    if (type == 0x0842) { typeStr = "WOL"; goto end; }
    if (type == 0x86DD) { typeStr = "IPv6"; goto end; }
    if (type == 0x8137) { typeStr = "IPX"; goto end; }
    if (type == 0x8863) { typeStr = "PPoE"; goto end; }
    if (type == 0x8864) { typeStr = "PPoE"; goto end; }

    typeStr = "protocol not supported";

    end:
    sAddrIP.S_un.S_addr = sIP;
    dAddrIP.S_un.S_addr = dIP;
    ui.treeWidget->addTopLevelItem( new QTreeWidgetItem(QStringList()
                                                        << QString::number(++packetsNo)
                                                        << time
                                                        << QString::number(length)
                                                        << sMac
                                                        << dMac
                                                        << typeStr
                                                        << inet_ntoa(sAddrIP)
                                                        << (sPort < 65536 ? QString::number(sPort) : "")
                                                        << inet_ntoa(dAddrIP)
                                                        << (dPort < 65536 ? QString::number(dPort) : "")
                                                        << info) );
    infoLabel->setText(tr("Packets: %1").arg(packetsNo));

    if (autoScroll)
        ui.treeWidget->scrollToBottom();
}

void PacketsMainWindow::clearTree()
{
    packetsNo = 0;
    ui.treeWidget->clear();
    infoLabel->setText(tr("Packets: 0"));
}

void PacketsMainWindow::showHelp()
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/help.pdf")))
    {
        QMessageBox::critical(this, tr("Critical"), tr("The help file could not be open. Probably help file does not exist."));
    }
}

void PacketsMainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}
