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

#include "transparencydialog.h"

#include <math.h>

TransparencyDialog::TransparencyDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));

    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(close()));

    ui.horizontalSlider->setValue(int(ceil(parent->windowOpacity() * 100)));
}

void TransparencyDialog::onValueChanged(int value)
{
    ui.labelLevel->setText(tr("Transparency [%1%]").arg(100-value));

    emit valueChanged(value);
}
