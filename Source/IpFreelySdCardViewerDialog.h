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
 * \file IpFreelySdCardViewerDialog.h
 * \brief File containing declaration of the IP camera SD card browser dialog.
 */
#ifndef IPFREELYSDCARDVIEWERDIALOG_H
#define IPFREELYSDCARDVIEWERDIALOG_H

#include <QDialog>

// Forward declarations.
namespace Ui
{
class IpFreelySdCardViewerDialog;
} // namespace Ui

namespace ipfreely
{
struct IpCamera;
} // namespace ipfreely

class QWebEngineDownloadItem;
class QWebEngineView;
class IpFreelyDownloadWidget;

/*! \brief The IpFreelySdCardViewerDialog class. */
class IpFreelySdCardViewerDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Initialising constructor.
     * \param[in] camera - The camera details.
     * \param[in] parent - (Optional) The parent QWidget object.
     */
    explicit IpFreelySdCardViewerDialog(ipfreely::IpCamera const& camera,
                                        QWidget*                  parent = nullptr);

    /*! \brief IpFreelyDownloadWidget destructor. */
    virtual ~IpFreelySdCardViewerDialog();

private slots:
    void removeClicked(IpFreelyDownloadWidget* downloadWidget);
    void downloadRequested(QWebEngineDownloadItem* download);

private:
    void SetDisplaySize();

private:
    Ui::IpFreelySdCardViewerDialog* ui;
    QWebEngineView*                 m_webView;
};

#endif // IPFREELYSDCARDVIEWERDIALOG_H
