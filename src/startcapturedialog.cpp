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

#include "startcapturedialog.h"

StartCaptureDialog::StartCaptureDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    // probably Qt Designer error, must be here disabled otherwise it is not working
    ui.frameStart->setDisabled(true);
    ui.frameDuration->setDisabled(true);

    QTime time(QTime::currentTime());
    ui.timeEditAt->setTime(time);
    ui.timeEditTo->setTime(time);

    ui.checkBoxMode->setChecked(Settings::captureThread.mode == 1 ? true : false);
    ui.spinBoxBytesOfPacket->setValue(Settings::captureThread.bytes);
    ui.spinBoxTimeout->setValue(Settings::captureThread.timeout);
   
    createConnections();
}

void StartCaptureDialog::createConnections()
{
    connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(onStart()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui.radioButtonImmediately, SIGNAL(toggled(bool)), ui.frameStart, SLOT(setDisabled(bool)));
    connect(ui.radioButtonImmediately, SIGNAL(toggled(bool)), ui.timeEditAt, SLOT(setDisabled(bool)));

    connect(ui.radioButtonAt, SIGNAL(toggled(bool)), ui.timeEditAt, SLOT(setEnabled(bool)));
    connect(ui.radioButtonAt, SIGNAL(toggled(bool)), ui.frameStart, SLOT(setDisabled(bool)));

    connect(ui.radioButtonAfter, SIGNAL(toggled(bool)), ui.frameStart, SLOT(setEnabled(bool)));
    connect(ui.radioButtonAfter, SIGNAL(toggled(bool)), ui.timeEditAt, SLOT(setDisabled(bool)));

    connect(ui.radioButtonUser, SIGNAL(toggled(bool)), ui.spinBoxPackets, SLOT(setDisabled(bool)));
    connect(ui.radioButtonUser, SIGNAL(toggled(bool)), ui.frameDuration, SLOT(setDisabled(bool)));
    connect(ui.radioButtonUser, SIGNAL(toggled(bool)), ui.timeEditTo, SLOT(setDisabled(bool)));

    connect(ui.radioButtonTo, SIGNAL(toggled(bool)), ui.timeEditTo, SLOT(setEnabled(bool)));
    connect(ui.radioButtonTo, SIGNAL(toggled(bool)), ui.spinBoxPackets, SLOT(setDisabled(bool)));
    connect(ui.radioButtonTo, SIGNAL(toggled(bool)), ui.frameDuration, SLOT(setDisabled(bool)));

    connect(ui.radioButtonPackets, SIGNAL(toggled(bool)), ui.spinBoxPackets, SLOT(setEnabled(bool)));
    connect(ui.radioButtonPackets, SIGNAL(toggled(bool)), ui.frameDuration, SLOT(setDisabled(bool)));
    connect(ui.radioButtonPackets, SIGNAL(toggled(bool)), ui.timeEditTo, SLOT(setDisabled(bool)));

    connect(ui.radioButtonTime, SIGNAL(toggled(bool)), ui.frameDuration, SLOT(setEnabled(bool)));
    connect(ui.radioButtonTime, SIGNAL(toggled(bool)), ui.spinBoxPackets, SLOT(setDisabled(bool)));
    connect(ui.radioButtonTime, SIGNAL(toggled(bool)), ui.timeEditTo, SLOT(setDisabled(bool)));
}

void StartCaptureDialog::onStart()
{
    if (ui.radioButtonTime->isChecked() && ui.spinBoxH->value() == 0 && ui.spinBoxM->value() == 0 && ui.spinBoxS->value() == 0)
    {
        QMessageBox::information(this, tr("Information"), tr("The capture duration time must be at least 1 second."));
        return;
    }

    if (ui.groupBoxAdvanced->isChecked())
    {
        Settings::captureThread.mode = ui.checkBoxMode->isChecked() ? 1 : 0;
        Settings::captureThread.bytes = ui.spinBoxBytesOfPacket->value();
        Settings::captureThread.timeout = ui.spinBoxTimeout->value();
    }

    accept();
    close();
}

CaptureData StartCaptureDialog::getCaptureData()
{
    CaptureData data;

    if (ui.radioButtonUser->isChecked())
        data.durationChoice = 1;

    if(ui.radioButtonTo->isChecked())
    {
        data.durationChoice = 2;
        data.durationValue = ui.timeEditTo->time().hour() * 60 * 60 + ui.timeEditTo->time().minute() * 60 + ui.timeEditTo->time().second();
    }

    if (ui.radioButtonPackets->isChecked())
    {
        data.durationValue = ui.spinBoxPackets->value();
        data.durationChoice = 3;
    }

    if (ui.radioButtonTime->isChecked())
    {
        data.durationValue = ui.spinBoxH->value() * 60 * 60 + ui.spinBoxM->value() * 60 + ui.spinBoxS->value();
        data.durationChoice = 4;
    }

    if (ui.radioButtonImmediately->isChecked())
        data.startChoice = 1;

    if (ui.radioButtonAt->isChecked())
    {
        data.startValue = ui.timeEditAt->time().hour() * 60 * 60 + ui.timeEditAt->time().minute() * 60 + ui.timeEditAt->time().second();
        data.startChoice = 2;
    }

    if (ui.radioButtonAfter->isChecked())
    {
        data.startValue = ui.spinBoxAfterH->value() * 60 * 60 + ui.spinBoxAfterM->value() * 60 + ui.spinBoxAfterS->value();
        data.startChoice = 3;
    }

    return data;
}

void StartCaptureDialog::setCaptureData(CaptureData data)
{
    ushort h, m, s;

    switch (data.durationChoice)
    {
        case 1: ui.radioButtonUser->setChecked(true);
                break;
        case 2: ui.radioButtonTo->setChecked(true);
                break;
        case 3: ui.radioButtonPackets->setChecked(true);
                ui.spinBoxPackets->setValue(data.durationValue);
                break;
        case 4: ui.radioButtonTime->setChecked(true);
                h = data.durationValue / 60 / 60;
                ui.spinBoxH->setValue(h);
                m = (data.durationValue - (h * 60 * 60)) / 60;
                ui.spinBoxM->setValue(m);
                s = (data.durationValue - (h * 60 * 60) - (m * 60));
                ui.spinBoxS->setValue(s);
                break;
        default: break;
    }

    switch (data.startChoice)
    {
        case 1: ui.radioButtonImmediately->setChecked(true);
                break;
        case 2: ui.radioButtonAt->setChecked(true);
                break;
        case 3: ui.radioButtonAfter->setChecked(true);
                h = data.startValue / 60 / 60;
                ui.spinBoxAfterH->setValue(h);
                m = (data.startValue - (h * 60 * 60)) / 60;
                ui.spinBoxAfterM->setValue(m);
                s = (data.startValue - (h * 60 * 60) - (m * 60));
                ui.spinBoxAfterS->setValue(s);
                break;
        default: break;
    }
}
