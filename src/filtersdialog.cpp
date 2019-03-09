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

#include "filtersdialog.h"

FiltersDialog::FiltersDialog(QWidget *parent, const QString &filterName)
    : QDialog(parent), filterName(filterName)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui.setupUi(this);

    itemIcon = new QIcon(":/images/o_filter.png");
    topIcon = new QIcon(":/images/o_folder.png");
    topOpenIcon = new QIcon(":/images/o_folder_open.png");

    fileName = QCoreApplication::applicationDirPath() + "/filters.txt";
}

void FiltersDialog::keyPressEvent(QKeyEvent *event)
{
    if (ui.treeWidgetFilters->hasFocus())
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

        if (event->key() == Qt::Key_F2)
        {
            if (ui.treeWidgetFilters->currentItem()->parent())
                ui.lineEditName->setFocus();

            return;
        }
    }

    QDialog::keyPressEvent(event);
    return;
}

void FiltersDialog::prepareDialog()
{
    readFilters();
    showFilters();

    createConnections();

    setFilter();
}

QString FiltersDialog::getFilterName()
{
    return filterName;
}

QString FiltersDialog::getFilterCode()
{
    return filterCode;
}

void FiltersDialog::createConnections()
{
    connect(ui.pushButtonUpdate, SIGNAL(clicked()), this, SLOT(onUpdate()));
    connect(ui.pushButtonAddNew, SIGNAL(clicked()), this, SLOT(onAddNew()));
    connect(ui.pushButtonDelete, SIGNAL(clicked()), this, SLOT(onDelete()));

    connect(ui.pushButtonEditName, SIGNAL(clicked()), this, SLOT(onOpenEditorName()));
    connect(ui.pushButtonEditCode, SIGNAL(clicked()), this, SLOT(onOpenEditorCode()));

    connect(ui.pushButtonShortHelp, SIGNAL(clicked()), this, SLOT(onShowShortHelp()));
    connect(ui.pushButtonExamples, SIGNAL(clicked()), this, SLOT(onLoadExamples()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui.treeWidgetFilters, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui.treeWidgetFilters, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(onItemExpanded(QTreeWidgetItem*)));
    connect(ui.treeWidgetFilters, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(onItemCollapsed(QTreeWidgetItem*)));
    connect(ui.treeWidgetFilters, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onDoubleClicked(QTreeWidgetItem*,int)));
}

void FiltersDialog::readFilters()
{
    QFile file(fileName);

    if (!file.exists())
    {
        // 3 - critical
        emit infoMessage(3, tr("Reading filters from file"), tr("Filters file does not exist. Example filters loaded."));

        loadExamples();
        writeFilters();
        return;
    }

    QSettings filtersFile(fileName, QSettings::IniFormat, this);

    filtersFile.sync();
    switch (filtersFile.status())
    {
        case QSettings::AccessError:
                        emit infoMessage(3, tr("Reading filters from file"), tr("An access error occurred (trying to read the file). Example filters loaded."));
                        loadExamples();
                        writeFilters();
                        return;
                        break;
        case QSettings::FormatError:
                        emit infoMessage(3, tr("Reading filters from file"), tr("A format error occurred (a malformed file). Example filters loaded."));
                        loadExamples();
                        writeFilters();
                        return;
                        break;
        default:
            // no error
            break;
    }

    // no error

    Filter filter;
    int size, i;

    size = filtersFile.beginReadArray("Link");
    for (i = 0; i < size; ++i)
    {
        filtersFile.setArrayIndex(i);
        filter.filterName = filtersFile.value("filterName").toString();
        filter.filterCode = filtersFile.value("filterCode").toString();
        linkFilters.append(filter);
    }
    filtersFile.endArray();

    size = filtersFile.beginReadArray("Network");
    for (i = 0; i < size; ++i)
    {
        filtersFile.setArrayIndex(i);
        filter.filterName = filtersFile.value("filterName").toString();
        filter.filterCode = filtersFile.value("filterCode").toString();
        networkFilters.append(filter);
    }
    filtersFile.endArray();

    size = filtersFile.beginReadArray("Transport");
    for (i = 0; i < size; ++i)
    {
        filtersFile.setArrayIndex(i);
        filter.filterName = filtersFile.value("filterName").toString();
        filter.filterCode = filtersFile.value("filterCode").toString();
        transportFilters.append(filter);
    }
    filtersFile.endArray();

    size = filtersFile.beginReadArray("Application");
    for (i = 0; i < size; ++i)
    {
        filtersFile.setArrayIndex(i);
        filter.filterName = filtersFile.value("filterName").toString();
        filter.filterCode = filtersFile.value("filterCode").toString();
        applicationFilters.append(filter);
    }
    filtersFile.endArray();

    size = filtersFile.beginReadArray("Other");
    for (i = 0; i < size; ++i)
    {
        filtersFile.setArrayIndex(i);
        filter.filterName = filtersFile.value("filterName").toString();
        filter.filterCode = filtersFile.value("filterCode").toString();
        otherFilters.append(filter);
    }
    filtersFile.endArray();
}

void FiltersDialog::writeFilters()
{
    QSettings filtersFile(fileName, QSettings::IniFormat, this);
    int i;

    filtersFile.clear();
    filtersFile.sync();

    if (filtersFile.status() == QSettings::AccessError)
    {
        emit infoMessage(3, tr("Writing filters to file"), tr("An access error occurred (trying to write the file). Changes not saved."));
        return;
    }

    filtersFile.beginWriteArray("Link");
    for (i = 0; i < linkFilters.size(); ++i)
    {
        filtersFile.setArrayIndex(i);
        filtersFile.setValue("filterName", linkFilters.at(i).filterName);
        filtersFile.setValue("filterCode", linkFilters.at(i).filterCode);
    }
    filtersFile.endArray();

    filtersFile.beginWriteArray("Network");
    for (i = 0; i < networkFilters.size(); ++i)
    {
        filtersFile.setArrayIndex(i);
        filtersFile.setValue("filterName", networkFilters.at(i).filterName);
        filtersFile.setValue("filterCode", networkFilters.at(i).filterCode);
    }
    filtersFile.endArray();

    filtersFile.beginWriteArray("Transport");
    for (i = 0; i < transportFilters.size(); ++i)
    {
        filtersFile.setArrayIndex(i);
        filtersFile.setValue("filterName", transportFilters.at(i).filterName);
        filtersFile.setValue("filterCode", transportFilters.at(i).filterCode);
    }
    filtersFile.endArray();

    filtersFile.beginWriteArray("Application");
    for (i = 0; i < applicationFilters.size(); ++i)
    {
        filtersFile.setArrayIndex(i);
        filtersFile.setValue("filterName", applicationFilters.at(i).filterName);
        filtersFile.setValue("filterCode", applicationFilters.at(i).filterCode);
    }
    filtersFile.endArray();

    filtersFile.beginWriteArray("Other");
    for (i = 0; i < otherFilters.size(); ++i)
    {
        filtersFile.setArrayIndex(i);
        filtersFile.setValue("filterName", otherFilters.at(i).filterName);
        filtersFile.setValue("filterCode", otherFilters.at(i).filterCode);
    }
    filtersFile.endArray();
}

void FiltersDialog::showFilters()
{
    int i;

    ui.treeWidgetFilters->clear();

    ui.treeWidgetFilters->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetFilters, QStringList() << tr("Link layer")));
    ui.treeWidgetFilters->topLevelItem(0)->setIcon(0, *topIcon);
    for (i = 0; i < linkFilters.size(); ++i)
    {
        ui.treeWidgetFilters->topLevelItem(0)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << linkFilters.at(i).filterName));
        ui.treeWidgetFilters->topLevelItem(0)->child(i)->setIcon(0, *itemIcon);
    }

    ui.treeWidgetFilters->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetFilters, QStringList() << tr("Network layer")));
    ui.treeWidgetFilters->topLevelItem(1)->setIcon(0, *topIcon);
    for (i = 0; i < networkFilters.size(); ++i)
    {
        ui.treeWidgetFilters->topLevelItem(1)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << networkFilters.at(i).filterName));
        ui.treeWidgetFilters->topLevelItem(1)->child(i)->setIcon(0, *itemIcon);
    }

    ui.treeWidgetFilters->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetFilters, QStringList() << tr("Transport layer")));
    ui.treeWidgetFilters->topLevelItem(2)->setIcon(0, *topIcon);
    for (i = 0; i < transportFilters.size(); ++i)
    {
        ui.treeWidgetFilters->topLevelItem(2)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << transportFilters.at(i).filterName));
        ui.treeWidgetFilters->topLevelItem(2)->child(i)->setIcon(0, *itemIcon);
    }

    ui.treeWidgetFilters->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetFilters, QStringList() << tr("Application layer")));
    ui.treeWidgetFilters->topLevelItem(3)->setIcon(0, *topIcon);
    for (i = 0; i < applicationFilters.size(); ++i)
    {
        ui.treeWidgetFilters->topLevelItem(3)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << applicationFilters.at(i).filterName));
        ui.treeWidgetFilters->topLevelItem(3)->child(i)->setIcon(0, *itemIcon);
    }

    ui.treeWidgetFilters->addTopLevelItem(new QTreeWidgetItem(ui.treeWidgetFilters, QStringList() << tr("Other")));
    ui.treeWidgetFilters->topLevelItem(4)->setIcon(0, *topIcon);
    for (i = 0; i < otherFilters.size(); ++i)
    {
        ui.treeWidgetFilters->topLevelItem(4)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << otherFilters.at(i).filterName));
        ui.treeWidgetFilters->topLevelItem(4)->child(i)->setIcon(0, *itemIcon);
    }
}

void FiltersDialog::setFilter()
{
    int i;

    for (i = 0; i < linkFilters.size(); ++i)
        if (linkFilters.at(i).filterName == filterName)
        {
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(0)->child(i));
            return;
        }

    for (i = 0; i < networkFilters.size(); ++i)
        if (networkFilters.at(i).filterName == filterName)
        {
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(1)->child(i));
            return;
        }

    for (i = 0; i < transportFilters.size(); ++i)
        if (transportFilters.at(i).filterName == filterName)
        {
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(2)->child(i));
            return;
        }

    for (i = 0; i < applicationFilters.size(); ++i)
        if (applicationFilters.at(i).filterName == filterName)
        {
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(3)->child(i));
            return;
        }

    for (i = 0; i < otherFilters.size(); ++i)
        if (otherFilters.at(i).filterName == filterName)
        {
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(4)->child(i));
            return;
        }
}

void FiltersDialog::loadExamples()
{
    Filter filter;

    // link

    linkFilters.clear();

    filter.filterName = tr("All packets");
    filter.filterCode = "";
    linkFilters.append(filter);

    filter.filterName = tr("Broadcast");
    filter.filterCode = "ether broadcast";
    linkFilters.append(filter);

    filter.filterName = tr("Multicast");
    filter.filterCode = "ether multicast";
    linkFilters.append(filter);

    filter.filterName = tr("No Broadcast and no Multicast");
    filter.filterCode = "not broadcast and not multicast";
    linkFilters.append(filter);

    filter.filterName = tr("ARP");
    filter.filterCode = "arp";
    linkFilters.append(filter);

    filter.filterName = tr("RARP");
    filter.filterCode = "rarp";
    linkFilters.append(filter);

    filter.filterName = tr("No ARP and no RARP");
    filter.filterCode = "not arp and not rarp";
    linkFilters.append(filter);

    filter.filterName = tr("Host with address 00:00:00:00:00:00");
    filter.filterCode = "ether host 00:00:00:00:00:00";
    linkFilters.append(filter);

    filter.filterName = tr("Packets >= 65 < 128 bytes");
    filter.filterCode = "greater 64 and less 128";
    linkFilters.append(filter);

    // network

    networkFilters.clear();

    filter.filterName = tr("IPv4");
    filter.filterCode = "ip";
    networkFilters.append(filter);

    filter.filterName = tr("IPv6");
    filter.filterCode = "ip6";
    networkFilters.append(filter);

    filter.filterName = tr("Tunnelled IPv6 in IPv4");
    filter.filterCode = "ip and ip[9]==41";
    networkFilters.append(filter);

    filter.filterName = tr("IPv4 Broadcast");
    filter.filterCode = "ip broadcast";
    networkFilters.append(filter);

    filter.filterName = tr("IPv4 Multicast");
    filter.filterCode = "ip multicast";
    networkFilters.append(filter);

    filter.filterName = tr("IPv6 Multicast");
    filter.filterCode = "ip6 multicast";
    networkFilters.append(filter);

    filter.filterName = tr("Packets with IPv4 source 192.168.1.1");
    filter.filterCode = "src host 192.168.1.1";
    networkFilters.append(filter);

    filter.filterName = tr("Packets with IPv4 destination 192.168.1.1");
    filter.filterCode = "dst host 192.168.1.1";
    networkFilters.append(filter);

    filter.filterName = tr("Packets with IPv4 source or destination 192.168.1.1");
    filter.filterCode = "src or dst host 192.168.1.1";
    networkFilters.append(filter);

    filter.filterName = tr("ICMP");
    filter.filterCode = "icmp";
    networkFilters.append(filter);

    filter.filterName = tr("IGMP");
    filter.filterCode = "igmp";
    networkFilters.append(filter);

    // transport

    transportFilters.clear();

    filter.filterName = tr("TCP");
    filter.filterCode = "tcp";
    transportFilters.append(filter);

    filter.filterName = tr("UDP");
    filter.filterCode = "udp";
    transportFilters.append(filter);

    filter.filterName = tr("No TCP");
    filter.filterCode = "no tcp";
    transportFilters.append(filter);

    filter.filterName = tr("No UDP");
    filter.filterCode = "no udp";
    transportFilters.append(filter);

    // application

    applicationFilters.clear();

    filter.filterName = tr("HTTP [port 80]");
    filter.filterCode = "port 80";
    applicationFilters.append(filter);

    filter.filterName = tr("FTP [port 20 or 21]");
    filter.filterCode = "port 20 or port 21";
    applicationFilters.append(filter);

    filter.filterName = tr("SMTP or POP3 [port 25 or 110]");
    filter.filterCode = "port 25 or port 110";
    applicationFilters.append(filter);

    // other

    otherFilters.clear();
}

void FiltersDialog::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current->parent())
    {
        ui.pushButtonDelete->setEnabled(true);
        ui.pushButtonUpdate->setEnabled(true);

        int childIndex = current->parent()->indexOfChild(current);
        QString parentName = current->parent()->text(0);

        if (parentName == tr("Link layer"))
        {
            ui.lineEditName->setText(linkFilters.at(childIndex).filterName);
            ui.lineEditCode->setText(linkFilters.at(childIndex).filterCode);
            return;
        }

        if (parentName == tr("Network layer"))
        {
            ui.lineEditName->setText(networkFilters.at(childIndex).filterName);
            ui.lineEditCode->setText(networkFilters.at(childIndex).filterCode);
            return;
        }

        if (parentName == tr("Transport layer"))
        {
            ui.lineEditName->setText(transportFilters.at(childIndex).filterName);
            ui.lineEditCode->setText(transportFilters.at(childIndex).filterCode);
            return;
        }

        if (parentName == tr("Application layer"))
        {
            ui.lineEditName->setText(applicationFilters.at(childIndex).filterName);
            ui.lineEditCode->setText(applicationFilters.at(childIndex).filterCode);
            return;
        }

        if (parentName == tr("Other"))
        {
            ui.lineEditName->setText(otherFilters.at(childIndex).filterName);
            ui.lineEditCode->setText(otherFilters.at(childIndex).filterCode);
            return;
        }
    }
    else
    {
        ui.pushButtonDelete->setDisabled(true);
        ui.pushButtonUpdate->setDisabled(true);

        ui.lineEditName->setText(tr("All packets"));
        ui.lineEditCode->setText("");
    }
}

void FiltersDialog::onItemExpanded(QTreeWidgetItem *item)
{
    item->setIcon(0, *topOpenIcon);
}

void FiltersDialog::onItemCollapsed(QTreeWidgetItem *item)
{
    item->setIcon(0, *topIcon);
}

void FiltersDialog::onUpdate()
{
    QTreeWidgetItem *currentItem = ui.treeWidgetFilters->currentItem();

    if(!currentItem)
        return;

    if (currentItem->parent())
    {
        int childIndex = currentItem->parent()->indexOfChild(currentItem);
        QString parentName = currentItem->parent()->text(0);

        if (parentName == tr("Link layer"))
        {
            linkFilters[childIndex].filterName = ui.lineEditName->text().simplified();
            linkFilters[childIndex].filterCode = ui.lineEditCode->text().simplified();
            ui.treeWidgetFilters->topLevelItem(0)->takeChild(childIndex);
            ui.treeWidgetFilters->topLevelItem(0)->insertChild(childIndex, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << linkFilters.at(childIndex).filterName));
            ui.treeWidgetFilters->topLevelItem(0)->child(childIndex)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(0)->child(childIndex));
            goto end;
        }

        if (parentName == tr("Network layer"))
        {
            networkFilters[childIndex].filterName = ui.lineEditName->text().simplified();
            networkFilters[childIndex].filterCode = ui.lineEditCode->text().simplified();
            ui.treeWidgetFilters->topLevelItem(1)->takeChild(childIndex);
            ui.treeWidgetFilters->topLevelItem(1)->insertChild(childIndex, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << networkFilters.at(childIndex).filterName));
            ui.treeWidgetFilters->topLevelItem(1)->child(childIndex)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(1)->child(childIndex));
            goto end;
        }

        if (parentName == tr("Transport layer"))
        {
            transportFilters[childIndex].filterName = ui.lineEditName->text().simplified();
            transportFilters[childIndex].filterCode = ui.lineEditCode->text().simplified();
            ui.treeWidgetFilters->topLevelItem(2)->takeChild(childIndex);
            ui.treeWidgetFilters->topLevelItem(2)->insertChild(childIndex, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << transportFilters.at(childIndex).filterName));
            ui.treeWidgetFilters->topLevelItem(2)->child(childIndex)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(2)->child(childIndex));
            goto end;
        }

        if (parentName == tr("Application layer"))
        {
            applicationFilters[childIndex].filterName = ui.lineEditName->text().simplified();
            applicationFilters[childIndex].filterCode = ui.lineEditCode->text().simplified();
            ui.treeWidgetFilters->topLevelItem(3)->takeChild(childIndex);
            ui.treeWidgetFilters->topLevelItem(3)->insertChild(childIndex, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << applicationFilters.at(childIndex).filterName));
            ui.treeWidgetFilters->topLevelItem(3)->child(childIndex)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(3)->child(childIndex));
            goto end;
        }

        if (parentName == tr("Other"))
        {
            otherFilters[childIndex].filterName = ui.lineEditName->text().simplified();
            otherFilters[childIndex].filterCode = ui.lineEditCode->text().simplified();
            ui.treeWidgetFilters->topLevelItem(4)->takeChild(childIndex);
            ui.treeWidgetFilters->topLevelItem(4)->insertChild(childIndex, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << otherFilters.at(childIndex).filterName));
            ui.treeWidgetFilters->topLevelItem(4)->child(childIndex)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(4)->child(childIndex));
            goto end;
        }

        end:
        //writeFilters();
        return;
    }
}

void FiltersDialog::onAddNew()
{
    QTreeWidgetItem *currentItem = ui.treeWidgetFilters->currentItem();

    if (!currentItem)
        return;

    Filter filter;

    if (currentItem->parent())
    {
        QString parentName = currentItem->parent()->text(0);
        int childIndex = currentItem->parent()->indexOfChild(currentItem);

        if (parentName == tr("Link layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            linkFilters.insert(childIndex+1, filter);
            ui.treeWidgetFilters->topLevelItem(0)->insertChild(childIndex+1, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << linkFilters.at(childIndex+1).filterName));
            ui.treeWidgetFilters->topLevelItem(0)->child(childIndex+1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(0)->child(childIndex+1));
            goto end;
        }

        if (parentName == tr("Network layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            networkFilters.insert(childIndex+1, filter);
            ui.treeWidgetFilters->topLevelItem(1)->insertChild(childIndex+1, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << networkFilters.at(childIndex+1).filterName));
            ui.treeWidgetFilters->topLevelItem(1)->child(childIndex+1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(1)->child(childIndex+1));
            goto end;
        }

        if (parentName == tr("Transport layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            transportFilters.insert(childIndex+1, filter);
            ui.treeWidgetFilters->topLevelItem(2)->insertChild(childIndex+1, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << transportFilters.at(childIndex+1).filterName));
            ui.treeWidgetFilters->topLevelItem(2)->child(childIndex+1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(2)->child(childIndex+1));
            goto end;
        }

        if (parentName == tr("Application layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            applicationFilters.insert(childIndex+1, filter);
            ui.treeWidgetFilters->topLevelItem(3)->insertChild(childIndex+1, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << applicationFilters.at(childIndex+1).filterName));
            ui.treeWidgetFilters->topLevelItem(3)->child(childIndex+1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(3)->child(childIndex+1));
            goto end;
        }

        if (parentName == tr("Other"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            otherFilters.insert(childIndex+1, filter);
            ui.treeWidgetFilters->topLevelItem(4)->insertChild(childIndex+1, new QTreeWidgetItem((QTreeWidget*)0, QStringList() << otherFilters.at(childIndex+1).filterName));
            ui.treeWidgetFilters->topLevelItem(4)->child(childIndex+1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(4)->child(childIndex+1));
            goto end;
        }
    }
    else
    {
        int childs;
        QString itemName = currentItem->text(0);

        if (itemName == tr("Link layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            linkFilters.append(filter);
            ui.treeWidgetFilters->topLevelItem(0)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << linkFilters.at(linkFilters.size()-1).filterName));
            childs = currentItem->childCount();
            ui.treeWidgetFilters->topLevelItem(0)->child(childs-1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(0)->child(childs-1));
            goto end;
        }

        if (itemName == tr("Network layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            networkFilters.append(filter);
            ui.treeWidgetFilters->topLevelItem(1)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << networkFilters.at(networkFilters.size()-1).filterName));
            childs = currentItem->childCount();
            ui.treeWidgetFilters->topLevelItem(1)->child(childs-1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(1)->child(childs-1));
            goto end;
        }

        if (itemName == tr("Transport layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            transportFilters.append(filter);
            ui.treeWidgetFilters->topLevelItem(2)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << transportFilters.at(transportFilters.size()-1).filterName));
            childs = currentItem->childCount();
            ui.treeWidgetFilters->topLevelItem(2)->child(childs-1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(2)->child(childs-1));
            goto end;
        }

        if (itemName == tr("Application layer"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            applicationFilters.append(filter);
            ui.treeWidgetFilters->topLevelItem(3)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << applicationFilters.at(applicationFilters.size()-1).filterName));
            childs = currentItem->childCount();
            ui.treeWidgetFilters->topLevelItem(3)->child(childs-1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(3)->child(childs-1));
            goto end;
        }

        if (itemName == tr("Other"))
        {
            filter.filterName = ui.lineEditName->text().simplified();
            filter.filterCode = ui.lineEditCode->text().simplified();
            otherFilters.append(filter);
            ui.treeWidgetFilters->topLevelItem(4)->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << otherFilters.at(otherFilters.size()-1).filterName));
            childs = currentItem->childCount();
            ui.treeWidgetFilters->topLevelItem(4)->child(childs-1)->setIcon(0, *itemIcon);
            ui.treeWidgetFilters->setCurrentItem(ui.treeWidgetFilters->topLevelItem(4)->child(childs-1));
            goto end;
        }
    }

    end:
    //writeFilters();
    return;
}

void FiltersDialog::onDelete()
{
    QTreeWidgetItem *currentItem = ui.treeWidgetFilters->currentItem();

    if (!currentItem)
        return;

    if (currentItem->parent())
    {
        int childIndex = currentItem->parent()->indexOfChild(currentItem);
        QString parentName = currentItem->parent()->text(0);

        if (parentName == tr("Link layer"))
        {
            ui.treeWidgetFilters->topLevelItem(0)->takeChild(childIndex);
            linkFilters.removeAt(childIndex);

            if (ui.treeWidgetFilters->topLevelItem(0)->childCount() == 0)
                ui.treeWidgetFilters->topLevelItem(0)->setIcon(0, *topIcon);

            goto end;
        }

        if (parentName == tr("Network layer"))
        {
            ui.treeWidgetFilters->topLevelItem(1)->takeChild(childIndex);
            networkFilters.removeAt(childIndex);

            if (ui.treeWidgetFilters->topLevelItem(1)->childCount() == 0)
                ui.treeWidgetFilters->topLevelItem(1)->setIcon(0, *topIcon);

            goto end;
        }

        if (parentName == tr("Transport layer"))
        {
            ui.treeWidgetFilters->topLevelItem(2)->takeChild(childIndex);
            transportFilters.removeAt(childIndex);

            if (ui.treeWidgetFilters->topLevelItem(2)->childCount() == 0)
                ui.treeWidgetFilters->topLevelItem(2)->setIcon(0, *topIcon);

            goto end;
        }

        if (parentName == tr("Application layer"))
        {
            ui.treeWidgetFilters->topLevelItem(3)->takeChild(childIndex);
            applicationFilters.removeAt(childIndex);

            if (ui.treeWidgetFilters->topLevelItem(3)->childCount() == 0)
                ui.treeWidgetFilters->topLevelItem(3)->setIcon(0, *topIcon);

            goto end;
        }

        if (parentName == tr("Other"))
        {
            ui.treeWidgetFilters->topLevelItem(4)->takeChild(childIndex);
            otherFilters.removeAt(childIndex);

            if (ui.treeWidgetFilters->topLevelItem(4)->childCount() == 0)
                ui.treeWidgetFilters->topLevelItem(4)->setIcon(0, *topIcon);

            goto end;
        }

        end:
        //writeFilters();
        return;
    }
}

void FiltersDialog::onOpenEditorName()
{
    EditorDialog dlg(this, ui.lineEditName->text().simplified());

    if (dlg.exec())
    {
        ui.lineEditName->setText(dlg.getText().simplified());
        //ui.lineEditName->setCursorPosition(0);
    }

    ui.lineEditName->setFocus();
}

void FiltersDialog::onOpenEditorCode()
{
    EditorDialog dlg(this, ui.lineEditCode->text().simplified());

    if (dlg.exec())
    {
        ui.lineEditCode->setText(dlg.getText().simplified());
        //ui.lineEditCode->setCursorPosition(0);
    }

    ui.lineEditCode->setFocus();
}

void FiltersDialog::onDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent())
        onOK();
}

void FiltersDialog::onLoadExamples()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Question"),tr("Are you sure you want to remove all filters and load example filters?"), 0, this);
    msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(msgBox.addButton(tr("&No"), QMessageBox::RejectRole));

    if (msgBox.exec() == QMessageBox::AcceptRole)
    {
        loadExamples();

        disconnect(ui.treeWidgetFilters, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
        showFilters();
        ui.lineEditName->clear();
        ui.lineEditCode->clear();
        connect(ui.treeWidgetFilters, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
        setFilter();
    }
}

void FiltersDialog::onOK()
{
    onUpdate();
    writeFilters();

    QTreeWidgetItem *currentItem = ui.treeWidgetFilters->currentItem();

    if (currentItem)
    {
        if (currentItem->parent())
        {
            int childIndex = currentItem->parent()->indexOfChild(currentItem);
            QString parentName = currentItem->parent()->text(0);

            if (parentName == tr("Link layer"))
            {
                filterName = linkFilters.at(childIndex).filterName;
                filterCode = linkFilters.at(childIndex).filterCode;
            }

            if (parentName == tr("Network layer"))
            {
                filterName = networkFilters.at(childIndex).filterName;
                filterCode = networkFilters.at(childIndex).filterCode;
            }

            if (parentName == tr("Transport layer"))
            {
                filterName = transportFilters.at(childIndex).filterName;
                filterCode = transportFilters.at(childIndex).filterCode;
            }

            if (parentName == tr("Application layer"))
            {
                filterName = applicationFilters.at(childIndex).filterName;
                filterCode = applicationFilters.at(childIndex).filterCode;
            }

            if (parentName == tr("Other"))
            {
                filterName = otherFilters.at(childIndex).filterName;
                filterCode = otherFilters.at(childIndex).filterCode;
            }
        }
        else
        {
            filterName = tr("All packets");
            filterCode = "";
        }
    }
    else
    {
        filterName = tr("All packets");
        filterCode = "";
    }

    accept();
    close();
}

void FiltersDialog::onShowShortHelp()
{
    QToolTip::showText(mapToGlobal(ui.pushButtonShortHelp->pos()), ui.pushButtonShortHelp->toolTip());
}
