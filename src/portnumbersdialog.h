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

#ifndef PORTNUMBERSDIALOG_H
#define PORTNUMBERSDIALOG_H

#include "ui_portnumbersdialog.h"

#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QToolTip>

class PortNumbersDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(PortNumbersDialog)

public:
    explicit PortNumbersDialog(QWidget *parent = 0);

    bool isModified() const { return modified; }

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::PortNumbersDialogClass ui;

    QString oldValue;

    bool modified;

    void createConnections();

    void readPorts();
    void writePorts();

private slots:
    void onShowShortHelp();
    void onOK();
    void onSave();
    void onAddNew();
    void onDelete();
    void onSearch();

    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void onTextChanged(const QString &text);
};

#endif // PORTNUMBERSDIALOG_H
