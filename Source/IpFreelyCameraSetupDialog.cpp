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
#include <QScreen>
#include "IpFreelyCameraDatabase.h"

static constexpr double LOW_SENSITIVITY_DIFF_THRESHOLD    = 75.0;
static constexpr double MEDIUM_SENSITIVITY_DIFF_THRESHOLD = 50.0;
static constexpr double HIGH_SENSITIVITY_DIFF_THRESHOLD   = 35.0;
static constexpr double LOW_SENSITIVITY_STDDEV            = 10.0;
static constexpr double MEDIUM_SENSITIVITY_STDDEV         = 20.0;
static constexpr double HIGH_SENSITIVITY_STDDEV           = 40.0;
static constexpr double LOW_SENSITIVITY_AREA_PERCENT      = 0.05;
static constexpr double MEDIUM_SENSITIVITY_AREA_PERCENT   = 0.025;
static constexpr double HIGH_SENSITIVITY_AREA_PERCENT     = 0.01;
static constexpr double BOUNDING_RECT_SMOOTHING_FACTOR    = 0.1;

IpFreelyCameraSetupDialog::IpFreelyCameraSetupDialog(ipfreely::IpCamera& camera, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IpFreelyCameraSetupDialog)
    , m_camera(camera)
    , m_clear(false)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = this->windowFlags();
    flags                 = flags & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);

    SetDisplaySize();

    InitialiseCameraSettings(m_camera);
}

IpFreelyCameraSetupDialog::~IpFreelyCameraSetupDialog()
{
    delete ui;
}

void IpFreelyCameraSetupDialog::on_buttonBox_accepted()
{
    if (m_clear)
    {
        m_clear  = false;
        m_camera = {};
    }

    m_camera.rtspUrl                  = ui->rtspUrlLineEdit->text().toStdString();
    m_camera.storageHttpUrl           = ui->storageUrlLineEdit->text().toStdString();
    m_camera.username                 = ui->usernameLineEdit->text().toStdString();
    m_camera.password                 = ui->passwordLineEdit->text().toStdString();
    m_camera.description              = ui->descriptionLineEdit->text().toStdString();
    m_camera.enableScheduledRecording = ui->scheduledRecordingCheckBox->checkState() == Qt::Checked;

    switch (ui->motionDetectModeComboBox->currentIndex())
    {
    case 1:
        m_camera.motionDectorMode = ipfreely::eMotionDetectorMode::lowSensitivity;
        break;
    case 2:
        m_camera.motionDectorMode = ipfreely::eMotionDetectorMode::mediumSensitivity;
        break;
    case 3:
        m_camera.motionDectorMode = ipfreely::eMotionDetectorMode::highSensitivity;
        break;
    case 4:
        m_camera.motionDectorMode = ipfreely::eMotionDetectorMode::manual;
        break;
    case 0:
    default:
        m_camera.motionDectorMode = ipfreely::eMotionDetectorMode::off;
        break;
    }

    m_camera.pixelThreshold             = ui->pixelLevelThresholdDoubleSpinBox->value();
    m_camera.maxMotionStdDev            = ui->maxStdDevDoubleSpinBox->value();
    m_camera.minMotionAreaPercentFactor = ui->minMotionAreaPercentDoubleSpinBox->value() / 100.0;
    m_camera.motionAreaAveFactor        = ui->motionAreaAveFactorDoubleSpinBox->value();
    m_camera.shrinkVideoFrames          = ui->shrinkFramesCheckBox->checkState() == Qt::Checked;

    accept();
}

void IpFreelyCameraSetupDialog::on_buttonBox_rejected()
{
    reject();
}

void IpFreelyCameraSetupDialog::on_clearSettingsPushButton_clicked()
{
    m_clear = true;
    ipfreely::IpCamera blankCamera;
    InitialiseCameraSettings(blankCamera);
}

void IpFreelyCameraSetupDialog::on_revertChangesPushButton_clicked()
{
    m_clear = false;
    InitialiseCameraSettings(m_camera);
}

void IpFreelyCameraSetupDialog::on_motionDetectModeComboBox_currentIndexChanged(int index)
{
    ui->motionDetectSettingsGroupBox->setEnabled(index == 4);

    switch (index)
    {
    case 1:
        ui->maxStdDevDoubleSpinBox->setValue(LOW_SENSITIVITY_STDDEV);
        ui->minMotionAreaPercentDoubleSpinBox->setValue(LOW_SENSITIVITY_AREA_PERCENT * 100.0);
        ui->pixelLevelThresholdDoubleSpinBox->setValue(LOW_SENSITIVITY_DIFF_THRESHOLD);
        ui->motionAreaAveFactorDoubleSpinBox->setValue(BOUNDING_RECT_SMOOTHING_FACTOR);
        break;
    case 2:
        ui->maxStdDevDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_STDDEV);
        ui->minMotionAreaPercentDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_AREA_PERCENT * 100.0);
        ui->pixelLevelThresholdDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_DIFF_THRESHOLD);
        ui->motionAreaAveFactorDoubleSpinBox->setValue(BOUNDING_RECT_SMOOTHING_FACTOR);
        break;
    case 3:
        ui->maxStdDevDoubleSpinBox->setValue(HIGH_SENSITIVITY_STDDEV);
        ui->minMotionAreaPercentDoubleSpinBox->setValue(HIGH_SENSITIVITY_AREA_PERCENT * 100.0);
        ui->pixelLevelThresholdDoubleSpinBox->setValue(HIGH_SENSITIVITY_DIFF_THRESHOLD);
        ui->motionAreaAveFactorDoubleSpinBox->setValue(BOUNDING_RECT_SMOOTHING_FACTOR);
        break;
    case 4:
        ui->maxStdDevDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_STDDEV);
        ui->minMotionAreaPercentDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_AREA_PERCENT * 100.0);
        ui->pixelLevelThresholdDoubleSpinBox->setValue(MEDIUM_SENSITIVITY_DIFF_THRESHOLD);
        ui->motionAreaAveFactorDoubleSpinBox->setValue(BOUNDING_RECT_SMOOTHING_FACTOR);
        break;
    case 0:
    default:
        ui->maxStdDevDoubleSpinBox->setValue(0.0);
        ui->minMotionAreaPercentDoubleSpinBox->setValue(0.0);
        ui->pixelLevelThresholdDoubleSpinBox->setValue(0.0);
        ui->motionAreaAveFactorDoubleSpinBox->setValue(0.0);
        break;
    }
}

void IpFreelyCameraSetupDialog::SetDisplaySize()
{
    static constexpr double DEFAULT_SCREEN_SIZE = 1080.0;
    static constexpr int    MIN_DISPLAY_WIDTH   = 640;
    static constexpr int    MIN_DISPLAY_HEIGHT  = 490;

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

void IpFreelyCameraSetupDialog::InitialiseCameraSettings(ipfreely::IpCamera const& camera)
{
    ui->rtspUrlLineEdit->setText(QString::fromStdString(camera.rtspUrl));
    ui->storageUrlLineEdit->setText(QString::fromStdString(camera.storageHttpUrl));
    ui->usernameLineEdit->setText(QString::fromStdString(camera.username));
    ui->passwordLineEdit->setText(QString::fromStdString(camera.password));
    ui->descriptionLineEdit->setText(QString::fromStdString(camera.description));
    ui->scheduledRecordingCheckBox->setCheckState(camera.enableScheduledRecording ? Qt::Checked
                                                                                  : Qt::Unchecked);
    switch (m_camera.motionDectorMode)
    {
    case ipfreely::eMotionDetectorMode::off:
        ui->motionDetectModeComboBox->setCurrentIndex(0);
        break;
    case ipfreely::eMotionDetectorMode::lowSensitivity:
        ui->motionDetectModeComboBox->setCurrentIndex(1);
        break;
    case ipfreely::eMotionDetectorMode::mediumSensitivity:
        ui->motionDetectModeComboBox->setCurrentIndex(2);
        break;
    case ipfreely::eMotionDetectorMode::highSensitivity:
        ui->motionDetectModeComboBox->setCurrentIndex(3);
        break;
    case ipfreely::eMotionDetectorMode::manual:
        ui->motionDetectModeComboBox->setCurrentIndex(4);
        break;
    }

    ui->pixelLevelThresholdDoubleSpinBox->setValue(camera.pixelThreshold);
    ui->maxStdDevDoubleSpinBox->setValue(camera.maxMotionStdDev);
    ui->minMotionAreaPercentDoubleSpinBox->setValue(camera.minMotionAreaPercentFactor * 100.0);
    ui->motionAreaAveFactorDoubleSpinBox->setValue(camera.motionAreaAveFactor);
    ui->shrinkFramesCheckBox->setCheckState(camera.shrinkVideoFrames ? Qt::Checked : Qt::Unchecked);
}
