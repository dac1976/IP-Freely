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
#include "IpFreelyDownloadWidget.h"
#include "ui_IpFreelyDownloadWidget.h"
#include <QFileInfo>
#include <QUrl>
#include <QWebEngineDownloadItem>

IpFreelyDownloadWidget::IpFreelyDownloadWidget(QWebEngineDownloadItem* download, QWidget* parent)
    : QFrame(parent)
    , ui(new Ui::IpFreelyDownloadWidget)
    , m_download(download)
    , m_timeAdded(QTime::currentTime())
{
    ui->setupUi(this);

    ui->dstName->setText(QFileInfo(m_download->path()).fileName());
    ui->srcUrl->setText(m_download->url().toDisplayString());

    connect(ui->cancelButton, &QPushButton::clicked, [this](bool) {
        if (m_download->state() == QWebEngineDownloadItem::DownloadInProgress)
            m_download->cancel();
        else
            emit removeClicked(this);
    });

    connect(m_download,
            &QWebEngineDownloadItem::downloadProgress,
            this,
            &IpFreelyDownloadWidget::updateWidget);

    connect(m_download,
            &QWebEngineDownloadItem::stateChanged,
            this,
            &IpFreelyDownloadWidget::updateWidget);

    updateWidget();
}

IpFreelyDownloadWidget::~IpFreelyDownloadWidget()
{
    delete ui;
}

inline QString IpFreelyDownloadWidget::withUnit(qreal bytes)
{
    if (bytes < (1 << 10))
    {
        return tr("%L1 B").arg(bytes);
    }
    else if (bytes < (1 << 20))
    {
        return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
    }
    else if (bytes < (1 << 30))
    {
        return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
    }
    else
    {
        return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
    }
}

void IpFreelyDownloadWidget::updateWidget()
{
    qreal totalBytes     = m_download->totalBytes();
    qreal receivedBytes  = m_download->receivedBytes();
    qreal bytesPerSecond = receivedBytes / m_timeAdded.elapsed() * 1000;

    auto state = m_download->state();
    switch (state)
    {
    case QWebEngineDownloadItem::DownloadRequested:
        Q_UNREACHABLE();
        break;
    case QWebEngineDownloadItem::DownloadInProgress:
        if (totalBytes >= 0)
        {
            ui->progressBar->setValue(qRound(100 * receivedBytes / totalBytes));
            ui->progressBar->setDisabled(false);
            ui->progressBar->setFormat(tr("%p% - %1 of %2 downloaded - %3/s")
                                           .arg(withUnit(receivedBytes))
                                           .arg(withUnit(totalBytes))
                                           .arg(withUnit(bytesPerSecond)));
        }
        else
        {
            ui->progressBar->setValue(0);
            ui->progressBar->setDisabled(false);
            ui->progressBar->setFormat(tr("unknown size - %1 downloaded - %2/s")
                                           .arg(withUnit(receivedBytes))
                                           .arg(withUnit(bytesPerSecond)));
        }
        break;
    case QWebEngineDownloadItem::DownloadCompleted:
        ui->progressBar->setValue(100);
        ui->progressBar->setDisabled(true);
        ui->progressBar->setFormat(tr("completed - %1 downloaded - %2/s")
                                       .arg(withUnit(receivedBytes))
                                       .arg(withUnit(bytesPerSecond)));
        emit removeClicked(this);
        break;
    case QWebEngineDownloadItem::DownloadCancelled:
        ui->progressBar->setValue(0);
        ui->progressBar->setDisabled(true);
        ui->progressBar->setFormat(tr("cancelled - %1 downloaded - %2/s")
                                       .arg(withUnit(receivedBytes))
                                       .arg(withUnit(bytesPerSecond)));
        emit removeClicked(this);
        break;
    case QWebEngineDownloadItem::DownloadInterrupted:
        ui->progressBar->setValue(0);
        ui->progressBar->setDisabled(true);
        ui->progressBar->setFormat(tr("interrupted: %1").arg(m_download->interruptReasonString()));
        break;
    }
}
