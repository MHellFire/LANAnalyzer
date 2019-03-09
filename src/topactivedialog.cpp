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

#include "topactivedialog.h"

TopActiveDialog::TopActiveDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.pushButtonOK, SIGNAL(clicked()), this, SLOT(close()));
}

void TopActiveDialog::setFirstItem(const QString &name, const QString &bytes)
{
    ui.labelName->setText(name);
    ui.labelBytes->setText(bytes);
}

void TopActiveDialog::insertItem(int i, const QString &name, const QString &strBytes, quint64 itemBytes, quint64 totalBytes)
{
    QLabel *label = new QLabel(strBytes);
    label->setAlignment(Qt::AlignHCenter);

    ui.gridLayout->addWidget(new QLabel(name), i+2, 0);
    ui.gridLayout->addWidget(label, i+2, 2);

    qreal b = (itemBytes * 100.0) / totalBytes;

    QProgressBar *bar = new QProgressBar();
    bar->setMinimumWidth(200);
    bar->setMaximumHeight(15);
    bar->setTextVisible(false);
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->setValue(int(b));
    ui.gridLayout->addWidget(bar, i+2, 4);

    QLabel *l = new QLabel(QString("%1%").arg(b, 0, 'f', 2));
    l->setAlignment(Qt::AlignRight);
    ui.gridLayout->addWidget(l, i+2, 6);
}
