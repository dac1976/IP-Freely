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
 * \file IpFreelyVideoForm.cpp
 * \brief File containing definition of IpFreelyVideoForm form.
 */
#include "IpFreelyVideoForm.h"
#include "ui_IpFreelyVideoForm.h"
#include <QLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QShowEvent>
#include <QScreen>

IpFreelyVideoForm::IpFreelyVideoForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IpFreelyVideoForm)
    , m_resetSize(true)
    , m_videoFrame(new QLabel(this))

{
    ui->setupUi(this);
    layout()->addWidget(m_videoFrame);

    Qt::WindowFlags flags = this->windowFlags();
    flags                 = flags & ~Qt::WindowContextHelpButtonHint;
    this->setWindowFlags(flags);
}

IpFreelyVideoForm::~IpFreelyVideoForm()
{
    delete ui;
}

void IpFreelyVideoForm::SetVideoFrame(QImage const& videoFrame, double const fps)
{
    auto title = m_title + ": " + QString::number(fps) + tr(" FPS");
    setWindowTitle(title);

    double frameAspectRatio =
        static_cast<double>(videoFrame.width()) / static_cast<double>(videoFrame.height());

    if (m_resetSize)
    {
        m_resetSize            = false;
        auto availableGeometry = QApplication::primaryScreen()->availableGeometry();
        auto w                 = 0.9 * static_cast<double>(availableGeometry.width() -
                                           (layout()->contentsMargins().left() +
                                            layout()->contentsMargins().right() + 2));
        auto h                 = 0.9 * static_cast<double>(availableGeometry.height() -
                                           (layout()->contentsMargins().top() +
                                            layout()->contentsMargins().bottom() + 2));

        if (videoFrame.height() >= static_cast<int>(h))
        {
            w = h * frameAspectRatio;
        }
        else
        {
            h = videoFrame.height() + layout()->contentsMargins().top() +
                layout()->contentsMargins().bottom() + 2;
            w = h * frameAspectRatio;
        }

        setMinimumSize(static_cast<int>(w), static_cast<int>(h));
        setMaximumSize(static_cast<int>(w), static_cast<int>(h));
    }

    m_videoFrame->setPixmap(QPixmap::fromImage(videoFrame.scaled(m_videoFrame->width(),
                                                                 m_videoFrame->height(),
                                                                 Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation)));
}

void IpFreelyVideoForm::SetTitle(QString const& title)
{
    m_title = title;
    setWindowTitle(m_title);
}

void IpFreelyVideoForm::showEvent(QShowEvent* /*event*/)
{
    m_resetSize = true;
}