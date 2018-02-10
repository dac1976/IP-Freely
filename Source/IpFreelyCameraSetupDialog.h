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
 * \file IpFreelyCameraSetupDialog.h
 * \brief File containing declaration of the IP camera setup dialog.
 */
#ifndef IPFREELYCAMERASETUPDIALOG_H
#define IPFREELYCAMERASETUPDIALOG_H

#include <QDialog>

// Forward declarations.
namespace Ui
{
class IpFreelyCameraSetupDialog;
} // namespace Ui

namespace ipfreely
{
struct IpCamera;
} // namespace ipfreely

class QShowEvent;

/*! \brief The IpFreelyCameraSetupDialog class. */
class IpFreelyCameraSetupDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Initialising constructor.
     * \param[in][out] camera - The camera settings object.
     * \param[in] parent - (Optional) The parent QWidget object.
     */
    explicit IpFreelyCameraSetupDialog(ipfreely::IpCamera& camera, QWidget* parent = nullptr);

    /*! \brief IpFreelyCameraSetupDialog destructor. */
    virtual ~IpFreelyCameraSetupDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_clearSettingsPushButton_clicked();
    void on_revertChangesPushButton_clicked();
    void on_motionDetectModeComboBox_currentIndexChanged(int index);

private:
    void SetDisplaySize();
    void InitialiseCameraSettings(ipfreely::IpCamera const& camera);

private:
    Ui::IpFreelyCameraSetupDialog* ui;
    ipfreely::IpCamera&            m_camera;
    bool                           m_clear;
};

#endif // IPFREELYCAMERASETUPDIALOG_H
