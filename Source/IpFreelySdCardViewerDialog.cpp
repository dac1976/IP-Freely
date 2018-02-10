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
#include <QScreen>
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

    SetDisplaySize();
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

void IpFreelySdCardViewerDialog::SetDisplaySize()
{
    static constexpr double DEFAULT_SCREEN_SIZE = 1080.0;
    static constexpr int    MIN_DISPLAY_WIDTH   = 640;
    static constexpr int    MIN_DISPLAY_HEIGHT  = 480;

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
