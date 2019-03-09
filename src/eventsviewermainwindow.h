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

#ifndef EVENTSVIEWERMAINWINDOW_H
#define EVENTSVIEWERMAINWINDOW_H

#include "ui_eventsviewermainwindow.h"

#include <QResizeEvent>
#include <QMoveEvent>
#include <QLabel>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QClipboard>

#include "aboutdialog.h"
#include "settings.h"

enum { EVENT_INFORMATION, EVENT_WARNING, EVENT_CRITICAL };

struct Event
{
    quint32 id;
    QString type;
    QString date;
    QString time;
    QString event;
    QString details;
};

class EventsViewerMainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(EventsViewerMainWindow)

public:
    explicit EventsViewerMainWindow(QWidget *parent = 0);
    ~EventsViewerMainWindow();

    //const quint8 EVENT_INFORMATION = 0;
    //const quint8 EVENT_WARNING = 1;
    //const quint8 EVENT_CRITICAL = 2;

    void addEvent(quint8 type, const QString &event, const QString &details);
    void writeSettings();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void moveEvent(QMoveEvent *event);

private:
    Ui::EventsViewerMainWindowClass ui;

    QSize oldSize;
    QPoint oldPosition;

    QList<Event> eventsList;
    quint32 infoCounter, warningCounter, criticalCounter;

    // menu
    QActionGroup *styleActionGroup;
    QActionGroup *sizeActionGroup;

    // toolbars
    // file
    QAction *saveAsAct;

    // filter
    QAction *informationAct;
    QAction *warningAct;
    QAction *criticalAct;

    // help
    QMenu *helpActMenu;
    QAction *helpAct;
    QAction *aboutAct;

    // status bar
    QLabel *eventsLabel, *infoLabel, *warningLabel, *criticalLabel;

    void createMenu();
    void createToolbars();
    void setToolbarIcons(int size);
    void createStatusBar();
    void restoreWindowState();

    bool saveFile(const QString &fileName);

    void updateEventsWidget();

private slots:
    void onSaveAs();

    void onInformationTriggered(bool checked);
    void onWarningTriggered(bool checked);
    void onCriticalTriggered(bool checked);

    void onChangeMovable(bool movable);
    void onToolbarsStyleChanged(QAction *action);
    void onToolbarsSizeChanged(QAction *action);
    void onDefaultToolbars();

    void toggleAlwaysOnTop(bool);

    void showHelp();
    void showAbout();

    void onShowContextMenu(const QPoint &pos);
};

#endif // EVENTSVIEWERMAINWINDOW_H
