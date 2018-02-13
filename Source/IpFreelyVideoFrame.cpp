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
 * \file IpFreelyVideoFrame.cpp
 * \brief File containing definition of IpFreelyVideoFrame widget.
 */
#include "IpFreelyVideoFrame.h"
#include "ui_IpFreelyVideoFrame.h"
#include <QMouseEvent>
#include <QImage>
#include <QRubberBand>
#include <QRect>
#include <QRectF>
#include <QSize>

IpFreelyVideoFrame::IpFreelyVideoFrame(int const                   cameraId,
                                       selection_callback_t const& selectionCallback,
                                       QWidget*                    parent)
    : QFrame(parent)
    , ui(new Ui::IpFreelyVideoFrame)
    , m_cameraId(cameraId)
    , m_selectionCallback(selectionCallback)
    , m_rubberBand(nullptr)
    , m_enableSelection(false)
    , m_videoHeight(0)
    , m_videoWidth(0)
{
    ui->setupUi(this);
}

IpFreelyVideoFrame::~IpFreelyVideoFrame()
{
    delete ui;
}

void IpFreelyVideoFrame::SetVideoFrame(QImage const& videoFrame)
{
    m_videoHeight = videoFrame.height();
    m_videoWidth  = videoFrame.width();

    ui->videoFrameLabel->setPixmap(QPixmap::fromImage(videoFrame));
}

void IpFreelyVideoFrame::SetEnableSelection(bool const enable)
{
    m_enableSelection = enable;
}

void IpFreelyVideoFrame::mousePressEvent(QMouseEvent* event)
{
    if (!m_enableSelection)
    {
        return;
    }

    m_origin = event->pos();

    if (!m_rubberBand)
    {
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    }

    m_rubberBand->setGeometry(QRect(m_origin, QSize()));
    m_rubberBand->show();
}

void IpFreelyVideoFrame::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_enableSelection || !m_rubberBand)
    {
        return;
    }

    m_rubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
}

void IpFreelyVideoFrame::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (!m_enableSelection || !m_selectionCallback || !m_rubberBand)
    {
        return;
    }

    auto selection = m_rubberBand->geometry();
    m_rubberBand->hide();

    double t = static_cast<double>(selection.top()) / static_cast<double>(m_videoHeight);
    double l = static_cast<double>(selection.left()) / static_cast<double>(m_videoWidth);
    double h = static_cast<double>(selection.height()) / static_cast<double>(m_videoHeight);
    double w = static_cast<double>(selection.width()) / static_cast<double>(m_videoWidth);

    if ((t >= 1.0) || (l >= 1.0) || (h >= 1.0) || (w >= 1.0) || (t + h > 1.0) || (l + w) > 1.0)
    {
        return;
    }

    m_selectionCallback(m_cameraId, QRectF(l, t, w, h));
}
