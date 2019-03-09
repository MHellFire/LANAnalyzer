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

#ifndef FILTERSDIALOG_H
#define FILTERSDIALOG_H

#include "ui_filtersdialog.h"

#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QToolTip>

#include "editordialog.h"

class FiltersDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(FiltersDialog)

public:
    explicit FiltersDialog(QWidget *parent = 0, const QString &filterName = "");

    void prepareDialog();
    QString getFilterCode();
    QString getFilterName();

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::FiltersDialogClass ui;

    QString filterCode, filterName;
    QString fileName;

    QIcon *topIcon;
    QIcon *topOpenIcon;
    QIcon *itemIcon;

    struct Filter
    {
        QString filterName;
        QString filterCode;
    };

    QList<Filter> linkFilters, networkFilters, transportFilters, applicationFilters, otherFilters;

    void createConnections();

    void showFilters();
    void setFilter();
    void readFilters();
    void writeFilters();
    void loadExamples();

private slots:
    void onShowShortHelp();
    void onLoadExamples();
    void onOK();
    void onUpdate();
    void onAddNew();
    void onDelete();
    void onOpenEditorName();
    void onOpenEditorCode();

    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemExpanded(QTreeWidgetItem *item);
    void onItemCollapsed(QTreeWidgetItem *item);

signals:
    void infoMessage(quint8 type, const QString &title, const QString &message);
};

#endif // FILTERSDIALOG_H
