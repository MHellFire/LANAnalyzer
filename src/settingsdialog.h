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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTranslator>
#include <QDateTime>

#include "settings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsDialog)

public:
    explicit SettingsDialog(QWidget *parent = 0);

private:
    Ui::SettingsDialogClass ui;

    Settings *settings;

    QStringList languageFiles;
    QStringList languageNames;
    QStringList languageAuthorName;
    QStringList languageAuthorWWW;
    QStringList languageAuthorEmail;
    QStringList languageAuthorNote;

    void setLanguage();
    QStringList findLanguageFiles();
    QString languageName(const QString &qmFile);

    QString makeLink(const QString &text);

    void applySettings();

    bool writeSettings();
    bool checkItems();

private slots:
    void onRestoreDefaults();
    void onOK();
    void onSave();

    void onLogFolderBrowse();

    void onExport();
    void onImport();

    void onScreenshotsFolderBrowse();
    void onFormatChanged(QString text);
    void onExtensionChanged(int index);
    void onFileNameChanged(QString text);

    void languageChanged(int index);
};

#endif // SETTINGSDIALOG_H
