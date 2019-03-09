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

#include "usertransfersgraphdialog.h"

UserTransfersGraphDialog::UserTransfersGraphDialog(QWidget *parent, ReceiverCore *receiverCore)
    : QDialog(parent)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint);

    ui.setupUi(this);

    qRegisterMetaType<qrealList>("QList<qreal>");

    connect(receiverCore, SIGNAL(signalNewUser(QString,QString)), this, SLOT(newUser(QString,QString)));
    connect(receiverCore, SIGNAL(signalUsersSpeed(QList<qreal>,QList<qreal>)), this, SLOT(setValue(QList<qreal>,QList<qreal>)), Qt::QueuedConnection);

    ui.widget->setXLabel(tr("Time (seconds)"));
    ui.widget->setYLabel(tr("Transfer (KB/s)"));

    clearData();

    time = 0;

    timerHour = new QTimer(this);
    connect(timerHour, SIGNAL(timeout()), this, SLOT(updateTimerHour()));

    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(ui.pushButtonRestore, SIGNAL(clicked()), this, SLOT(onRestoreDefaults()));

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui.comboBoxUsers, SIGNAL(currentIndexChanged(int)), this, SLOT(onUserChanged(int)));
    connect(ui.comboBoxTime, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimeChanged(int)));
    connect(ui.comboBoxShow, SIGNAL(currentIndexChanged(int)), this, SLOT(onShowChanged(int)));

    connect(ui.checkBoxH, SIGNAL(toggled(bool)), this, SLOT(toggledHLines(bool)));
    connect(ui.checkBoxV, SIGNAL(toggled(bool)), this, SLOT(toggledVLines(bool)));
    connect(ui.checkBoxBackground, SIGNAL(toggled(bool)), this, SLOT(toggledBackground(bool)));
    connect(ui.checkBoxFilled, SIGNAL(toggled(bool)), this, SLOT(toggledFilled(bool)));
    connect(ui.checkBoxAntialiasing, SIGNAL(toggled(bool)), this, SLOT(toggledAntialiasing(bool)));
    connect(ui.checkBoxTooltip, SIGNAL(toggled(bool)), this, SLOT(toggledTooltip(bool)));

    ui.comboBoxTime->setCurrentIndex(Settings::userTransfersGraphDialog.time);
    ui.comboBoxShow->setCurrentIndex(Settings::userTransfersGraphDialog.show);

    ui.checkBoxH->setChecked(Settings::userTransfersGraphDialog.h);
    ui.checkBoxV->setChecked(Settings::userTransfersGraphDialog.v);
    ui.checkBoxBackground->setChecked(Settings::userTransfersGraphDialog.background);
    ui.checkBoxFilled->setChecked(Settings::userTransfersGraphDialog.filled);
    ui.checkBoxAntialiasing->setChecked(Settings::userTransfersGraphDialog.antialiasing);
    ui.checkBoxTooltip->setChecked(Settings::userTransfersGraphDialog.tooltip);
}

void UserTransfersGraphDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous())
    {
        // our application event (not system event)
        resize(Settings::userTransfersGraphDialog.size);
        move(Settings::userTransfersGraphDialog.position);
    }

    event->accept();
}

void UserTransfersGraphDialog::closeEvent(QCloseEvent *event)
{
    Settings::userTransfersGraphDialog.size = size();
    Settings::userTransfersGraphDialog.position = pos();

    event->accept();
}

void UserTransfersGraphDialog::writeSettings()
{
    Settings::userTransfersGraphDialog.size = size();
    Settings::userTransfersGraphDialog.position = pos();
}

void UserTransfersGraphDialog::startGraph()
{
    ui.widget->clearGraph();
    clearData();
    timerHour->start(60000);
}

void UserTransfersGraphDialog::stopGraph()
{
    timerHour->stop();
}

void UserTransfersGraphDialog::onRestoreDefaults()
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

void UserTransfersGraphDialog::onSave()
{
    QString graphName = ui.labelTitle->text();
    if (!ui.comboBoxUsers->currentText().isEmpty())
    {
        graphName.append("_");
        graphName.append(ui.comboBoxUsers->currentText());
    }

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

void UserTransfersGraphDialog::newUser(const QString &user, const QString &timeOn)
{
    TransferTab t;

    for (int i = 0; i < 61; ++i)
    {
        t.dataDown[i]=0;
        t.dataUp[i]=0;
    }

    dataMinuteList.append(t);
    dataHourList.append(t);

    ui.comboBoxUsers->addItem(user);
    ui.comboBoxUsers->setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void UserTransfersGraphDialog::setValue(QList<qreal> uesrsUpSpeed, QList<qreal> usersDownSpeed)
{
    for (int i = 0; i < dataMinuteList.count(); ++i)
    {
        for (int j = 60; j > 0; --j)
        {
            dataMinuteList[i].dataUp[j] = dataMinuteList[i].dataUp[j-1];
            dataMinuteList[i].dataDown[j] = dataMinuteList[i].dataDown[j-1];
        }

        dataMinuteList[i].dataUp[0] = int(uesrsUpSpeed.at(i));
        dataMinuteList[i].dataDown[0] = int(usersDownSpeed.at(i));
    }

    if (time == 0)
    {
        int i = ui.comboBoxUsers->currentIndex();
        if (i >= 0)
            ui.widget->setData(dataMinuteList.at(i).dataUp, dataMinuteList.at(i).dataDown);
    }
}

void UserTransfersGraphDialog::updateTimerHour()
{
    int totalUp, totalDown;

    for (int i = 0; i < dataMinuteList.count(); ++i)
    {
        totalUp = 0;
        totalDown = 0;

        for (int j = 0; j < 61; ++j)
        {
            totalUp+=dataMinuteList.at(i).dataUp[j];
            totalDown+=dataMinuteList.at(i).dataDown[j];
        }

        for (int j = 60; j > 0; --j)
        {
            dataHourList[i].dataUp[j] = dataHourList[i].dataUp[j-1];
            dataHourList[i].dataDown[j] = dataHourList[i].dataDown[j-1];
        }

        dataHourList[i].dataUp[0] = totalUp/61;
        dataHourList[i].dataDown[0] = totalDown/61;
    }

    if (time == 1)
    {
        int i = ui.comboBoxUsers->currentIndex();
        if (i >= 0)
            ui.widget->setData(dataHourList.at(i).dataUp, dataHourList.at(i).dataDown);
    }

    timerHour->start(60000);
}

void UserTransfersGraphDialog::clearData()
{
    dataMinuteList.clear();
    dataHourList.clear();

    ui.comboBoxUsers->clear();
}

void UserTransfersGraphDialog::onUserChanged(int i)
{
    if (i >= 0)
    {
        if (time == 0)
            ui.widget->setData(dataMinuteList.at(i).dataUp, dataMinuteList.at(i).dataDown);

        if (time == 1)
            ui.widget->setData(dataHourList.at(i).dataUp, dataHourList.at(i).dataDown);
    }
}

void UserTransfersGraphDialog::onTimeChanged(int val)
{
    int i = ui.comboBoxUsers->currentIndex();

    Settings::userTransfersGraphDialog.time = val;

    switch (val)
    {
        case 0: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (seconds)"));
                ui.widget->setYLabel(tr("Transfer (KB/s)"));
                if (i >= 0)
                    ui.widget->setData(dataMinuteList.at(i).dataUp, dataMinuteList.at(i).dataDown);
                time = 0;
                break;

        case 1: ui.widget->clearGraph();
                ui.widget->setXLabel(tr("Time (minutes)"));
                ui.widget->setYLabel(tr("Average transfer (KB/s)"));
                if (i >= 0)
                    ui.widget->setData(dataHourList.at(i).dataUp, dataHourList.at(i).dataDown);
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

void UserTransfersGraphDialog::onShowChanged(int val)
{
    Settings::userTransfersGraphDialog.show = val;

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

void UserTransfersGraphDialog::toggledHLines(bool checked)
{
    ui.widget->setHLines(checked);

    Settings::userTransfersGraphDialog.h = checked;
}

void UserTransfersGraphDialog::toggledVLines(bool checked)
{
    ui.widget->setVLines(checked);

    Settings::userTransfersGraphDialog.v = checked;
}

void UserTransfersGraphDialog::toggledBackground(bool checked)
{
    ui.widget->setBackground(checked);

    Settings::userTransfersGraphDialog.background = checked;
}

void UserTransfersGraphDialog::toggledFilled(bool checked)
{
    ui.widget->setFilled(checked);

    Settings::userTransfersGraphDialog.filled = checked;
}

void UserTransfersGraphDialog::toggledAntialiasing(bool checked)
{
    ui.widget->setAntialiasing(checked);

    Settings::userTransfersGraphDialog.antialiasing = checked;
}

void UserTransfersGraphDialog::toggledTooltip(bool checked)
{
    ui.widget->setMouseTracking(checked);
    ui.widget->setEnabled(checked);

    Settings::userTransfersGraphDialog.tooltip = checked;
}
