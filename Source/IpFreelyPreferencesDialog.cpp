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
 * \file IpFreelyPreferencesDialog.cpp
 * \brief File containing definition of IpFreelyPreferencesDialog form.
 */
#include "IpFreelyPreferencesDialog.h"
#include "ui_IpFreelyPreferencesDialog.h"
#include <QFileDialog>
#include "IpFreelyPreferences.h"

IpFreelyPreferencesDialog::IpFreelyPreferencesDialog(ipfreely::IpFreelyPreferences& prefs,
                                                     QWidget*                       parent)
    : QDialog(parent)
    , ui(new Ui::IpFreelyPreferencesDialog)
    , m_prefs(prefs)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags();
    flags                 = flags & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);

    ui->tabWidget->setCurrentIndex(0);
    ui->saveFolderPathLineEdit->setText(QString::fromStdString(m_prefs.SaveFolderPath()));
    ui->fileDurationDoubleSpinBox->setValue(m_prefs.FileDurationInSecs());
    ui->connectOnStartupCheckBox->setChecked(m_prefs.ConnectToCamerasOnStartup());

    InitialisSchedule();
}

IpFreelyPreferencesDialog::~IpFreelyPreferencesDialog()
{
    delete ui;
}

void IpFreelyPreferencesDialog::on_buttonBox_accepted()
{
    m_prefs.SetSaveFolderPath(ui->saveFolderPathLineEdit->text().toStdString());
    m_prefs.SetFileDurationInSecs(ui->fileDurationDoubleSpinBox->value());
    m_prefs.SetConnectToCamerasOnStartup(ui->connectOnStartupCheckBox->isChecked());

    auto schedule = m_prefs.RecordingSchedule();

    for (int row = 0; row < ui->scheduleTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->scheduleTableWidget->columnCount(); ++col)
        {
            schedule[static_cast<size_t>(row)][static_cast<size_t>(col)] =
                ui->scheduleTableWidget->item(row, col)->checkState() == Qt::Checked;
        }
    }

    m_prefs.SetRecordingSchedule(schedule);

    m_prefs.Save();
    accept();
}

void IpFreelyPreferencesDialog::on_buttonBox_rejected()
{
    reject();
}

void IpFreelyPreferencesDialog::on_saveFolderPathToolButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select parent save folder..."),
        QString::fromStdString(m_prefs.SaveFolderPath()),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    ui->saveFolderPathLineEdit->setText(dir);
}

void IpFreelyPreferencesDialog::on_selectNonePushButton_clicked()
{
    for (int row = 0; row < ui->scheduleTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->scheduleTableWidget->columnCount(); ++col)
        {
            ui->scheduleTableWidget->item(row, col)->setCheckState(Qt::Unchecked);
        }
    }
}

void IpFreelyPreferencesDialog::on_selectAllPushButton_clicked()
{
    for (int row = 0; row < ui->scheduleTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->scheduleTableWidget->columnCount(); ++col)
        {
            ui->scheduleTableWidget->item(row, col)->setCheckState(Qt::Checked);
        }
    }
}

void IpFreelyPreferencesDialog::on_revertSchedulePushButton_clicked()
{
    InitialisSchedule();
}

void IpFreelyPreferencesDialog::InitialisSchedule()
{
    auto schedule = m_prefs.RecordingSchedule();
    int  row      = 0;

    for (auto const& day : schedule)
    {
        int col = 0;

        for (auto hour : day)
        {
            ui->scheduleTableWidget->item(row, col++)
                ->setCheckState(hour ? Qt::Checked : Qt::Unchecked);
        }

        ++row;
    }
}
