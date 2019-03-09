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

#include "settings.h"

MainWindowSettings Settings::mainWindow;
CaptureThreadSettings Settings::captureThread;
DevicesDialogSettings Settings::devicesDialog;
ExportDataDialogSettings Settings::exportDataDialog;
EventsViewerMainWindowSettings Settings::eventsViewerMainWindow;
PacketsMainWindowSettings Settings::packetsMainWindow;
ScreenshotsSettings Settings::screenshots;
NetPacketsGraphDialogSettings Settings::netPacketsGraphDialog;
NetTransferGraphDialogSettings Settings::netTransferGraphDialog;
UserTransfersGraphDialogSettings Settings::userTransfersGraphDialog;
NetTransferDialogSettings Settings::netTransferDialog;

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    settingsFileName = QCoreApplication::applicationDirPath() + "/settings.ini";
}

Settings::~Settings()
{
}

void Settings::create()
{
    QSettings s(settingsFileName, QSettings::IniFormat, this);

    s.beginGroup("MainWindow");
    s.setValue("firstRun", true);
    s.setValue("languageFile", "English (Default)");
    s.setValue("startMaximized", false);
    s.setValue("maximized", false);
    s.setValue("fullScreen", false);
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/8)*6, (int)(QApplication::desktop()->height()/8)*6));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/8), (int)(QApplication::desktop()->height()/8)));
    s.setValue("toolbarsState", QByteArray());
    s.setValue("toolbarsDefault", QByteArray());
    s.setValue("toolbarsMovable", true);
    s.setValue("toolbarsStyle", 3);
    s.setValue("toolbarsIconSize", 24);
    s.setValue("statusBar", true);
    s.setValue("trayIcon", false);
    s.setValue("alwaysOnTop", false);
    s.setValue("currentTab", 0);
    s.setValue("splitterApplication", QByteArray());
    s.setValue("splitterHosts", QByteArray());
    s.setValue("deviceName", "");
    s.setValue("filterName", "All packets");
    s.setValue("filterCode", "");
    s.setValue("autoStart", false);
    s.setValue("minimizeToTrayOnStart", false);
    s.setValue("minimizeToTrayOnClose", false);
    s.setValue("autoCheckUpdate", false);
    s.endGroup();

    s.beginGroup("CaptureThread");
    s.setValue("mode", 1);
    s.setValue("bytes", 65535);
    s.setValue("timeout", 1000);
    s.endGroup();

    s.beginGroup("DevicesDialog");
    s.setValue("showDevicesInfo", false);
    s.setValue("showIPv4", true);
    s.setValue("showIPv6", true);
    s.endGroup();

    s.beginGroup("ExportDataDialog");
    s.setValue("users", true);
    s.setValue("packets", true);
    s.setValue("transfers", true);
    s.setValue("applications", true);
    s.setValue("hosts", true);
    s.setValue("fields", 0);
    s.setValue("lines", 0);
    s.setValue("header", false);
    s.setValue("folder", QDir::toNativeSeparators(QDir::homePath()));
    s.setValue("openAfter", false);
    s.setValue("summary", true);
    s.endGroup();

    s.beginGroup("EventsViewerMainWindow");
    s.setValue("maximized", false);
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/6)*4, (int)(QApplication::desktop()->height()/4)*2));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/6), (int)(QApplication::desktop()->height()/4)));
    s.setValue("toolbarsState", QByteArray());
    s.setValue("toolbarsDefault", QByteArray());
    s.setValue("toolbarsMovable", true);
    s.setValue("toolbarsStyle", 3);
    s.setValue("toolbarsIconSize", 24);
    s.setValue("statusBar", true);
    s.setValue("alwaysOnTop", false);
    s.setValue("information", true);
    s.setValue("warning", true);
    s.setValue("critical", true);
    s.setValue("createFile", true);
    s.setValue("folder", QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
    s.endGroup();

    s.beginGroup("PacketsMainWindow");
    s.setValue("maximized", false);
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/6)*4, (int)(QApplication::desktop()->height()/4)*2));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/6), (int)(QApplication::desktop()->height()/4)));
    s.setValue("toolbarsState", QByteArray());
    s.setValue("toolbarsDefault", QByteArray());
    s.setValue("toolbarsMovable", true);
    s.setValue("toolbarsStyle", 3);
    s.setValue("toolbarsIconSize", 24);
    s.setValue("statusBar", true);
    s.setValue("alwaysOnTop", false);
    s.setValue("autoScroll", false);
    s.endGroup();

    s.beginGroup("Screenshots");
    s.setValue("dialog", true);
    s.setValue("format", "Portable Network Graphics (*.png)");
    s.setValue("extension", "png");
    s.setValue("quality", -1);
    s.setValue("folder", QDir::toNativeSeparators(QDir::homePath()));
    s.setValue("name", "%D_%T_%N");
    s.endGroup();

    s.beginGroup("NetPacketsGraphDialog");
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6)));
    s.setValue("time", 0);
    s.setValue("h", true);
    s.setValue("v", false);
    s.setValue("background", true);
    s.setValue("filled", false);
    s.setValue("antialiasing", false);
    s.setValue("tooltip", true);
    s.endGroup();

    s.beginGroup("NetTransferGraphDialog");
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6)));
    s.setValue("time", 0);
    s.setValue("show", 0);
    s.setValue("h", true);
    s.setValue("v", false);
    s.setValue("background", true);
    s.setValue("filled", false);
    s.setValue("antialiasing", false);
    s.setValue("tooltip", true);
    s.endGroup();

    s.beginGroup("UserTransfersGraphDialog");
    s.setValue("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4));
    s.setValue("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6)));
    s.setValue("time", 0);
    s.setValue("show", 0);
    s.setValue("h", true);
    s.setValue("v", false);
    s.setValue("background", true);
    s.setValue("filled", false);
    s.setValue("antialiasing", false);
    s.setValue("tooltip", true);
    s.endGroup();

    s.beginGroup("NetTransferDialog");
    s.setValue("up", 0);
    s.setValue("down", 0);
    s.endGroup();
}

void Settings::read()
{
    error = 0;

    QFile file(settingsFileName);
    if (!file.exists())
    {
        //QMessageBox::warning(0, tr("Reading settings from file"), tr("Settings file does not exist. Default settings loaded."));

        error = 1;

        create();
    }
    file.close();

    QSettings s(settingsFileName, QSettings::IniFormat, this);

    s.beginGroup("MainWindow");
    mainWindow.firstRun = s.value("firstRun", true).toBool();
    mainWindow.languageFile = s.value("languageFile", "English (Default)").toString();
    mainWindow.startMaximized = s.value("startMaximized", false).toBool();
    mainWindow.maximized = s.value("maximized", false).toBool();
    mainWindow.fullScreen = s.value("fullScreen", false).toBool();
    mainWindow.size = s.value("size", QSize((int)(QApplication::desktop()->width()/8)*6, (int)(QApplication::desktop()->height()/8)*6)).toSize();
    mainWindow.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/8), (int)(QApplication::desktop()->height()/8))).toPoint();
    mainWindow.toolbarsState = s.value("toolbarsState", QByteArray()).toByteArray();
    mainWindow.toolbarsDefault = s.value("toolbarsDefault", QByteArray()).toByteArray();
    mainWindow.toolbarsMovable = s.value("toolbarsMovable", true).toBool();
    mainWindow.toolbarsStyle = s.value("toolbarsStyle", 3).toInt();
    mainWindow.toolbarsIconSize = s.value("toolbarsIconSize", 24).toInt();
    mainWindow.statusBar = s.value("statusBar", true).toBool();
    mainWindow.trayIcon = s.value("trayIcon", false).toBool();
    mainWindow.alwaysOnTop = s.value("alwaysOnTop", false).toBool();
    mainWindow.currentTab = s.value("currentTab", 0).toInt();
    mainWindow.splitterApplication = s.value("splitterApplication", QByteArray()).toByteArray();
    mainWindow.splitterHosts = s.value("splitterHosts", QByteArray()).toByteArray();
    mainWindow.deviceName = s.value("deviceName", "").toString();
    mainWindow.filterName = s.value("filterName", "All packets").toString();
    mainWindow.filterCode = s.value("filterCode", "").toString();
    mainWindow.autoStart = s.value("autoStart", false).toBool();
    mainWindow.minimizeToTrayOnStart = s.value("minimizeToTrayOnStart", false).toBool();
    mainWindow.minimizeToTrayOnClose = s.value("minimizeToTrayOnClose", false).toBool();
    mainWindow.autoCheckUpdate = s.value("autoCheckUpdate", false).toBool();
    s.endGroup();

    s.beginGroup("CaptureThread");
    captureThread.mode = s.value("mode", 1).toInt();
    captureThread.bytes = s.value("bytes", 65535).toInt();
    captureThread.timeout = s.value("timeout", 1000).toInt();  // milliseconds
    s.endGroup();

    s.beginGroup("DevicesDialog");
    devicesDialog.showDevicesInfo = s.value("showDevicesInfo", false).toBool();
    devicesDialog.showIPv4 = s.value("showIPv4", true).toBool();
    devicesDialog.showIPv6 = s.value("showIPv6", true).toBool();
    s.endGroup();

    s.beginGroup("ExportDataDialog");
    exportDataDialog.users = s.value("users", true).toBool();
    exportDataDialog.packets = s.value("packets", true).toBool();
    exportDataDialog.transfers = s.value("transfers", true).toBool();
    exportDataDialog.applications = s.value("applications", true).toBool();
    exportDataDialog.hosts = s.value("hosts", true).toBool();
    exportDataDialog.fields = s.value("fields", 0).toInt();
    exportDataDialog.lines = s.value("lines", 0).toInt();
    exportDataDialog.header = s.value("header", false).toBool();
    exportDataDialog.folder = s.value("folder", QDir::toNativeSeparators(QDir::homePath())).toString();
    exportDataDialog.openAfter = s.value("openAfter", false).toBool();
    exportDataDialog.summary = s.value("summary", true).toBool();
    s.endGroup();

    s.beginGroup("EventsViewerMainWindow");
    eventsViewerMainWindow.maximized = s.value("maximized", false).toBool();
    eventsViewerMainWindow.size = s.value("size", QSize((int)(QApplication::desktop()->width()/6)*4, (int)(QApplication::desktop()->height()/4)*2)).toSize();
    eventsViewerMainWindow.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/6), (int)(QApplication::desktop()->height()/4))).toPoint();
    eventsViewerMainWindow.toolbarsState = s.value("toolbarsState", QByteArray()).toByteArray();
    eventsViewerMainWindow.toolbarsDefault = s.value("toolbarsDefault", QByteArray()).toByteArray();
    eventsViewerMainWindow.toolbarsMovable = s.value("toolbarsMovable", true).toBool();
    eventsViewerMainWindow.toolbarsStyle = s.value("toolbarsStyle", 3).toInt();
    eventsViewerMainWindow.toolbarsIconSize = s.value("toolbarsIconSize", 24).toInt();
    eventsViewerMainWindow.statusBar = s.value("statusBar", true).toBool();
    eventsViewerMainWindow.alwaysOnTop = s.value("alwaysOnTop", false).toBool();
    eventsViewerMainWindow.information = s.value("information", true).toBool();
    eventsViewerMainWindow.warning = s.value("warning", true).toBool();
    eventsViewerMainWindow.critical = s.value("critical", true).toBool();
    eventsViewerMainWindow.createFile = s.value("createFile", true).toBool();
    eventsViewerMainWindow.folder = s.value("folder", QDir::toNativeSeparators(QCoreApplication::applicationDirPath())).toString();
    s.endGroup();

    s.beginGroup("PacketsMainWindow");
    packetsMainWindow.maximized = s.value("maximized", false).toBool();
    packetsMainWindow.size = s.value("size", QSize((int)(QApplication::desktop()->width()/6)*4, (int)(QApplication::desktop()->height()/4)*2)).toSize();
    packetsMainWindow.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/6), (int)(QApplication::desktop()->height()/4))).toPoint();
    packetsMainWindow.toolbarsState = s.value("toolbarsState", QByteArray()).toByteArray();
    packetsMainWindow.toolbarsDefault = s.value("toolbarsDefault", QByteArray()).toByteArray();
    packetsMainWindow.toolbarsMovable = s.value("toolbarsMovable", true).toBool();
    packetsMainWindow.toolbarsStyle = s.value("toolbarsStyle", 3).toInt();
    packetsMainWindow.toolbarsIconSize = s.value("toolbarsIconSize", 24).toInt();
    packetsMainWindow.statusBar = s.value("statusBar", true).toBool();
    packetsMainWindow.alwaysOnTop = s.value("alwaysOnTop", false).toBool();
    packetsMainWindow.autoScroll = s.value("autoScroll", false).toBool();
    s.endGroup();

    s.beginGroup("Screenshots");
    screenshots.dialog = s.value("dialog", true).toBool();
    screenshots.format = s.value("format", "Portable Network Graphics (*.png)").toString();
    screenshots.extension = s.value("extension", "png").toString();
    screenshots.quality = s.value("quality", -1).toInt();
    screenshots.folder = s.value("folder", QDir::toNativeSeparators(QDir::homePath())).toString();
    screenshots.name = s.value("name", "%D_%T_%N").toString();
    s.endGroup();

    s.beginGroup("NetPacketsGraphDialog");
    netPacketsGraphDialog.size = s.value("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4)).toSize();
    netPacketsGraphDialog.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6))).toPoint();
    netPacketsGraphDialog.time = s.value("time", 0).toInt();
    netPacketsGraphDialog.h = s.value("h", true).toBool();
    netPacketsGraphDialog.v = s.value("v", false).toBool();
    netPacketsGraphDialog.background = s.value("background", true).toBool();
    netPacketsGraphDialog.filled = s.value("filled", false).toBool();
    netPacketsGraphDialog.antialiasing = s.value("antialiasing", false).toBool();
    netPacketsGraphDialog.tooltip = s.value("tooltip", true).toBool();
    s.endGroup();

    s.beginGroup("NetTransferGraphDialog");
    netTransferGraphDialog.size = s.value("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4)).toSize();
    netTransferGraphDialog.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6))).toPoint();
    netTransferGraphDialog.time = s.value("time", 0).toInt();
    netTransferGraphDialog.show = s.value("show", 0).toInt();
    netTransferGraphDialog.h = s.value("h", true).toBool();
    netTransferGraphDialog.v = s.value("v", false).toBool();
    netTransferGraphDialog.background = s.value("background", true).toBool();
    netTransferGraphDialog.filled = s.value("filled", false).toBool();
    netTransferGraphDialog.antialiasing = s.value("antialiasing", false).toBool();
    netTransferGraphDialog.tooltip = s.value("tooltip", true).toBool();
    s.endGroup();

    s.beginGroup("UserTransfersGraphDialog");
    userTransfersGraphDialog.size = s.value("size", QSize((int)(QApplication::desktop()->width()/4)*2, (int)(QApplication::desktop()->height()/6)*4)).toSize();
    userTransfersGraphDialog.position = s.value("position", QPoint((int)(QApplication::desktop()->width()/4), (int)(QApplication::desktop()->height()/6))).toPoint();
    userTransfersGraphDialog.time = s.value("time", 0).toInt();
    userTransfersGraphDialog.show = s.value("show", 0).toInt();
    userTransfersGraphDialog.h = s.value("h", true).toBool();
    userTransfersGraphDialog.v = s.value("v", false).toBool();
    userTransfersGraphDialog.background = s.value("background", true).toBool();
    userTransfersGraphDialog.filled = s.value("filled", false).toBool();
    userTransfersGraphDialog.antialiasing = s.value("antialiasing", false).toBool();
    userTransfersGraphDialog.tooltip = s.value("tooltip", true).toBool();
    s.endGroup();

    s.beginGroup("NetTransferDialog");
    netTransferDialog.up = s.value("up", 0).toInt();
    netTransferDialog.down = s.value("down", 0).toInt();
    s.endGroup();

    s.sync();
    switch (s.status())
    {
        case QSettings::AccessError:
                            //QMessageBox::warning(this, tr("Reading settings from file"), tr("An access error occurred (trying to read the file). Default settings loaded."));
                            error = 2;
                            break;
        case QSettings::FormatError:
                            //QMessageBox::warning(this, tr("Reading settings from file"), tr("A format error occurred (a malformed INI file) while reading the file. Some settings reset to defaults."));
                            error = 3;
                            break;
        default: break;
    }
}

void Settings::write()
{
    error = 0;

    QSettings s(settingsFileName, QSettings::IniFormat, this);

    s.beginGroup("MainWindow");
    s.setValue("firstRun", mainWindow.firstRun);
    s.setValue("languageFile", mainWindow.languageFile);
    s.setValue("startMaximized", mainWindow.startMaximized);
    s.setValue("maximized", mainWindow.maximized);
    s.setValue("fullScreen", mainWindow.fullScreen);
    s.setValue("size", mainWindow.size);
    s.setValue("position", mainWindow.position);
    s.setValue("toolbarsState", mainWindow.toolbarsState);
    s.setValue("toolbarsDefault", mainWindow.toolbarsDefault);
    s.setValue("toolbarsMovable", mainWindow.toolbarsMovable);
    s.setValue("toolbarsStyle", mainWindow.toolbarsStyle);
    s.setValue("toolbarsIconSize", mainWindow.toolbarsIconSize);
    s.setValue("statusBar", mainWindow.statusBar);
    s.setValue("trayIcon", mainWindow.trayIcon);
    s.setValue("alwaysOnTop", mainWindow.alwaysOnTop);
    s.setValue("currentTab", mainWindow.currentTab);
    s.setValue("splitterApplication", mainWindow.splitterApplication);
    s.setValue("splitterHosts", mainWindow.splitterHosts);
    s.setValue("deviceName", mainWindow.deviceName);
    s.setValue("filterName", mainWindow.filterName);
    s.setValue("filterCode", mainWindow.filterCode);
    s.setValue("autoStart", mainWindow.autoStart);
    s.setValue("minimizeToTrayOnStart", mainWindow.minimizeToTrayOnStart);
    s.setValue("minimizeToTrayOnClose", mainWindow.minimizeToTrayOnClose);
    s.setValue("autoCheckUpdate", mainWindow.autoCheckUpdate);
    s.endGroup();

    s.beginGroup("CaptureThread");
    s.setValue("mode", captureThread.mode);
    s.setValue("bytes", captureThread.bytes);
    s.setValue("timeout", captureThread.timeout);
    s.endGroup();

    s.beginGroup("DevicesDialog");
    s.setValue("showDevicesInfo", devicesDialog.showDevicesInfo);
    s.setValue("showIPv4", devicesDialog.showIPv4);
    s.setValue("showIPv6", devicesDialog.showIPv6);
    s.endGroup();

    s.beginGroup("ExportDataDialog");
    s.setValue("users", exportDataDialog.users);
    s.setValue("packets", exportDataDialog.packets);
    s.setValue("transfers", exportDataDialog.transfers);
    s.setValue("applications", exportDataDialog.applications);
    s.setValue("hosts", exportDataDialog.hosts);
    s.setValue("fields", exportDataDialog.fields);
    s.setValue("lines", exportDataDialog.lines);
    s.setValue("header", exportDataDialog.header);
    s.setValue("folder", exportDataDialog.folder);
    s.setValue("openAfter", exportDataDialog.openAfter);
    s.setValue("summary", exportDataDialog.summary);
    s.endGroup();

    s.beginGroup("EventsViewerMainWindow");
    s.setValue("maximized", eventsViewerMainWindow.maximized);
    s.setValue("size", eventsViewerMainWindow.size);
    s.setValue("position", eventsViewerMainWindow.position);
    s.setValue("toolbarsState", eventsViewerMainWindow.toolbarsState);
    s.setValue("toolbarsDefault", eventsViewerMainWindow.toolbarsDefault);
    s.setValue("toolbarsMovable", eventsViewerMainWindow.toolbarsMovable);
    s.setValue("toolbarsStyle", eventsViewerMainWindow.toolbarsStyle);
    s.setValue("toolbarsIconSize", eventsViewerMainWindow.toolbarsIconSize);
    s.setValue("statusBar", eventsViewerMainWindow.statusBar);
    s.setValue("alwaysOnTop", eventsViewerMainWindow.alwaysOnTop);
    s.setValue("information", eventsViewerMainWindow.information);
    s.setValue("warning", eventsViewerMainWindow.warning);
    s.setValue("critical", eventsViewerMainWindow.critical);
    s.setValue("createFile", eventsViewerMainWindow.createFile);
    s.setValue("folder", eventsViewerMainWindow.folder);
    s.endGroup();

    s.beginGroup("PacketsMainWindow");
    s.setValue("maximized", packetsMainWindow.maximized);
    s.setValue("size", packetsMainWindow.size);
    s.setValue("position", packetsMainWindow.position);
    s.setValue("toolbarsState", packetsMainWindow.toolbarsState);
    s.setValue("toolbarsDefault", packetsMainWindow.toolbarsDefault);
    s.setValue("toolbarsMovable", packetsMainWindow.toolbarsMovable);
    s.setValue("toolbarsStyle", packetsMainWindow.toolbarsStyle);
    s.setValue("toolbarsIconSize", packetsMainWindow.toolbarsIconSize);
    s.setValue("statusBar", packetsMainWindow.statusBar);
    s.setValue("alwaysOnTop", packetsMainWindow.alwaysOnTop);
    s.setValue("autoScroll", packetsMainWindow.autoScroll);
    s.endGroup();

    s.beginGroup("Screenshots");
    s.setValue("dialog", screenshots.dialog);
    s.setValue("format", screenshots.format);
    s.setValue("extension", screenshots.extension);
    s.setValue("quality", screenshots.quality);
    s.setValue("folder", screenshots.folder);
    s.setValue("name", screenshots.name);
    s.endGroup();

    s.beginGroup("NetPacketsGraphDialog");
    s.setValue("size", netPacketsGraphDialog.size);
    s.setValue("position", netPacketsGraphDialog.position);
    s.setValue("time", netPacketsGraphDialog.time);
    s.setValue("h", netPacketsGraphDialog.h);
    s.setValue("v", netPacketsGraphDialog.v);
    s.setValue("background", netPacketsGraphDialog.background);
    s.setValue("filled", netPacketsGraphDialog.filled);
    s.setValue("antialiasing", netPacketsGraphDialog.antialiasing);
    s.setValue("tooltip", netPacketsGraphDialog.tooltip);
    s.endGroup();

    s.beginGroup("NetTransferGraphDialog");
    s.setValue("size", netTransferGraphDialog.size);
    s.setValue("position", netTransferGraphDialog.position);
    s.setValue("time", netTransferGraphDialog.time);
    s.setValue("show", netTransferGraphDialog.show);
    s.setValue("h", netTransferGraphDialog.h);
    s.setValue("v", netTransferGraphDialog.v);
    s.setValue("background", netTransferGraphDialog.background);
    s.setValue("filled", netTransferGraphDialog.filled);
    s.setValue("antialiasing", netTransferGraphDialog.antialiasing);
    s.setValue("tooltip", netTransferGraphDialog.tooltip);
    s.endGroup();

    s.beginGroup("UserTransfersGraphDialog");
    s.setValue("size", userTransfersGraphDialog.size);
    s.setValue("position", userTransfersGraphDialog.position);
    s.setValue("time", userTransfersGraphDialog.time);
    s.setValue("show", userTransfersGraphDialog.show);
    s.setValue("h", userTransfersGraphDialog.h);
    s.setValue("v", userTransfersGraphDialog.v);
    s.setValue("background", userTransfersGraphDialog.background);
    s.setValue("filled", userTransfersGraphDialog.filled);
    s.setValue("antialiasing", userTransfersGraphDialog.antialiasing);
    s.setValue("tooltip", userTransfersGraphDialog.tooltip);
    s.endGroup();

    s.beginGroup("NetTransferDialog");
    s.setValue("up", netTransferDialog.up);
    s.setValue("down", netTransferDialog.down);
    s.endGroup();

    s.sync();
    switch (s.status())
    {
        case QSettings::AccessError:
                            error = 1;
                            //QMessageBox::warning(this, tr("Writing settings to file"), tr("An access error occurred (trying to write the file). Settings not saved."));
                            break;
        case QSettings::FormatError:
                            // settings are saved, the file is repaired !!!

                            //error = 2;
                            //QMessageBox::warning(this, tr("Writing settings to file"), tr("A format error occurred. Settings not saved."));
                            break;
        default: break;
    }
}
