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

#include "exportdatadialog.h"

ExportDataDialog::ExportDataDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    ui.checkBoxUsers->setChecked(Settings::exportDataDialog.users);
    ui.checkBoxPackets->setChecked(Settings::exportDataDialog.packets);
    ui.checkBoxTransfers->setChecked(Settings::exportDataDialog.transfers);
    ui.checkBoxApplications->setChecked(Settings::exportDataDialog.applications);
    ui.checkBoxHosts->setChecked(Settings::exportDataDialog.hosts);

    ui.comboBoxFields->setCurrentIndex(Settings::exportDataDialog.fields);
    ui.comboBoxLines->setCurrentIndex(Settings::exportDataDialog.lines);

    ui.checkBoxHeader->setChecked(Settings::exportDataDialog.header);

    ui.checkBoxSummary->setChecked(Settings::exportDataDialog.summary);

    ui.lineEditFolder->setText(Settings::exportDataDialog.folder);
    ui.lineEditFolder->setCursorPosition(0);

    ui.checkBoxOpenAfter->setChecked(Settings::exportDataDialog.openAfter);

    connect(ui.pushButtonAll, SIGNAL(clicked()), this, SLOT(onSelectAll()));
    connect(ui.pushButtonNone, SIGNAL(clicked()), this, SLOT(onSelectNone()));

    connect(ui.pushButtonBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));

    connect(ui.pushButtonRestore, SIGNAL(clicked()), this, SLOT(onRestore()));

    connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(onExport()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void ExportDataDialog::onSelectAll()
{
    ui.checkBoxUsers->setChecked(true);
    ui.checkBoxPackets->setChecked(true);
    ui.checkBoxTransfers->setChecked(true);
    ui.checkBoxApplications->setChecked(true);
    ui.checkBoxHosts->setChecked(true);
}

void ExportDataDialog::onSelectNone()
{
    ui.checkBoxUsers->setChecked(false);
    ui.checkBoxPackets->setChecked(false);
    ui.checkBoxTransfers->setChecked(false);
    ui.checkBoxApplications->setChecked(false);
    ui.checkBoxHosts->setChecked(false);
}

void ExportDataDialog::onBrowse()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    QString directory = QFileDialog::getExistingDirectory(this, tr("Select folder"), ui.lineEditFolder->text(), options);

    if (!directory.isEmpty())
    {
        ui.lineEditFolder->setText(QDir::toNativeSeparators(directory));
        ui.lineEditFolder->setCursorPosition(0);
    }
}

void ExportDataDialog::onRestore()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Question"),tr("Are you sure you want to reset settings to the default?"), 0, this);
    msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(msgBox.addButton(tr("&No"), QMessageBox::RejectRole));

    if (msgBox.exec() == QMessageBox::AcceptRole)
    {
        ui.checkBoxUsers->setChecked(true);
        ui.checkBoxPackets->setChecked(true);
        ui.checkBoxTransfers->setChecked(true);
        ui.checkBoxApplications->setChecked(true);
        ui.checkBoxHosts->setChecked(true);

        ui.comboBoxFields->setCurrentIndex(0);
        ui.comboBoxLines->setCurrentIndex(0);
        ui.checkBoxHeader->setChecked(false);

        ui.checkBoxSummary->setChecked(true);

        ui.lineEditFolder->setText(QDir::toNativeSeparators(QDir::homePath()));
        ui.lineEditFolder->setCursorPosition(0);

        ui.checkBoxOpenAfter->setChecked(false);
    }
}

void ExportDataDialog::onExport()
{
    if (ui.checkBoxUsers->isChecked() || ui.checkBoxPackets->isChecked() || ui.checkBoxTransfers->isChecked() || ui.checkBoxApplications->isChecked() || ui.checkBoxHosts->isChecked())
    {
        if (ui.lineEditFolder->text().isEmpty())
        {
            QMessageBox::information(this, tr("Information"), tr("Please select correct destination folder."));
            return;
        }

        QDir dir(ui.lineEditFolder->text());

        if (!dir.exists())
        {
            QMessageBox::critical(this, tr("Critical"), tr("The selected folder does not exist."));
            return;
        }

        if (!dir.isReadable())
        {
            QMessageBox::critical(this, tr("Critical"), tr("The selected folder is not readable."));
            return;
        }

        Settings::exportDataDialog.users = ui.checkBoxUsers->isChecked();
        Settings::exportDataDialog.packets = ui.checkBoxPackets->isChecked();
        Settings::exportDataDialog.transfers = ui.checkBoxTransfers->isChecked();
        Settings::exportDataDialog.applications = ui.checkBoxApplications->isChecked();
        Settings::exportDataDialog.hosts = ui.checkBoxHosts->isChecked();

        Settings::exportDataDialog.fields = ui.comboBoxFields->currentIndex();
        Settings::exportDataDialog.lines = ui.comboBoxLines->currentIndex();

        Settings::exportDataDialog.header = ui.checkBoxHeader->isChecked();

        Settings::exportDataDialog.folder = ui.lineEditFolder->text();

        Settings::exportDataDialog.openAfter = ui.checkBoxOpenAfter->isChecked();

        Settings::exportDataDialog.summary = ui.checkBoxSummary->isChecked();

        accept();
        close();
    }
    else
    {
        if (ui.lineEditFolder->text().isEmpty())
        {
            QMessageBox::information(this, tr("Information"), tr("Please select correct destination folder and at least one page for data export."));
            return;
        }

        QDir dir(ui.lineEditFolder->text());

        if (!dir.exists())
        {
            QMessageBox::critical(this, tr("Critical"), tr("The selected folder does not exist and please select at least one page for data export."));
            return;
        }

        if (!dir.isReadable())
        {
            QMessageBox::critical(this, tr("Critical"), tr("The selected folder is not readable and please select at least one page for data export."));
            return;
        }

        QMessageBox::information(this, tr("Information"), tr("Please select at least one page for data export."));
        return;
    }    
}
