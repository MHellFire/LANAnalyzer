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

#include "eventsviewermainwindow.h"

class MyTreeWidgetItem : public QTreeWidgetItem
{
public:
    MyTreeWidgetItem(QTreeWidget *parent) : QTreeWidgetItem(parent)
    {
    }

    MyTreeWidgetItem(QTreeWidget *parent, const QStringList &strings) : QTreeWidgetItem (parent, strings)
    {
    }

private:
    bool operator< (const QTreeWidgetItem &other) const
    {
        int sortCol = treeWidget()->sortColumn();
        if (sortCol == 0)
        {
            int myNumber = text(sortCol).toInt();
            int otherNumber = other.text(sortCol).toInt();
            return myNumber < otherNumber;
        }
        else
        {
            return text(sortCol) < other.text(sortCol);
        }
    }
};

EventsViewerMainWindow::EventsViewerMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.treeWidget->resizeColumnToContents(0);

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);

    ui.treeWidget->clear();
    ui.treeWidget->sortItems(0, Qt::AscendingOrder);

    eventsList.clear();

    infoCounter = 0;
    warningCounter = 0;
    criticalCounter = 0;

    createMenu();
    createToolbars();
    createStatusBar();

    restoreWindowState();

    connect(ui.treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onShowContextMenu(QPoint)));

    if (Settings::mainWindow.firstRun)
    {
        Settings::eventsViewerMainWindow.toolbarsState = saveState(9);
        Settings::eventsViewerMainWindow.toolbarsDefault = saveState(9);
    }
}

EventsViewerMainWindow::~EventsViewerMainWindow()
{
    if (Settings::eventsViewerMainWindow.createFile)
        if (!saveFile(Settings::eventsViewerMainWindow.folder + "/lananalyzer.log"))
        {
            QMessageBox::critical(0, tr("Critical"), tr("Unable to write log file."));
        }
}

void EventsViewerMainWindow::resizeEvent(QResizeEvent *event)
{
    oldSize = event->oldSize();
}

void EventsViewerMainWindow::moveEvent(QMoveEvent *event)
{
    oldPosition = event->oldPos();
}

void EventsViewerMainWindow::createMenu()
{
    // file
    connect(ui.actionSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAs()));
    connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));

    // filter
    ui.actionInfomation->setChecked(Settings::eventsViewerMainWindow.information);
    connect(ui.actionInfomation, SIGNAL(triggered(bool)), this, SLOT(onInformationTriggered(bool)));
    ui.actionWarning->setChecked(Settings::eventsViewerMainWindow.warning);
    connect(ui.actionWarning, SIGNAL(triggered(bool)), this, SLOT(onWarningTriggered(bool)));
    ui.actionCritical->setChecked(Settings::eventsViewerMainWindow.critical);
    connect(ui.actionCritical, SIGNAL(triggered(bool)), this, SLOT(onCriticalTriggered(bool)));

    // options
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarFile->toggleViewAction());
    ui.menuToolbars->insertAction(ui.actionMovable, ui.toolBarFilter->toggleViewAction());
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

void EventsViewerMainWindow::createToolbars()
{
    // file
    saveAsAct = new QAction(tr("Save as..."), this);
    ui.toolBarFile->addAction(saveAsAct);
    saveAsAct->setEnabled(true);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(onSaveAs()));

    // filter
    informationAct = new QAction(tr("Information"), this);
    ui.toolBarFilter->addAction(informationAct);
    informationAct->setCheckable(true);
    informationAct->setChecked(Settings::eventsViewerMainWindow.information);
    connect(informationAct, SIGNAL(triggered(bool)), this, SLOT(onInformationTriggered(bool)));

    warningAct = new QAction(tr("Warning"), this);
    ui.toolBarFilter->addAction(warningAct);
    warningAct->setCheckable(true);
    warningAct->setChecked(Settings::eventsViewerMainWindow.warning);
    connect(warningAct, SIGNAL(triggered(bool)), this, SLOT(onWarningTriggered(bool)));

    criticalAct = new QAction(tr("Critical"), this);
    ui.toolBarFilter->addAction(criticalAct);
    criticalAct->setCheckable(true);
    criticalAct->setChecked(Settings::eventsViewerMainWindow.critical);
    connect(criticalAct, SIGNAL(triggered(bool)), this, SLOT(onCriticalTriggered(bool)));

    updateEventsWidget();

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

    restoreState(Settings::eventsViewerMainWindow.toolbarsState, 9);

    ui.actionMovable->setChecked(Settings::eventsViewerMainWindow.toolbarsMovable);

    switch (Settings::eventsViewerMainWindow.toolbarsStyle)
    {
        case 0: ui.actionIconOnly->trigger(); break;
        case 1: ui.actionTextOnly->trigger(); break;
        case 2: ui.actionTextBesideIcon->trigger(); break;
        case 3: ui.actionTextUnderIcon->trigger(); break;
        default: break;
    }

    switch (Settings::eventsViewerMainWindow.toolbarsIconSize)
    {
        case 16: ui.action16->trigger(); break;
        case 24: ui.action24->trigger(); break;
        case 32: ui.action32->trigger(); break;
        default: break;
    }

}

void EventsViewerMainWindow::onChangeMovable(bool movable)
{
    ui.toolBarFile->setMovable(movable);
    ui.toolBarFilter->setMovable(movable);
    ui.toolBarHelp->setMovable(movable);
}

void EventsViewerMainWindow::onToolbarsStyleChanged(QAction *action)
{
    if (ui.actionIconOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarFilter->setToolButtonStyle(Qt::ToolButtonIconOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    if (ui.actionTextOnly->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarFilter->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    if (ui.actionTextBesideIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarFilter->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    if (ui.actionTextUnderIcon->isChecked())
    {
        ui.toolBarFile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarFilter->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui.toolBarHelp->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
}

void EventsViewerMainWindow::onToolbarsSizeChanged(QAction *action)
{
    int size = 24;

    if (action->text().contains("16")) size = 16;
    if (action->text().contains("24")) size = 24;
    if (action->text().contains("32")) size = 32;

    ui.toolBarFile->setIconSize(QSize(size, size));
    ui.toolBarFilter->setIconSize(QSize(size, size));
    ui.toolBarHelp->setIconSize(QSize(size, size));

    setToolbarIcons(size);
}

void EventsViewerMainWindow::setToolbarIcons(int size)
{
    // file
    saveAsAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_save.png")));

    // filter
    informationAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_information.png")));
    warningAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_warning.png")));
    criticalAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_critical.png")));

    // help
    helpAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_help.png")));
    aboutAct->setIcon(QIcon(QString(":/images/"+QString::number(size)+"_about.png")));
}

void EventsViewerMainWindow::onDefaultToolbars()
{
    restoreState(Settings::eventsViewerMainWindow.toolbarsDefault, 9);
    ui.actionMovable->setChecked(true);
    ui.actionTextUnderIcon->trigger();
    ui.action24->trigger();
}

void EventsViewerMainWindow::createStatusBar()
{
    ui.statusbar->addPermanentWidget(eventsLabel = new QLabel(tr("Events: 0")), 1);
    ui.statusbar->addPermanentWidget(infoLabel = new QLabel(tr("Information events: 0")), 1);
    ui.statusbar->addPermanentWidget(warningLabel = new QLabel(tr("Warning events: 0")), 1);
    ui.statusbar->addPermanentWidget(criticalLabel = new QLabel(tr("Critical events: 0")), 1);
}

void EventsViewerMainWindow::restoreWindowState()
{
    resize(Settings::eventsViewerMainWindow.size);
    move(Settings::eventsViewerMainWindow.position);

    if (Settings::eventsViewerMainWindow.alwaysOnTop)
    {
        Qt::WindowFlags flags = windowFlags();
        flags.testFlag(Qt::WindowStaysOnTopHint) ? flags ^= Qt::WindowStaysOnTopHint : flags |= Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        ui.actionAlwaysOnTop->setChecked(true);
    }

    if (Settings::eventsViewerMainWindow.maximized)
    {
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }

    ui.actionStatusBar->setChecked(Settings::eventsViewerMainWindow.statusBar);
}

void EventsViewerMainWindow::onShowContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui.treeWidget->itemAt(pos);
    if (!item)
        return;

    QMenu menu(tr("Context menu"), this);
    menu.addAction(tr("Copy"));

    QAction *action = menu.exec(ui.treeWidget->viewport()->mapToGlobal(pos));

    if (action == 0)
        return;

    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setText(item->text(0) + " " + item->text(1) + " " + item->text(2) + " " + item->text(3) + " " + item->text(4) + " " + item->text(5));
}

void EventsViewerMainWindow::addEvent(quint8 type, const QString &event, const QString &details)
{
    int sortColumn = ui.treeWidget->sortColumn();
    Qt::SortOrder sortOrder = ui.treeWidget->header()->sortIndicatorOrder();
    ui.treeWidget->setSortingEnabled(false);

    Event e;
    int x = eventsList.count()+1;

    switch (type)
    {
        // information
        case EVENT_INFORMATION:
                e.id = x;
                e.type = tr("Information");
                e.date = QDate::currentDate().toString("yyyy-MM-dd");
                e.time = QTime::currentTime().toString("hh:mm:ss");
                e.event = event;
                e.details = details;

                eventsList.append(e);

                ui.treeWidget->addTopLevelItem(new MyTreeWidgetItem(ui.treeWidget, QStringList() << QString::number(x) << e.type << e.date << e.time << e.event << e.details));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setIcon(1, QIcon(":/images/16_information.png"));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setHidden(!ui.actionInfomation->isChecked());

                infoLabel->setText(QString("Information events: %1").arg(++infoCounter));
                eventsLabel->setText(QString("Events: %1").arg(infoCounter + warningCounter + criticalCounter));
                break;

        // warning
        case EVENT_WARNING:
                e.id = x;
                e.type = tr("Warning");
                e.date = QDate::currentDate().toString("yyyy-MM-dd");
                e.time = QTime::currentTime().toString("hh:mm:ss");
                e.event = event;
                e.details = details;

                eventsList.append(e);

                ui.treeWidget->addTopLevelItem(new MyTreeWidgetItem(ui.treeWidget, QStringList() << QString::number(x) << e.type << e.date << e.time << e.event << e.details));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setIcon(1, QIcon(":/images/16_warning.png"));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setHidden(!ui.actionWarning->isChecked());

                warningLabel->setText(QString("Warning events: %1").arg(++warningCounter));
                eventsLabel->setText(QString("Events: %1").arg(infoCounter + warningCounter + criticalCounter));
                break;

        // critical
        case EVENT_CRITICAL:
                e.id = x;
                e.type = tr("Critical");
                e.date = QDate::currentDate().toString("yyyy-MM-dd");
                e.time = QTime::currentTime().toString("hh:mm:ss");
                e.event = event;
                e.details = details;

                eventsList.append(e);

                ui.treeWidget->addTopLevelItem(new MyTreeWidgetItem(ui.treeWidget, QStringList() << QString::number(x) << e.type << e.date << e.time << e.event << e.details));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setIcon(1, QIcon(":/images/16_critical.png"));
                ui.treeWidget->topLevelItem(ui.treeWidget->topLevelItemCount()-1)->setHidden(!ui.actionCritical->isChecked());

                criticalLabel->setText(QString("Critical events: %1").arg(++criticalCounter));
                eventsLabel->setText(QString("Events: %1").arg(infoCounter + warningCounter + criticalCounter));
                break;

        default: break;
    }

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortItems(sortColumn, sortOrder);
}

void EventsViewerMainWindow::writeSettings()
{
    Settings::eventsViewerMainWindow.maximized = isMaximized();
    if (isMaximized())
    {
        Settings::eventsViewerMainWindow.size = oldSize;
        Settings::eventsViewerMainWindow.position = oldPosition;
    }
    else
    {
        Settings::eventsViewerMainWindow.size = size();
        Settings::eventsViewerMainWindow.position = pos();
    }

    Settings::eventsViewerMainWindow.toolbarsMovable = ui.actionMovable->isChecked();
    if (ui.actionIconOnly->isChecked()) Settings::eventsViewerMainWindow.toolbarsStyle = 0;
    if (ui.actionTextOnly->isChecked()) Settings::eventsViewerMainWindow.toolbarsStyle = 1;
    if (ui.actionTextBesideIcon->isChecked()) Settings::eventsViewerMainWindow.toolbarsStyle = 2;
    if (ui.actionTextUnderIcon->isChecked()) Settings::eventsViewerMainWindow.toolbarsStyle = 3;

    if (ui.action16->isChecked()) Settings::eventsViewerMainWindow.toolbarsIconSize = 16;
    if (ui.action24->isChecked()) Settings::eventsViewerMainWindow.toolbarsIconSize = 24;
    if (ui.action32->isChecked()) Settings::eventsViewerMainWindow.toolbarsIconSize = 32;

    Settings::eventsViewerMainWindow.toolbarsState = saveState(9);
    Settings::eventsViewerMainWindow.statusBar = ui.actionStatusBar->isChecked();
    Settings::eventsViewerMainWindow.alwaysOnTop = ui.actionAlwaysOnTop->isChecked();

    Settings::eventsViewerMainWindow.information = ui.actionInfomation->isChecked();
    Settings::eventsViewerMainWindow.warning = ui.actionWarning->isChecked();
    Settings::eventsViewerMainWindow.critical = ui.actionCritical->isChecked();
}

bool EventsViewerMainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    for (int i = 0; i < eventsList.count(); ++i)
    {
        out << QString::number(eventsList.at(i).id) << " ";
        out << eventsList.at(i).type << " ";
        out << eventsList.at(i).date << " ";
        out << eventsList.at(i).time << " ";
        out << eventsList.at(i).event << " ";
        out << eventsList.at(i).details << "\r\n";
    }

    file.close();

    return true;
}

void EventsViewerMainWindow::onSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save LANAnalyzer events log"), QDir::homePath() + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss"), tr("Log files (*.log);;Text files (*.txt);;All files (*.*)"));

    if (fileName.isEmpty())
        return;

    if (!saveFile(fileName))
        QMessageBox::critical(this, tr("Critical"), tr("Unable to write file."));
}

void EventsViewerMainWindow::onInformationTriggered(bool checked)
{
    ui.actionInfomation->setChecked(checked);
    informationAct->setChecked(checked);

    updateEventsWidget();
}

void EventsViewerMainWindow::onWarningTriggered(bool checked)
{
    ui.actionWarning->setChecked(checked);
    warningAct->setChecked(checked);

    updateEventsWidget();
}

void EventsViewerMainWindow::onCriticalTriggered(bool checked)
{
    ui.actionCritical->setChecked(checked);
    criticalAct->setChecked(checked);

    updateEventsWidget();
}

void EventsViewerMainWindow::updateEventsWidget()
{
    int sortColumn = ui.treeWidget->sortColumn();
    Qt::SortOrder sortOrder = ui.treeWidget->header()->sortIndicatorOrder();
    ui.treeWidget->setSortingEnabled(false);

    for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
    {
        if (ui.treeWidget->topLevelItem(i)->text(1).contains("Information", Qt::CaseSensitive))
            ui.treeWidget->topLevelItem(i)->setHidden(!ui.actionInfomation->isChecked());

        if (ui.treeWidget->topLevelItem(i)->text(1).contains("Warning", Qt::CaseSensitive))
            ui.treeWidget->topLevelItem(i)->setHidden(!ui.actionWarning->isChecked());

        if (ui.treeWidget->topLevelItem(i)->text(1).contains("Critical", Qt::CaseSensitive))
            ui.treeWidget->topLevelItem(i)->setHidden(!ui.actionCritical->isChecked());
    }

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortItems(sortColumn, sortOrder);
}

void EventsViewerMainWindow::toggleAlwaysOnTop(bool checked)
{
    setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    show();
}

void EventsViewerMainWindow::showHelp()
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/help.pdf")))
    {
        QMessageBox::critical(this, tr("Critical"), tr("The help file could not be open. Probably help file does not exist."));
    }
}

void EventsViewerMainWindow::showAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}
