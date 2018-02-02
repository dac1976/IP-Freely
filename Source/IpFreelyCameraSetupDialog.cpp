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
 * \file IpFreelyCameraSetupDialog.cpp
 * \brief File containing definition of the IP camera setup dialog.
 */
#include "IpFreelyCameraSetupDialog.h"
#include "ui_IpFreelyCameraSetupDialog.h"
#include "IpFreelyCameraDatabase.h"

IpFreelyCameraSetupDialog::IpFreelyCameraSetupDialog(ipfreely::IpCamera& camera, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IpFreelyCameraSetupDialog)
    , m_camera(camera)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags();
    flags                 = flags & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);

    InitialiseCameraSettings(m_camera);
}

IpFreelyCameraSetupDialog::~IpFreelyCameraSetupDialog()
{
    delete ui;
}

void IpFreelyCameraSetupDialog::on_buttonBox_accepted()
{
    m_camera.rtspUrl                  = ui->rtspUrlLineEdit->text().toStdString();
    m_camera.storageHttpUrl           = ui->storageUrlLineEdit->text().toStdString();
    m_camera.username                 = ui->usernameLineEdit->text().toStdString();
    m_camera.password                 = ui->passwordLineEdit->text().toStdString();
    m_camera.description              = ui->descriptionLineEdit->text().toStdString();
    m_camera.enableScheduledRecording = ui->scheduledRecordingCheckBox->checkState() == Qt::Checked;

    accept();
}

void IpFreelyCameraSetupDialog::on_buttonBox_rejected()
{
    reject();
}

void IpFreelyCameraSetupDialog::on_clearSettingsPushButton_clicked()
{
    ipfreely::IpCamera blankCamera;
    InitialiseCameraSettings(blankCamera);
}

void IpFreelyCameraSetupDialog::on_revertChangesPushButton_clicked()
{
    InitialiseCameraSettings(m_camera);
}

void IpFreelyCameraSetupDialog::InitialiseCameraSettings(ipfreely::IpCamera const& camera)
{
    ui->rtspUrlLineEdit->setText(QString::fromStdString(camera.rtspUrl));
    ui->storageUrlLineEdit->setText(QString::fromStdString(camera.storageHttpUrl));
    ui->usernameLineEdit->setText(QString::fromStdString(camera.username));
    ui->passwordLineEdit->setText(QString::fromStdString(camera.password));
    ui->descriptionLineEdit->setText(QString::fromStdString(camera.description));
    ui->scheduledRecordingCheckBox->setCheckState(camera.enableScheduledRecording ? Qt::Checked
                                                                                  : Qt::Unchecked);
}
