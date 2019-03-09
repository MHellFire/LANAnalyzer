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

#include "netpacketsgraphdialog.h"

NetPacketsGraphDialog::NetPacketsGraphDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint);

    ui.setupUi(this);

    connect(receiverCore, SIGNAL(signalNetPacketsSpeed(quint16)), this, SLOT(setValue(quint16)));

    ui.widget->setXLabel(tr("Time (seconds)"));
    ui.widget->setYLabel(tr("Number of packets"));

    clearData();

    time = 0;

    timerHour = new QTimer(this);
    connect(timerHour, SIGNAL(timeout()), this, SLOT(updateTimerHour()));

    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ui.pushButtonRestore, SIGNAL(clicked()), this, SLOT(onRestoreDefaults()));

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui.comboBoxTime, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimeChanged(int)));
    connect(ui.checkBoxH, SIGNAL(toggled(bool)), this, SLOT(toggledHLines(bool)));
    connect(ui.checkBoxV, SIGNAL(toggled(bool)), this, SLOT(toggledVLines(bool)));
    connect(ui.checkBoxBackground, SIGNAL(toggled(bool)), this, SLOT(toggledBackground(bool)));
    connect(ui.checkBoxFilled, SIGNAL(toggled(bool)), this, SLOT(toggledFilled(bool)));
    connect(ui.checkBoxAntialiasing, SIGNAL(toggled(bool)), this, SLOT(toggledAntialiasing(bool)));
    connect(ui.checkBoxTooltip, SIGNAL(toggled(bool)), this, SLOT(toggledTooltip(bool)));

    ui.comboBoxTime->setCurrentIndex(Settings::netPacketsGraphDialog.time);

    ui.checkBoxH->setChecked(Settings::netPacketsGraphDialog.h);
    ui.checkBoxV->setChecked(Settings::netPacketsGraphDialog.v);
    ui.checkBoxBackground->setChecked(Settings::netPacketsGraphDialog.background);
    ui.checkBoxFilled->setChecked(Settings::netPacketsGraphDialog.filled);
    ui.checkBoxAntialiasing->setChecked(Settings::netPacketsGraphDialog.antialiasing);
    ui.checkBoxTooltip->setChecked(Settings::netPacketsGraphDialog.tooltip);
}

void NetPacketsGraphDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
    {
        // our application event (not system event)
        resize(Settings::netPacketsGraphDialog.size);
        move(Settings::netPacketsGraphDialog.position);
    }

    event->accept();
}

void NetPacketsGraphDialog::closeEvent(QCloseEvent *event)
{
    Settings::netPacketsGraphDialog.size = size();
    Settings::netPacketsGraphDialog.position = pos();

    event->accept();
}

void NetPacketsGraphDialog::writeSettings()
{
    Settings::netPacketsGraphDialog.size = size();
    Settings::netPacketsGraphDialog.position = pos();
}

void NetPacketsGraphDialog::startGraph()
{
    ui.widget->clearGraph();
    clearData();
    timerHour->start(60000);
}

void NetPacketsGraphDialog::stopGraph()
{
    timerHour->stop();
}

void NetPacketsGraphDialog::onRestoreDefaults()
{    
    ui.comboBoxTime->setCurrentIndex(0);
    ui.checkBoxH->setChecked(true);
    ui.checkBoxV->setChecked(false);
    ui.checkBoxBackground->setChecked(true);
    ui.checkBoxFilled->setChecked(false);
    ui.checkBoxAntialiasing->setChecked(false);
    ui.checkBoxTooltip->setChecked(true);
}

void NetPacketsGraphDialog::onSave()
{
    QString graphName = ui.labelTitle->text();
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh.mm.ss");
    QString fileName = "";

    QString fileCode = Settings::screenshots.name;
    int i = 0;

    while (i < fileCode.length())
    {
        if ((fileCode.at(i) == '%') && (i+1 < fileCode.length()))
        {
            if (fileCode.at(i+1) == 'N') { fileName.append(graphName); i+=2; continue; }
            if (fileCode.at(i+1) == 'D') { fileName.append(date); i+=2; continue; }
            if (fileCode.at(i+1) == 'T') { fileName.append(time); i+=2; continue; }

            fileName.append(fileCode.at(i));
            fileName.append(fileCode.at(i+1));
            i+=2;
        }
        else
        {
            fileName.append(fileCode.at(i));
            ++i;
        }
    }

    QString path = Settings::screenshots.folder;
    QString extension = Settings::screenshots.extension;
    int quality = Settings::screenshots.quality;

    QPainter painter;

    if (Settings::screenshots.dialog)
    {
        QString types = tr("Joint Photographic Experts Group (*.jpeg *.jpg);;"
                           "Portable Network Graphics (*.png);;"
                           "Scalable Vector Graphics (*.svg);;"
                           "Tagged Image File Format (*.tiff *.tif);;"
                           "Windows Bitmap (*.bmp);;");
        QString filter = Settings::screenshots.format;

        QString newFileName = QFileDialog::getSaveFileName(this, tr("Save as"), path + "/" + fileName, types, &filter);

        if (!newFileName.isEmpty())
        {
            QFileInfo fi(newFileName);

            if (fi.suffix().contains("svg", Qt::CaseInsensitive))
            {
                QSvgGenerator generator;
                generator.setFileName(newFileName);
                generator.setSize(ui.widget->size());
                generator.setViewBox(QRect(0, 0, ui.widget->width(), ui.widget->height()));
                generator.setTitle(graphName);
                //generator.setDescription(tr(""));
                painter.begin(&generator);
                ui.widget->paint(painter);
                painter.end();
            }
            else
            {
                QImage image(ui.widget->size(), QImage::Format_RGB32);
                image.invertPixels(QImage::InvertRgb);
                painter.begin(&image);
                ui.widget->paint(painter);
                painter.end();

                if (!image.save(newFileName, fi.suffix().toAscii().constData(), quality))
                {
                    QMessageBox::critical(this, tr("Critical"), QString(tr("Unable to save file: %1")).arg(newFileName));
                }
            }
        }
    }
    else
    {
        if (extension.contains("svg", Qt::CaseInsensitive))
        {
            QSvgGenerator generator;
            generator.setFileName(path+"/"+fileName+".svg");
            generator.setSize(ui.widget->size());
            generator.setViewBox(QRect(0, 0, ui.widget->width(), ui.widget->height()));
            generator.setTitle(graphName);
            //generator.setDescription(tr(""));
            painter.begin(&generator);
            ui.widget->paint(painter);
            painter.end();
        }
        else
        {
            QImage image(ui.widget->size(), QImage::Format_RGB32);
            image.invertPixels(QImage::InvertRgb);
            painter.begin(&image);
            ui.widget->paint(painter);
            painter.end();

            if (!image.save(path+"/"+fileName+"."+extension, extension.toAscii().constData(), quality))
            {
                QMessageBox::critical(this, tr("Critical"), QString(tr("Unable to save file: %1")).arg(path+"/"+fileName+"."+extension));
            }
        }
    }
}

void NetPacketsGraphDialog::setValue(quint16 packetsSpeed)
{
    for (int i = 60; i > 0; --i)
        dataMinute[i] = dataMinute[i-1];

    dataMinute[0] = packetsSpeed;

    if (time == 0)
        ui.widget->setData(dataMinute);
}

void NetPacketsGraphDialog::updateTimerHour()
{
    int total = 0;

    for (int i = 0; i < 61; ++i)
        total+=dataMinute[i];

    for (int i = 60; i > 0; --i)
        dataHour[i] = dataHour[i-1];

    dataHour[0] = int (total/61);

    if (time == 1)
        ui.widget->setData(dataHour);

    timerHour->start(60000);
}

void NetPacketsGraphDialog::clearData()
{
    for (int i = 0; i < 61; ++i)
    {
        dataMinute[i] = 0;
        dataHour[i] = 0;
    }
}

void NetPacketsGraphDialog::onTimeChanged(int val)
{
    Settings::netPacketsGraphDialog.time = val;

    switch (val)
    {
        case 0: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (seconds)"));
                ui.widget->setYLabel(tr("Number of packets"));
                ui.widget->setData(dataMinute);
                time = 0;
                break;

        case 1: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (minutes)"));
                ui.widget->setYLabel(tr("Average packets/second"));
                ui.widget->setData(dataHour);
                time = 1;
                break;

        case 2: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (hours)"));
                ui.widget->setYLabel(tr("Average packets/second"));
                time = 2;
                break;

        default: break;
    }
}

void NetPacketsGraphDialog::toggledHLines(bool checked)
{
    ui.widget->setHLines(checked);

    Settings::netPacketsGraphDialog.h = checked;
}

void NetPacketsGraphDialog::toggledVLines(bool checked)
{
    ui.widget->setVLines(checked);

    Settings::netPacketsGraphDialog.v = checked;
}

void NetPacketsGraphDialog::toggledBackground(bool checked)
{
    ui.widget->setBackground(checked);

    Settings::netPacketsGraphDialog.background = checked;
}

void NetPacketsGraphDialog::toggledFilled(bool checked)
{
    ui.widget->setFilled(checked);

    Settings::netPacketsGraphDialog.filled = checked;
}

void NetPacketsGraphDialog::toggledAntialiasing(bool checked)
{
    ui.widget->setAntialiasing(checked);

    Settings::netPacketsGraphDialog.antialiasing = checked;
}

void NetPacketsGraphDialog::toggledTooltip(bool checked)
{
    ui.widget->setMouseTracking(checked);
    ui.widget->setEnabled(checked);

    Settings::netPacketsGraphDialog.tooltip = checked;
}
