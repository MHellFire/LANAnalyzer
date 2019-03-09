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

#include "aboutdialog.h"

#include "WpdPack/Include/pcap.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    ui.lineEditQt->setText(qVersion());
    ui.lineEditQt->setCursorPosition(0);
    ui.lineEditPcap->setText(pcap_lib_version());
    ui.lineEditPcap->setCursorPosition(0);

    connect(ui.pushButtonMore, SIGNAL(toggled(bool)), this, SLOT(onMoreToggled(bool)));
    connect(ui.pushButtonMore, SIGNAL(toggled(bool)), ui.widgetMore, SLOT(setVisible(bool)));

    ui.pushButtonMore->setIcon(QIcon(":/images/o_more_down.png"));
    ui.widgetMore->hide();

    connect(ui.pushButtonLicense, SIGNAL(clicked()), this, SLOT(onShowLicense()));
    connect(ui.pushButtonAboutQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));
}

void AboutDialog::onMoreToggled(bool checked)
{
    checked ? ui.pushButtonMore->setIcon(QIcon(":/images/o_more_up.png")) : ui.pushButtonMore->setIcon(QIcon(":/images/o_more_down.png"));
}

void AboutDialog::onShowLicense()
{
    LicenseDialog dlg(this);
    dlg.exec();
}
