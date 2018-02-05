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
 * \file IpFreelyPreferencesDialog.h
 * \brief File containing declaration of IpFreelyPreferencesDialog form.
 */
#ifndef IPFREELYPREFERENCESDIALOG_H
#define IPFREELYPREFERENCESDIALOG_H

#include <QDialog>

// Forward declarations.
namespace Ui
{
class IpFreelyPreferencesDialog;
} // namespace Ui

namespace ipfreely
{
class IpFreelyPreferences;
} // namespace ipfreely

/*! \brief The IpFreelyPreferencesDialog class. */
class IpFreelyPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Initialising constructor.
     * \param[in][out] prefs - The preferences object.
     * \param[in] parent - (Optional) The parent QWidget object.
     */
    explicit IpFreelyPreferencesDialog(ipfreely::IpFreelyPreferences& prefs, QWidget* parent = 0);

    /*! \brief IpFreelyPreferencesDialog destructor. */
    virtual ~IpFreelyPreferencesDialog();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_saveFolderPathToolButton_clicked();
    void on_selectNonePushButton_clicked();
    void on_selectAllPushButton_clicked();
    void on_revertSchedulePushButton_clicked();
    void on_selectNoneMtPushButton_clicked();
    void on_selectAllMtPushButton_clicked();
    void on_revertScheduleMtPushButton_clicked();

private:
    void SetDisplaySize();
    void InitialisSchedules();

private:
    Ui::IpFreelyPreferencesDialog* ui;
    ipfreely::IpFreelyPreferences& m_prefs;
};

#endif // IPFREELYPREFERENCESDIALOG_H
