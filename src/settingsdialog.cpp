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

#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    settings = new Settings();
    applySettings();

    ui.stackedWidget->setCurrentIndex(0);
    ui.listWidget->setCurrentRow(0);

    connect(ui.listWidget, SIGNAL(currentRowChanged(int)), ui.stackedWidget, SLOT(setCurrentIndex(int)));

    connect(ui.pushButtonRestore, SIGNAL(clicked()), this, SLOT(onRestoreDefaults()));

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));

    connect(ui.pushButtonExport, SIGNAL(clicked()), this, SLOT(onExport()));
    connect(ui.pushButtonImport, SIGNAL(clicked()), this, SLOT(onImport()));

    connect(ui.pushButtonLogFolder, SIGNAL(clicked()), this, SLOT(onLogFolderBrowse()));

    connect(ui.comboBoxScreenshotsFormat, SIGNAL(currentIndexChanged(QString)), this, SLOT(onFormatChanged(QString)));
    connect(ui.comboBoxScreenshotsExtension, SIGNAL(currentIndexChanged(int)), this, SLOT(onExtensionChanged(int)));
    connect(ui.pushButtonScreenshotsFolder, SIGNAL(clicked()), this, SLOT(onScreenshotsFolderBrowse()));
    connect(ui.lineEditScreenshotsName, SIGNAL(textChanged(QString)), this, SLOT(onFileNameChanged(QString)));

    connect(ui.comboBoxLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(languageChanged(int)));
}

void SettingsDialog::setLanguage()
{
    languageFiles = findLanguageFiles();

    // comboBoxLanguage always contain "English (Default)" item
    languageFiles.prepend("English (Default)");
    languageNames.prepend("English (Default)");
    languageAuthorName.prepend("Mariusz Helfajer");
    languageAuthorWWW.prepend(makeLink("http://helfajer.info"));
    languageAuthorEmail.prepend(makeLink("none"));
    languageAuthorNote.prepend("This is default application language");

    for (int i = 1; i < languageNames.size(); ++i)
        ui.comboBoxLanguage->addItem(languageNames.at(i));

    int index = ui.comboBoxLanguage->findText(languageName(settings->mainWindow.languageFile));

    if ((languageNames.count() > 1) && (index != -1))
    {
        ui.comboBoxLanguage->setCurrentIndex(index);
        languageChanged(index);
    }
    else
    {
        ui.comboBoxLanguage->setCurrentIndex(0);
        languageChanged(0);
    }
}

QStringList SettingsDialog::findLanguageFiles()
{
    QDir dir(QCoreApplication::applicationDirPath() + "/languages/");
    QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);

    QMutableStringListIterator i (fileNames);
    while (i.hasNext())
    {
        QString val = i.next();

        if (val.contains("qt_"))
        {
            // remove files with "qt_", that are qt libraries translation, not necessary for our application
            i.remove();
        }
        else
        {
            QString name = languageName(val);

            if (name.isEmpty())
            {
                // remove files that have not been correct translated or corrupt
                i.remove();
            }
            else
            {
                // add translated language name to list
                languageNames.append(name);

                QTranslator translator;
                translator.load(QCoreApplication::applicationDirPath()+"/languages/"+val);

                //: translator's name
                languageAuthorName.append(translator.translate("SettingsDialogClass", "Mariusz Helfajer"));

                //: translator's www page, use: "http://" or "www."
                languageAuthorWWW.append(makeLink(translator.translate("SettingsDialogClass", "http://helfajer.info")));

                //: translator's e-mail, use a@b.c otherwise nothing will be displayed
                languageAuthorEmail.append(makeLink(translator.translate("SettingsDialogClass", "none")));

                //: translator's notes
                languageAuthorNote.append(translator.translate("SettingsDialogClass", "This is default application language"));
            }
        }
    }

    return fileNames;
}

QString SettingsDialog::languageName(const QString &qmFile)
{
    QTranslator translator;
    translator.load(QCoreApplication::applicationDirPath()+"/languages/"+qmFile);

    //: language name in translator language, e.g. Polski, Deutsch
    return translator.translate("SettingsDialogClass", "English (Default)");
}

void SettingsDialog::languageChanged(int index)
{
    ui.labelAuthorName->setText(languageAuthorName.at(index));
    ui.labelAuthorWWW->setText(languageAuthorWWW.at(index));
    ui.labelAuthorEmail->setText(languageAuthorEmail.at(index));
    ui.labelAuthorNote->setText(languageAuthorNote.at(index));
}

QString SettingsDialog::makeLink(const QString &text)
{
    if (text.contains("@"))
        // e-mail
        return QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                       "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> p, li { white-space: pre-wrap; } </style></head>"
                       "<body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">"
                       "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "<a href=\"mailto:%1\"><span style=\" text-decoration: underline; color:#0000ff;\">%2</span></a></p></body></html>").arg(text).arg(text);

    if (text.startsWith("http://", Qt::CaseInsensitive) || text.startsWith("www.", Qt::CaseInsensitive))
        // www
        return QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                       "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> p, li { white-space: pre-wrap; } </style></head>"
                       "<body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">"
                       "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "<a href=\"%1\"><span style=\" text-decoration: underline; color:#0000ff;\">%2</span></a></p></body></html>").arg(text).arg(text);

    return QString("");
}

void SettingsDialog::onRestoreDefaults()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Question"),tr("Are you sure you want to reset all settings to the default?"), 0, this);
    msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(msgBox.addButton(tr("&No"), QMessageBox::RejectRole));

    if (msgBox.exec() == QMessageBox::AcceptRole)
    {
        //
        ui.checkBoxMaximized->setChecked(false);
        ui.checkBoxMinimizeToTrayOnStart->setChecked(false);
        ui.checkBoxMinimizeToTrayOnClose->setChecked(false);

        //
        ui.groupBoxLogFile->setChecked(true);
        ui.lineEditLogFolder->setText(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
        ui.lineEditLogFolder->setCursorPosition(0);

        //
        ui.checkBoxDevicesInfo->setChecked(false);
        ui.checkBoxIPv4->setChecked(true);
        ui.checkBoxIPv6->setChecked(true);

        //
        ui.checkBoxAutoStart->setChecked(false);
        ui.groupBoxCapture->setChecked(false);
        ui.checkBoxMode->setChecked(true);
        ui.spinBoxBytesOfPacket->setValue(65535);
        ui.spinBoxTimeout->setValue(1000);

        //
        ui.spinBoxUp->setValue(0);
        ui.spinBoxDown->setValue(0);

        //
        ui.checkBoxScreenshotsFileDlg->setChecked(true);

        int formatIndex = ui.comboBoxScreenshotsFormat->findText("Portable Network Graphics (*.png)", Qt::MatchExactly);
        if (formatIndex < 0) formatIndex = 0;
        ui.comboBoxScreenshotsFormat->setCurrentIndex(formatIndex);
        onFormatChanged(ui.comboBoxScreenshotsFormat->currentText());

        int extensionIndex = ui.comboBoxScreenshotsExtension->findText("png", Qt::MatchExactly);
        if (extensionIndex < 0) extensionIndex = 0;
        ui.comboBoxScreenshotsExtension->setCurrentIndex(extensionIndex);
        onExtensionChanged(ui.comboBoxScreenshotsExtension->currentIndex());

        ui.spinBoxScreenshotsQuality->setValue(-1);
        ui.lineEditScreenshotsFolder->setText(QDir::toNativeSeparators(QDir::homePath()));
        ui.lineEditScreenshotsFolder->setCursorPosition(0);
        ui.lineEditScreenshotsName->setText("%D_%T_%N");
        ui.lineEditScreenshotsName->setCursorPosition(0);
        onFileNameChanged(ui.lineEditScreenshotsName->text());

        //
        ui.checkBoxAutoCheckUpdate->setChecked(false);

        //
        ui.comboBoxLanguage->setCurrentIndex(0);
    }
}

void SettingsDialog::onOK()
{
    if (!writeSettings())
        return;

    accept();
    close();
}

void SettingsDialog::onSave()
{
    writeSettings();
}

bool SettingsDialog::checkItems()
{
    if (ui.groupBoxLogFile->isChecked())
    {
        if (ui.lineEditLogFolder->text().isEmpty())
        {
            ui.listWidget->setCurrentRow(1);
            QMessageBox::information(this, tr("Information"), tr("Please select events viewer log file folder."));
            return false;
        }

        QDir logDir(ui.lineEditLogFolder->text());
        if (!logDir.exists())
        {
            ui.listWidget->setCurrentRow(1);
            QMessageBox::critical(this, tr("Critical"), tr("The selected events viewer log file folder does not exist."));
            return false;
        }
        if (!logDir.isReadable())
        {
            ui.listWidget->setCurrentRow(1);
            QMessageBox::critical(this, tr("Critical"), tr("The selected events viewer log file folder is not readable."));
            return false;
        }
    }

    if (ui.lineEditScreenshotsFolder->text().isEmpty())
    {
        ui.listWidget->setCurrentRow(5);
        QMessageBox::information(this, tr("Information"), tr("Please select screenshots destination folder."));
        return false;
    }

    QDir screenshotsDir(ui.lineEditScreenshotsFolder->text());
    if (!screenshotsDir.exists())
    {
        ui.listWidget->setCurrentRow(5);
        QMessageBox::critical(this, tr("Critical"), tr("The selected screenshots folder does not exist."));
        return false;
    }
    if (!screenshotsDir.isReadable())
    {
        ui.listWidget->setCurrentRow(5);
        QMessageBox::critical(this, tr("Critical"), tr("The selected screenshots folder is not readable."));
        return false;
    }

    if (ui.lineEditScreenshotsName->text().isEmpty())
    {
        ui.listWidget->setCurrentRow(5);
        QMessageBox::information(this, tr("Information"), tr("Please enter default screenshots file name."));
        return false;
    }

    QString text = ui.lineEditScreenshotsName->text();
    text.remove("\\");
    text.remove("/");
    text.remove(":");
    text.remove("*");
    text.remove("?");
    text.remove("\"");
    text.remove("<");
    text.remove(">");
    text.remove("|");
    if (text.isEmpty())
    {
        ui.listWidget->setCurrentRow(5);
        QMessageBox::information(this, tr("Information"), tr("Please enter correct default screenshots file name.\n"
                                                             "Don't use any of the following characters: \\ / : * ? \" < > |"));
        return false;
    }
    else
    {
        ui.lineEditScreenshotsName->setText(text);
        ui.lineEditScreenshotsName->setCursorPosition(0);
    }

    return true;
}

void SettingsDialog::applySettings()
{
    //
    ui.checkBoxMaximized->setChecked(settings->mainWindow.startMaximized);
    ui.checkBoxMinimizeToTrayOnStart->setChecked(settings->mainWindow.minimizeToTrayOnStart);
    ui.checkBoxMinimizeToTrayOnClose->setChecked(settings->mainWindow.minimizeToTrayOnClose);

    //
    ui.groupBoxLogFile->setChecked(settings->eventsViewerMainWindow.createFile);
    ui.lineEditLogFolder->setText(settings->eventsViewerMainWindow.folder);
    ui.lineEditLogFolder->setCursorPosition(0);

    //
    ui.checkBoxDevicesInfo->setChecked(settings->devicesDialog.showDevicesInfo);
    ui.checkBoxIPv4->setChecked(settings->devicesDialog.showIPv4);
    ui.checkBoxIPv6->setChecked(settings->devicesDialog.showIPv6);

    //
    ui.checkBoxAutoStart->setChecked(settings->mainWindow.autoStart);
    ui.groupBoxCapture->setChecked(false);
    ui.checkBoxMode->setChecked(settings->captureThread.mode);
    ui.spinBoxBytesOfPacket->setValue(settings->captureThread.bytes);
    ui.spinBoxTimeout->setValue(settings->captureThread.timeout);

    //
    ui.spinBoxUp->setValue(settings->netTransferDialog.up);
    ui.spinBoxDown->setValue(settings->netTransferDialog.down);

    //
    ui.checkBoxScreenshotsFileDlg->setChecked(settings->screenshots.dialog);

    int formatIndex = ui.comboBoxScreenshotsFormat->findText(settings->screenshots.format, Qt::MatchExactly);
    if (formatIndex < 0) formatIndex = 0;
    ui.comboBoxScreenshotsFormat->setCurrentIndex(formatIndex);
    onFormatChanged(ui.comboBoxScreenshotsFormat->currentText());

    int extensionIndex = ui.comboBoxScreenshotsExtension->findText(settings->screenshots.extension, Qt::MatchExactly);
    if (extensionIndex < 0) extensionIndex = 0;
    ui.comboBoxScreenshotsExtension->setCurrentIndex(extensionIndex);
    onExtensionChanged(ui.comboBoxScreenshotsExtension->currentIndex());

    ui.spinBoxScreenshotsQuality->setValue(settings->screenshots.quality);
    ui.lineEditScreenshotsFolder->setText(settings->screenshots.folder);
    ui.lineEditScreenshotsFolder->setCursorPosition(0);
    ui.lineEditScreenshotsName->setText(settings->screenshots.name);
    ui.lineEditScreenshotsName->setCursorPosition(0);
    onFileNameChanged(ui.lineEditScreenshotsName->text());

    //
    ui.checkBoxAutoCheckUpdate->setChecked(settings->mainWindow.autoCheckUpdate);

    //
    setLanguage();
}

bool SettingsDialog::writeSettings()
{
    if (!checkItems())
        return false;

    //
    settings->mainWindow.startMaximized = ui.checkBoxMaximized->isChecked();
    settings->mainWindow.minimizeToTrayOnStart = ui.checkBoxMinimizeToTrayOnStart->isChecked();
    settings->mainWindow.minimizeToTrayOnClose = ui.checkBoxMinimizeToTrayOnClose->isChecked();

    //
    settings->eventsViewerMainWindow.createFile = ui.groupBoxLogFile->isChecked();
    settings->eventsViewerMainWindow.folder = ui.lineEditLogFolder->text();

    //
    settings->devicesDialog.showDevicesInfo = ui.checkBoxDevicesInfo->isChecked();
    settings->devicesDialog.showIPv4 = ui.checkBoxIPv4->isChecked();
    settings->devicesDialog.showIPv6 = ui.checkBoxIPv6->isChecked();

    //
    settings->mainWindow.autoStart = ui.checkBoxAutoStart->isChecked();
    settings->captureThread.mode = ui.checkBoxMode->isChecked();
    settings->captureThread.bytes = ui.spinBoxBytesOfPacket->value();
    settings->captureThread.timeout = ui.spinBoxTimeout->value();

    //
    settings->netTransferDialog.up = ui.spinBoxUp->value();
    settings->netTransferDialog.down = ui.spinBoxDown->value();

    //
    settings->screenshots.dialog = ui.checkBoxScreenshotsFileDlg->isChecked();
    settings->screenshots.format = ui.comboBoxScreenshotsFormat->currentText();
    settings->screenshots.extension = ui.comboBoxScreenshotsExtension->currentText();
    settings->screenshots.quality = ui.spinBoxScreenshotsQuality->value();
    settings->screenshots.folder = ui.lineEditScreenshotsFolder->text();
    settings->screenshots.name = ui.lineEditScreenshotsName->text();

    //
    settings->mainWindow.autoCheckUpdate = ui.checkBoxAutoCheckUpdate->isChecked();

    //
    settings->mainWindow.languageFile = languageFiles.at(ui.comboBoxLanguage->currentIndex());

    // write settings to file
    settings->write();

    switch (settings->getError())
    {
        case 0: return true;
        case 1: QMessageBox::warning(this, tr("Warning"), tr("An access error occurred (trying to write the file). Settings not saved."));
                return false;
        default: return false;
    }

    return true;
}

void SettingsDialog::onLogFolderBrowse()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    QString directory = QFileDialog::getExistingDirectory(this, tr("Select folder"), ui.lineEditLogFolder->text(), options);

    if (!directory.isEmpty())
    {
        ui.lineEditLogFolder->setText(QDir::toNativeSeparators(directory));
        ui.lineEditLogFolder->setCursorPosition(0);
    }
}

void SettingsDialog::onExport()
{
    if (!writeSettings())
        return;

    QFile settingsFile(QCoreApplication::applicationDirPath() + "/settings.ini");

    if (!settingsFile.exists())
    {
        QMessageBox::critical(this, tr("Critical"), tr("Can't find the settings.ini file!"));
        return;
    }

    QString filter = tr("INI files (*.ini)");
    QString newFileName = QFileDialog::getSaveFileName(this, tr("Export settings"), QString(QDir::homePath() + "/settings"), tr("INI files (*.ini);;All files (*.*)"), &filter);

    if (newFileName.isEmpty())
        return;

    QFile newFile(newFileName);

    if (newFile.exists())
        newFile.remove();

    if (!settingsFile.copy(newFileName))
        QMessageBox::critical(this, tr("Critical"), tr("File not exported."));
}

void SettingsDialog::onImport()
{
    QString filter = tr("INI files (*.ini)");
    QString newFileName = QFileDialog::getOpenFileName(this, tr("Import settings"), QString(QDir::homePath() + "/settings"), tr("INI files (*.ini);;All files (*.*)"), &filter);

    if (newFileName.isEmpty())
        return;

    QFile newFile(newFileName);
    QFile settingsFile(QCoreApplication::applicationDirPath() + "/settings.ini");

    if (settingsFile.exists())
        settingsFile.remove();

    if (!newFile.copy(QCoreApplication::applicationDirPath()+"/settings.ini"))
    {
        QMessageBox::critical(this, tr("Critical"), tr("File not imported."));
        return;
    }

    settings->read();
    switch (settings->getError())
    {
        case 0: break;
        case 1: QMessageBox::warning(this, tr("Warning"), tr("Settings file does not exist. Default settings loaded."));
                break;
        case 2: QMessageBox::warning(this, tr("Warning"), tr("An access error occurred (trying to read the file). Default settings loaded."));
                break;
        case 3: QMessageBox::warning(this, tr("Warning"), tr("A format error occurred (a malformed INI file) while reading the file. Some settings reset to defaults."));
                break;
        default: break;
    }

    applySettings();
}

void SettingsDialog::onScreenshotsFolderBrowse()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    QString directory = QFileDialog::getExistingDirectory(this, tr("Select folder"), ui.lineEditScreenshotsFolder->text(), options);

    if (!directory.isEmpty())
    {
        ui.lineEditScreenshotsFolder->setText(QDir::toNativeSeparators(directory));
        ui.lineEditScreenshotsFolder->setCursorPosition(0);
    }
}

void SettingsDialog::onFormatChanged(QString text)
{
    ui.comboBoxScreenshotsExtension->clear();

    QString str;
    int index, n, i = 0;

    while ((index = text.indexOf("*.", i)) != -1)
    {
        n = text.indexOf(" ", index) - index - 2;

        str = text.mid(index + 2, n);

        if (str.endsWith(")"))
            str.chop(1);

        ui.comboBoxScreenshotsExtension->addItem(str);

        i = index + n;
    }

    onFileNameChanged(ui.lineEditScreenshotsName->text());
}

void SettingsDialog::onExtensionChanged(int index)
{
    onFileNameChanged(ui.lineEditScreenshotsName->text());
}

void SettingsDialog::onFileNameChanged(QString text)
{
    QString graphName = tr("User transfers");
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh.mm.ss");
    QString extension = ui.comboBoxScreenshotsExtension->currentText();
    QString str = "";

    text.remove("\\");
    text.remove("/");
    text.remove(":");
    text.remove("*");
    text.remove("?");
    text.remove("\"");
    text.remove("<");
    text.remove(">");
    text.remove("|");

    int i = 0;

    while (i < text.length())
    {
        if ((text.at(i) == '%') && (i+1 < text.length()))
        {
            if (text.at(i+1) == 'N') { str.append(graphName); i+=2; continue; }
            if (text.at(i+1) == 'D') { str.append(date); i+=2; continue; }
            if (text.at(i+1) == 'T') { str.append(time); i+=2; continue; }

            str.append(text.at(i));
            str.append(text.at(i+1));

            i+=2;
        }
        else
        {
            str.append(text.at(i));

            ++i;
        }
    }

    str.append("." + extension);

    ui.lineEditScreenshotsPreview->setText(str);
    ui.lineEditScreenshotsPreview->setCursorPosition(0);
}
