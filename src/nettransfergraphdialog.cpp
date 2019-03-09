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

#include "nettransfergraphdialog.h"

NetTransferGraphDialog::NetTransferGraphDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint);

    ui.setupUi(this);

    connect(receiverCore, SIGNAL(signalNetSpeed(qreal,qreal)), this, SLOT(setValue(qreal,qreal)));

    ui.widget->setXLabel(tr("Time (seconds)"));
    ui.widget->setYLabel(tr("Transfer (KB/s)"));

    clearData();

    time = 0;

    timerHour = new QTimer(this);
    connect(timerHour, SIGNAL(timeout()), this, SLOT(updateTimerHour()));

    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ui.pushButtonRestore, SIGNAL(clicked()), this, SLOT(onRestoreDefaults()));

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui.comboBoxTime, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimeChanged(int)));
    connect(ui.comboBoxShow, SIGNAL(currentIndexChanged(int)), this, SLOT(onShowChanged(int)));

    connect(ui.checkBoxH, SIGNAL(toggled(bool)), this, SLOT(toggledHLines(bool)));
    connect(ui.checkBoxV, SIGNAL(toggled(bool)), this, SLOT(toggledVLines(bool)));
    connect(ui.checkBoxBackground, SIGNAL(toggled(bool)), this, SLOT(toggledBackground(bool)));
    connect(ui.checkBoxFilled, SIGNAL(toggled(bool)), this, SLOT(toggledFilled(bool)));
    connect(ui.checkBoxAntialiasing, SIGNAL(toggled(bool)), this, SLOT(toggledAntialiasing(bool)));
    connect(ui.checkBoxTooltip, SIGNAL(toggled(bool)), this, SLOT(toggledTooltip(bool)));

    ui.comboBoxTime->setCurrentIndex(Settings::netTransferGraphDialog.time);
    ui.comboBoxShow->setCurrentIndex(Settings::netTransferGraphDialog.show);

    ui.checkBoxH->setChecked(Settings::netTransferGraphDialog.h);
    ui.checkBoxV->setChecked(Settings::netTransferGraphDialog.v);
    ui.checkBoxBackground->setChecked(Settings::netTransferGraphDialog.background);
    ui.checkBoxFilled->setChecked(Settings::netTransferGraphDialog.filled);
    ui.checkBoxAntialiasing->setChecked(Settings::netTransferGraphDialog.antialiasing);
    ui.checkBoxTooltip->setChecked(Settings::netTransferGraphDialog.tooltip);
}

void NetTransferGraphDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
    {
        // our application event (not system event)
        resize(Settings::netTransferGraphDialog.size);
        move(Settings::netTransferGraphDialog.position);
    }

    event->accept();
}

void NetTransferGraphDialog::closeEvent(QCloseEvent *event)
{
    Settings::netTransferGraphDialog.size = size();
    Settings::netTransferGraphDialog.position = pos();

    event->accept();
}

void NetTransferGraphDialog::writeSettings()
{
    Settings::netTransferGraphDialog.size = size();
    Settings::netTransferGraphDialog.position = pos();
}

void NetTransferGraphDialog::startGraph()
{
    ui.widget->clearGraph();
    clearData();
    timerHour->start(60000);
}

void NetTransferGraphDialog::stopGraph()
{
    timerHour->stop();
}

void NetTransferGraphDialog::onRestoreDefaults()
{
    ui.comboBoxTime->setCurrentIndex(0);
    ui.comboBoxShow->setCurrentIndex(0);
    ui.checkBoxH->setChecked(true);
    ui.checkBoxV->setChecked(false);
    ui.checkBoxBackground->setChecked(true);
    ui.checkBoxFilled->setChecked(false);
    ui.checkBoxAntialiasing->setChecked(false);
    ui.checkBoxTooltip->setChecked(true);
}

void NetTransferGraphDialog::onSave()
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

void NetTransferGraphDialog::setValue(const qreal &upSpeed, const qreal &downSpeed)
{
    for (int i = 60; i > 0; --i)
    {
        dataUpMinute[i] = dataUpMinute[i-1];
        dataDownMinute[i] = dataDownMinute[i-1];
    }

    dataUpMinute[0] = int(upSpeed);
    dataDownMinute[0] = int(downSpeed);

    if (time == 0)
        ui.widget->setData(dataUpMinute, dataDownMinute);
}

void NetTransferGraphDialog::updateTimerHour()
{
    int totalUp = 0, totalDown = 0;

    for (int i = 0; i < 61; ++i)
    {
        totalUp+=dataUpMinute[i];
        totalDown+=dataDownMinute[i];
    }

    for (int i = 60; i > 0; --i)
    {
        dataUpHour[i] = dataUpHour[i-1];
        dataDownHour[i] = dataDownHour[i-1];
    }

    dataUpHour[0] = int (totalUp/61);
    dataDownHour[0] = int (totalDown/61);

    if (time == 1)
        ui.widget->setData(dataUpHour, dataDownHour);

    timerHour->start(60000);
}

void NetTransferGraphDialog::clearData()
{
    for (int i = 0; i < 61; ++i)
    {
        dataUpMinute[i] = 0;
        dataDownMinute[i] = 0;
        dataUpHour[i] = 0;
        dataDownHour[i] = 0;
    }
}

void NetTransferGraphDialog::onTimeChanged(int val)
{
    Settings::netTransferGraphDialog.time = val;

    switch (val)
    {
        case 0: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (seconds)"));
                ui.widget->setYLabel(tr("Transfer (KB/s)"));
                ui.widget->setData(dataUpMinute, dataDownMinute);
                time = 0;
                break;

        case 1: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (minutes)"));
                ui.widget->setYLabel(tr("Average transfer (KB/s)"));
                ui.widget->setData(dataUpHour, dataDownHour);
                time = 1;
                break;

        case 2: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (hours)"));
                ui.widget->setYLabel(tr("Average transfer (KB/s)"));
                time = 2;
                break;

        default: break;
    }
}

void NetTransferGraphDialog::onShowChanged(int val)
{
    Settings::netTransferGraphDialog.show = val;

    switch (val)
    {
        case 0: //ui.widget->clearGraph();
                ui.widget->setUp(true);
                ui.widget->setDown(true);
                ui.widget->repaint();
                break;

        case 1: //ui.widget->clearGraph();
                ui.widget->setUp(true);
                ui.widget->setDown(false);
                ui.widget->repaint();
                break;

        case 2: //ui.widget->clearGraph();
                ui.widget->setUp(false);
                ui.widget->setDown(true);
                ui.widget->repaint();
                break;

        default: break;
    }
}

void NetTransferGraphDialog::toggledHLines(bool checked)
{
    ui.widget->setHLines(checked);

    Settings::netTransferGraphDialog.h = checked;
}

void NetTransferGraphDialog::toggledVLines(bool checked)
{
    ui.widget->setVLines(checked);

    Settings::netTransferGraphDialog.v = checked;
}

void NetTransferGraphDialog::toggledBackground(bool checked)
{
    ui.widget->setBackground(checked);

    Settings::netTransferGraphDialog.background = checked;
}

void NetTransferGraphDialog::toggledFilled(bool checked)
{
    ui.widget->setFilled(checked);

    Settings::netTransferGraphDialog.filled = checked;
}

void NetTransferGraphDialog::toggledAntialiasing(bool checked)
{
    ui.widget->setAntialiasing(checked);

    Settings::netTransferGraphDialog.antialiasing = checked;
}

void NetTransferGraphDialog::toggledTooltip(bool checked)
{
    ui.widget->setMouseTracking(checked);
    ui.widget->setEnabled(checked);

    Settings::netTransferGraphDialog.tooltip = checked;
}
