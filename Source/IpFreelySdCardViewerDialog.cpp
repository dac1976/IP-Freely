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
 * \file IpFreelySdCardViewerDialog.cpp
 * \brief File containing definition of the IP camera SD card browser dialog.
 */
#include "IpFreelySdCardViewerDialog.h"
#include "ui_IpFreelySdCardViewerDialog.h"
#include <QWebEngineView>
#include <QFileDialog>
#include <QWebEngineDownloadItem>
#include <QWebEngineProfile>
#include <QVBoxLayout>
#include "IpFreelyDownloadWidget.h"
#include "IpFreelyCameraDatabase.h"

IpFreelySdCardViewerDialog::IpFreelySdCardViewerDialog(ipfreely::IpCamera const& camera,
                                                       QWidget*                  parent)
    : QDialog(parent)
    , ui(new Ui::IpFreelySdCardViewerDialog)
    , m_webView(nullptr)
{
    ui->setupUi(this);

    connect(QWebEngineProfile::defaultProfile(),
            &QWebEngineProfile::downloadRequested,
            this,
            &IpFreelySdCardViewerDialog::downloadRequested);

    m_webView = new QWebEngineView(parent);
    m_webView->load(QUrl(QString::fromStdString(camera.CompleteStorageHttpUrl())));

    QString title = windowTitle();
    title += ": ";
    title += QString::fromStdString(camera.storageHttpUrl);
    setWindowTitle(title);

    layout()->addWidget(m_webView);
}

IpFreelySdCardViewerDialog::~IpFreelySdCardViewerDialog()
{
    delete ui;
}

void IpFreelySdCardViewerDialog::downloadRequested(QWebEngineDownloadItem* download)
{
    Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);

    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), download->path());

    if (path.isEmpty())
        return;

    download->setPath(path);
    download->accept();

    m_webView->setEnabled(false);

    auto downloadWidget = new IpFreelyDownloadWidget(download);

    connect(downloadWidget,
            &IpFreelyDownloadWidget::removeClicked,
            this,
            &IpFreelySdCardViewerDialog::removeClicked);

    layout()->addWidget(downloadWidget);
    qobject_cast<QVBoxLayout*>(layout())->setStretch(0, 1);
    qobject_cast<QVBoxLayout*>(layout())->setStretch(1, 0);
}

void IpFreelySdCardViewerDialog::removeClicked(IpFreelyDownloadWidget* downloadWidget)
{
    layout()->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
    m_webView->setEnabled(true);
}
