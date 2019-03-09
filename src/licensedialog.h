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

#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include "ui_licensedialog.h"

class LicenseDialog : public QDialog
{
    Q_OBJECT

public:
    LicenseDialog(QWidget *parent = 0);

private:
    Ui::LicenseDialogClass ui;
};

#endif // LICENSEDIALOG_H
