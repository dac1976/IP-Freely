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
#include <QScreen>
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

    SetDisplaySize();

    InitialisSchedules();
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

    schedule = m_prefs.MotionTrackingSchedule();

    for (int row = 0; row < ui->motionTrackingTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->motionTrackingTableWidget->columnCount(); ++col)
        {
            schedule[static_cast<size_t>(row)][static_cast<size_t>(col)] =
                ui->motionTrackingTableWidget->item(row, col)->checkState() == Qt::Checked;
        }
    }

    m_prefs.SetMotionTrackingSchedule(schedule);

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

void IpFreelyPreferencesDialog::SetDisplaySize()
{
    static constexpr double DEFAULT_SCREEN_SIZE = 1080.0;
    static constexpr int    MIN_DISPLAY_WIDTH   = 600;
    static constexpr int    MIN_DISPLAY_HEIGHT  = 320;

    auto      displayGeometry = geometry();
    auto      screenPos       = mapToGlobal(QPoint(displayGeometry.left(), displayGeometry.top()));
    auto      screen          = qApp->screenAt(screenPos);
    double    scaleFactor     = static_cast<double>(screen->size().height()) / DEFAULT_SCREEN_SIZE;
    int const maxDisplayWidth =
        static_cast<int>(static_cast<double>(screen->size().width()) * 0.75);
    int const maxDisplayHeight =
        static_cast<int>(static_cast<double>(screen->size().height()) * 0.75);

    int displayWidth = static_cast<int>(static_cast<double>(displayGeometry.width()) * scaleFactor);

    if (displayWidth < MIN_DISPLAY_WIDTH)
    {
        displayWidth = MIN_DISPLAY_WIDTH;
    }
    else if (displayWidth > maxDisplayWidth)
    {
        displayWidth = maxDisplayWidth;
    }

    int displayHeight =
        static_cast<int>(static_cast<double>(displayGeometry.height()) * scaleFactor);

    if (displayHeight < MIN_DISPLAY_HEIGHT)
    {
        displayHeight = MIN_DISPLAY_HEIGHT;
    }
    else if (displayHeight > maxDisplayHeight)
    {
        displayHeight = maxDisplayHeight;
    }

    int displayLeft =
        static_cast<int>(static_cast<double>(screen->size().width() - displayWidth) / 2.0);

    int displayTop =
        static_cast<int>(static_cast<double>(screen->size().height() - displayHeight) / 2.0);

    displayGeometry.setTop(displayTop);
    displayGeometry.setLeft(displayLeft);
    displayGeometry.setWidth(displayWidth);
    displayGeometry.setHeight(displayHeight);
    setGeometry(displayGeometry);
}

void IpFreelyPreferencesDialog::InitialisSchedules()
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

    schedule = m_prefs.MotionTrackingSchedule();
    row      = 0;

    for (auto const& day : schedule)
    {
        int col = 0;

        for (auto hour : day)
        {
            ui->motionTrackingTableWidget->item(row, col++)
                ->setCheckState(hour ? Qt::Checked : Qt::Unchecked);
        }

        ++row;
    }
}

void IpFreelyPreferencesDialog::on_selectNoneMtPushButton_clicked()
{
    for (int row = 0; row < ui->motionTrackingTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->motionTrackingTableWidget->columnCount(); ++col)
        {
            ui->motionTrackingTableWidget->item(row, col)->setCheckState(Qt::Unchecked);
        }
    }
}

void IpFreelyPreferencesDialog::on_selectAllMtPushButton_clicked()
{
    for (int row = 0; row < ui->motionTrackingTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < ui->motionTrackingTableWidget->columnCount(); ++col)
        {
            ui->motionTrackingTableWidget->item(row, col)->setCheckState(Qt::Checked);
        }
    }
}

void IpFreelyPreferencesDialog::on_revertScheduleMtPushButton_clicked()
{
    auto schedule = m_prefs.MotionTrackingSchedule();
    int  row      = 0;

    for (auto const& day : schedule)
    {
        int col = 0;

        for (auto hour : day)
        {
            ui->motionTrackingTableWidget->item(row, col++)
                ->setCheckState(hour ? Qt::Checked : Qt::Unchecked);
        }

        ++row;
    }
}
