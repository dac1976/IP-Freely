// This file is part of IpFreely application.
//
// Copyright (C) 2018, Duncan Crutchley
// Contact <dac1976github@outlook.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License and GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// and GNU Lesser General Public License along with this program. If
// not, see <http://www.gnu.org/licenses/>.

/*!
 * \file IpFreelyAbout.cpp
 * \brief File containing definition of IpFreelyAbout dialog.
 */
#include "IpFreelyAbout.h"
#include "ui_IpFreelyAbout.h"

IpFreelyAbout::IpFreelyAbout(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IpFreelyAbout)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags();
    flags                 = flags & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);

    ui->tabWidget->setCurrentIndex(0);
}

IpFreelyAbout::~IpFreelyAbout()
{
    delete ui;
}

void IpFreelyAbout::SetTitle(const QString& title)
{
    ui->titleLabel->setText(title);
}
