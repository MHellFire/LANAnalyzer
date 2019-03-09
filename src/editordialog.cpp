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

#include "editordialog.h"

EditorDialog::EditorDialog(QWidget *parent, const QString &text)
    : QDialog(parent)
{
    QDialog::setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui.setupUi(this);

    ui.plainTextEdit->setPlainText(text);
    ui.plainTextEdit->moveCursor(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);

    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(onOK()));
}

QString EditorDialog::getText() const
{
    return ui.plainTextEdit->toPlainText();
}

void EditorDialog::onOK()
{
    accept();
    close();
}
