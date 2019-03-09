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

#ifndef PACKETSMAINWINDOW_H
#define PACKETSMAINWINDOW_H

#include "ui_packetsmainwindow.h"

#include <QResizeEvent>
#include <QMoveEvent>
#include <QLabel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

#include "aboutdialog.h"
#include "capturethread.h"
#include "settings.h"

class PacketsMainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(PacketsMainWindow)

public:
    explicit PacketsMainWindow(QWidget *parent = 0, CaptureThread *thread = 0);
    ~PacketsMainWindow();

    void clearTree();
    void writeSettings();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void moveEvent(QMoveEvent *event);

private:
    Ui::PacketsMainWindowClass ui;

    QSize oldSize;
    QPoint oldPosition;

    CaptureThread *thread;

    quint64 packetsNo;
    QString typeStr;

    struct in_addr sAddrIP, dAddrIP;

    bool autoScroll;

    // menu
    QActionGroup *styleActionGroup;
    QActionGroup *sizeActionGroup;

    // toolbars
    // file
    QAction *exportDataAct;

    // view
    QAction *startAct;
    QAction *stopAct;
    QAction *clearAct;
    QAction *autoScrollAct;

    // help
    QMenu *helpActMenu;
    QAction *helpAct;
    QAction *aboutAct;

    // statusbar
    QLabel *infoLabel;

    void createMenu();
    void createToolbars();
    void setToolbarIcons(int size);
    void createStatusBar();
    void restoreWindowState();

private slots:
    void receivedPacket(const QString &time,
                        quint32 length,
                        const QString &sMac,
                        const QString &dMac,
                        quint16 type,
                        quint32 sIP,
                        quint32 dIP,
                        quint32 sPort,
                        quint32 dPort,
                        const QString &info);

    void onExportData();

    void onStart();
    void onStop();
    void onClear();
    void onAutoScrollTriggered(bool checked);

    void onChangeMovable(bool movable);
    void onToolbarsStyleChanged(QAction *action);
    void onToolbarsSizeChanged(QAction *action);
    void onDefaultToolbars();

    void toggleAlwaysOnTop(bool);

    void showHelp();
    void showAbout();
};

#endif // PACKETSMAINWINDOW_H
