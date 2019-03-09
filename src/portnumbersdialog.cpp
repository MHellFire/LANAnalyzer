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

#include "portnumbersdialog.h"

class MyColumnTreeWidgetItem : public QTreeWidgetItem
{
public:
    MyColumnTreeWidgetItem(QTreeWidget *parent) : QTreeWidgetItem(parent)
    {
    }

    MyColumnTreeWidgetItem(QTreeWidget *parent, const QStringList &strings) : QTreeWidgetItem (parent, strings)
    {
    }

private:
    bool operator< (const QTreeWidgetItem &other) const
    {
        int sortCol = treeWidget()->sortColumn();
        if (sortCol == 1)
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

PortNumbersDialog::PortNumbersDialog(QWidget *parent)
    : QDialog(parent)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui.setupUi(this);

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui.treeWidget->clear();

    modified = false;

    readPorts();

    createConnections();

    ui.lineEdit->setFocus();
}

void PortNumbersDialog::keyPressEvent(QKeyEvent *event)
{
    if (ui.treeWidget->hasFocus())
    {
        if (event->key() == Qt::Key_Delete)
        {
            onDelete();
            return;
        }

        if (event->key() == Qt::Key_Insert)
        {
            onAddNew();
            return;
        }
    }

    if (event->key() == Qt::Key_F3)
    {
        ui.lineEdit->setFocus();
        return;
    }

    QDialog::keyPressEvent(event);
    return;
}

void PortNumbersDialog::createConnections()
{
    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ui.pushButtonAddNew, SIGNAL(clicked()), this, SLOT(onAddNew()));
    connect(ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(onDelete()));

    connect(ui.pushButtonSearch, SIGNAL(clicked()), this, SLOT(onSearch()));

    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));

    connect(ui.pushButtonShortHelp, SIGNAL(clicked()), this, SLOT(onShowShortHelp()));

    connect(ui.treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui.treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(onItemChanged(QTreeWidgetItem*,int)));
    connect(ui.lineEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
}

void PortNumbersDialog::readPorts()
{
    QFile file(QCoreApplication::applicationDirPath() + "/ports.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(0, tr("Critical"), tr("Unable to open ports file:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    ui.treeWidget->setSortingEnabled(false);

    QString line;
    int i = 0;

    while (!in.atEnd())
    {
        line = in.readLine();

        ui.treeWidget->addTopLevelItem(new MyColumnTreeWidgetItem(ui.treeWidget, QStringList() << line.left(3) << line.mid(4, line.indexOf("=", 0) - 4) << line.right(line.length() - line.indexOf("=", 3) -1).simplified()));
        ui.treeWidget->topLevelItem(i)->setFlags(ui.treeWidget->topLevelItem(i)->flags() | Qt::ItemIsEditable);

        ++i;
    }

    ui.treeWidget->setSortingEnabled(true);

    ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(0));

    file.close();
}

void PortNumbersDialog::writePorts()
{
    QFile file(QCoreApplication::applicationDirPath() + "/ports.txt");

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Critical"), tr("Unable to save ports file:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // save sorting settings
    int sortColumn = ui.treeWidget->sortColumn();
    Qt::SortOrder sortOrder = ui.treeWidget->header()->sortIndicatorOrder();

    // sorting by port number and writing to file

    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui.treeWidget->setSortingEnabled(false);

    for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
    {
        out << ui.treeWidget->topLevelItem(i)->text(0).toLower() << " " << ui.treeWidget->topLevelItem(i)->text(1) << "=" << ui.treeWidget->topLevelItem(i)->text(2).simplified() << "\r\n";
    }

    // restore sorting settings
    ui.treeWidget->setSortingEnabled(true);
    ui.treeWidget->sortByColumn(sortColumn, sortOrder);

    file.close();
    
    modified = true;
}

void PortNumbersDialog::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current != 0)
        ui.pushButtonDelete->setEnabled(true);
    else
        ui.pushButtonDelete->setDisabled(true);
}

void PortNumbersDialog::onSave()
{
    writePorts();

    ui.pushButtonSave->setDisabled(true);
}

void PortNumbersDialog::onAddNew()
{
    QTreeWidgetItem *currentItem = ui.treeWidget->currentItem();

    if (currentItem != 0)
    {
        MyColumnTreeWidgetItem *item = new MyColumnTreeWidgetItem(ui.treeWidget, QStringList() << currentItem->text(0) << currentItem->text(1) << currentItem->text(2));
        ui.treeWidget->addTopLevelItem(item);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
    else
    {
        MyColumnTreeWidgetItem *item = new MyColumnTreeWidgetItem(ui.treeWidget, QStringList() << "tcp" << "0" << "");
        ui.treeWidget->addTopLevelItem(item);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void PortNumbersDialog::onDelete()
{
    if (ui.treeWidget->takeTopLevelItem(ui.treeWidget->indexOfTopLevelItem(ui.treeWidget->currentItem())) != 0)
        ui.pushButtonSave->setEnabled(true);
}

void PortNumbersDialog::onTextChanged(const QString &text)
{
    if (text != "")
    {
        ui.pushButtonSearch->setEnabled(true);
        ui.pushButtonSearch->setDefault(true);
    }
    else
    {
        ui.pushButtonSearch->setDisabled(true);
        ui.pushButtonSearch->setDefault(false);
    }
}

void PortNumbersDialog::onSearch()
{
    QString str = ui.lineEdit->text();
    bool ok;
    int x, i;

    x = str.toInt(&ok, 10);

    if (!ok || x < 0 || x > 65535) // search description
    {
        for (i = ui.treeWidget->indexOfTopLevelItem(ui.treeWidget->currentItem()) + 1; i < ui.treeWidget->topLevelItemCount(); ++i)
            if (ui.treeWidget->topLevelItem(i)->text(2).contains(str, Qt::CaseInsensitive))
            {
                ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(i));
                return;
            }

        for (i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
            if (ui.treeWidget->topLevelItem(i)->text(2).contains(str, Qt::CaseInsensitive))
            {
                ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(i));
                return;
            }
    }
    else //search port number
    {
        for (i = ui.treeWidget->indexOfTopLevelItem(ui.treeWidget->currentItem()) + 1; i < ui.treeWidget->topLevelItemCount(); ++i)
            if (ui.treeWidget->topLevelItem(i)->text(1) == str)
            {
                ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(i));
                return;
            }

        for (i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
            if (ui.treeWidget->topLevelItem(i)->text(1) == str)
            {
                ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(i));
                return;
            }
    }
}

void PortNumbersDialog::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    oldValue = item->text(column);
}

void PortNumbersDialog::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == 0)
    {
        if (item->text(0).toLower() != "tcp" && item->text(0).toLower() != "udp")
        {
            QMessageBox::information(this, tr("Information"), tr("Not valid protocol type!"));

            if (oldValue != "")
                item->setText(0, oldValue);
            else
                item->setText(0, "tcp");
        }
    }

    if (column == 1)
    {
        bool ok;
        int x = item->text(1).toInt(&ok, 10);

        if (!ok || x < 0 || x > 65535)
        {
            QMessageBox::information(this, tr("Information"), tr("Not valid port number!"));

            if (oldValue != "")
                item->setText(1, oldValue);
            else
                item->setText(1, "0");
        }
    }

    ui.pushButtonSave->setEnabled(true);
}

void PortNumbersDialog::onOK()
{
    if (ui.pushButtonSave->isEnabled())
        writePorts();

    accept();
    close();
}

void PortNumbersDialog::onShowShortHelp()
{
    QToolTip::showText(mapToGlobal(ui.pushButtonShortHelp->pos()), ui.pushButtonShortHelp->toolTip());
}
