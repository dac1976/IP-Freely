/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
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
 * \file IpFreelyDownloadWidget.h
 * \brief File containing declaration of the IP camera download widget.
 */
#ifndef IPFREELYDOWNLOADWIDGET_H
#define IPFREELYDOWNLOADWIDGET_H

#include <QFrame>
#include <QTime>

// Forward declarations.
namespace Ui
{
class IpFreelyDownloadWidget;
} // namespace Ui

class QWebEngineDownloadItem;

/*! \brief The IpFreelyDownloadWidget class. */
class IpFreelyDownloadWidget final : public QFrame
{
    Q_OBJECT

public:
    /*!
     * \brief Initialising constructor.
     * \param[in] download - The web download item.
     * \param[in] parent - (Optional) The parent QWidget object.
     */
    explicit IpFreelyDownloadWidget(QWebEngineDownloadItem* download, QWidget* parent = nullptr);

    /*! \brief IpFreelyDownloadWidget destructor. */
    virtual ~IpFreelyDownloadWidget();

signals:
    /*!
     * \brief Signal removeClicked notifies a slot that we need to remove the download widget.
     * \param[in] self - The "web download item.
     * \param[in] parent - The parent QWidget object."this" pointer to this download object
     */
    void removeClicked(IpFreelyDownloadWidget* self);

private slots:
    void updateWidget();

private:
    QString withUnit(qreal bytes);

private:
    Ui::IpFreelyDownloadWidget* ui;
    QWebEngineDownloadItem*     m_download;
    QTime                       m_timeAdded;
};

#endif // IPFREELYDOWNLOADWIDGET_H
