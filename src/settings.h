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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QSize>
#include <QPoint>
#include <QDir>
#include <QFile>

struct MainWindowSettings
{
    bool firstRun;
    QString languageFile;
    bool startMaximized;
    bool maximized;
    bool fullScreen;
    QSize size;
    QPoint position;
    QByteArray toolbarsState;
    QByteArray toolbarsDefault;
    bool toolbarsMovable;
    int toolbarsStyle;
    int toolbarsIconSize;
    bool statusBar;
    bool trayIcon;
    bool alwaysOnTop;
    int currentTab;
    QByteArray splitterApplication;
    QByteArray splitterHosts;
    QString deviceName;
    QString filterName;
    QString filterCode;
    bool autoStart;
    bool minimizeToTrayOnStart;
    bool minimizeToTrayOnClose;
    bool autoCheckUpdate;
};

struct CaptureThreadSettings
{
    int mode;
    int bytes;
    int timeout;
};

struct DevicesDialogSettings
{
    bool showDevicesInfo;
    bool showIPv4;
    bool showIPv6;
};

struct ExportDataDialogSettings
{
    bool users;
    bool packets;
    bool transfers;
    bool applications;
    bool hosts;
    int fields;
    int lines;
    bool header;
    QString folder;
    bool openAfter;
    bool summary;
};

struct EventsViewerMainWindowSettings
{
    bool maximized;
    QSize size;
    QPoint position;
    QByteArray toolbarsState;
    QByteArray toolbarsDefault;
    bool toolbarsMovable;
    int toolbarsStyle;
    int toolbarsIconSize;
    bool statusBar;
    bool alwaysOnTop;
    bool information;
    bool warning;
    bool critical;
    bool createFile;
    QString folder;
};

struct PacketsMainWindowSettings
{
    bool maximized;
    QSize size;
    QPoint position;
    QByteArray toolbarsState;
    QByteArray toolbarsDefault;
    bool toolbarsMovable;
    int toolbarsStyle;
    int toolbarsIconSize;
    bool statusBar;
    bool alwaysOnTop;
    bool autoScroll;
};

struct ScreenshotsSettings
{
    bool dialog;
    QString format;
    QString extension;
    int quality;
    QString folder;
    QString name;
};

struct NetPacketsGraphDialogSettings
{
    QSize size;
    QPoint position;
    int time;
    bool h;
    bool v;
    bool background;
    bool filled;
    bool antialiasing;
    bool tooltip;
};

struct NetTransferGraphDialogSettings
{
    QSize size;
    QPoint position;
    int time;
    int show;
    bool h;
    bool v;
    bool background;
    bool filled;
    bool antialiasing;
    bool tooltip;
};

struct UserTransfersGraphDialogSettings
{
    QSize size;
    QPoint position;
    int time;
    int show;
    bool h;
    bool v;
    bool background;
    bool filled;
    bool antialiasing;
    bool tooltip;
};

struct NetTransferDialogSettings
{
    int up;
    int down;
};

class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)

public:
    explicit Settings(QObject *parent = 0);
    ~Settings();

    void read();
    void write();

    int getError() const { return error; }

    static MainWindowSettings mainWindow;
    static CaptureThreadSettings captureThread;
    static DevicesDialogSettings devicesDialog;
    static ExportDataDialogSettings exportDataDialog;
    static EventsViewerMainWindowSettings eventsViewerMainWindow;
    static PacketsMainWindowSettings packetsMainWindow;
    static ScreenshotsSettings screenshots;
    static NetPacketsGraphDialogSettings netPacketsGraphDialog;
    static NetTransferGraphDialogSettings netTransferGraphDialog;
    static UserTransfersGraphDialogSettings userTransfersGraphDialog;
    static NetTransferDialogSettings netTransferDialog;

private:
    int error;

    QString settingsFileName;

    void create();
};

#endif // SETTINGS_H
