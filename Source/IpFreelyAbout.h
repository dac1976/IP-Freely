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
 * \file IpFreelyAbout.h
 * \brief File containing declaration of IpFreelyAbout dialog.
 */
#ifndef IPFREELYABOUT_H
#define IPFREELYABOUT_H

#include <QDialog>

// Forward declarations.
namespace Ui
{
class IpFreelyAbout;
} // namespace Ui

/*! \brief The IpFreelyAbout dialog class. */
class IpFreelyAbout : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Initialising constructor.
     * \param[in] parent - The parent QWidget object.
     */
    explicit IpFreelyAbout(QWidget* parent = nullptr);
    /*! \brief IpFreelyAbout destructor. */
    virtual ~IpFreelyAbout();
    /*!
     * \brief Set the title string of the dialog.
     * \param[in] title - The title string.
     */
    void SetTitle(const QString& title);

private:
    void SetDisplaySize();

private:
    Ui::IpFreelyAbout* ui;
};

#endif // IPFREELYABOUT_H
