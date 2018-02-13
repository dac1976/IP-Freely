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
#include <QPainter>
#include <QPen>
#include <QBrush>

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

void IpFreelyVideoForm::SetVideoFrame(QImage const& videoFrame, double const fps,
                                      QRect const& motionBoundingRect,
                                      bool const streamBeingWritten, regions_t const& motionRegions)
{
    auto title = m_title + ": " + QString::number(fps) + tr(" FPS");
    setWindowTitle(title);

    double frameAspectRatio =
        static_cast<double>(videoFrame.width()) / static_cast<double>(videoFrame.height());

    if (m_resetSize)
    {
        m_resetSize            = false;
        auto availableGeometry = qApp->screenAt(this->geometry().topLeft())->availableGeometry();
        auto w                 = 0.9 * static_cast<double>(availableGeometry.width() -
                                           (layout()->contentsMargins().left() +
                                            layout()->contentsMargins().right() + 2));
        auto h = 0.9 * static_cast<double>(availableGeometry.height() -
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

    double scalar =
        static_cast<double>(m_videoFrame->height()) / static_cast<double>(videoFrame.height());

    auto resizedImage = videoFrame.scaled(m_videoFrame->width(),
                                          m_videoFrame->height(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);

    QPainter p(&resizedImage);

    if (!motionRegions.empty())
    {
        auto pen = QPen(Qt::cyan);
        pen.setWidth(2);
        p.setPen(pen);
        p.setBackground(QBrush(Qt::NoBrush));
        p.setBackgroundMode(Qt::TransparentMode);
        p.setBrush(QBrush(Qt::NoBrush));

        for (auto const& motionRegion : motionRegions)
        {
            QRect rect;
            rect.setTop(static_cast<int>(static_cast<double>(resizedImage.height()) *
                                         motionRegion.first.second));
            rect.setLeft(static_cast<int>(static_cast<double>(resizedImage.width()) *
                                          motionRegion.first.first));
            rect.setRight(static_cast<int>(
                static_cast<double>(rect.left()) +
                (static_cast<double>(resizedImage.width()) * motionRegion.second.first)));
            rect.setBottom(static_cast<int>(
                static_cast<double>(rect.top()) +
                (static_cast<double>(resizedImage.height()) * motionRegion.second.second)));
            p.drawRect(rect);
        }
    }

    auto rect = motionBoundingRect;

    if (!rect.isNull())
    {
        rect.setTop(static_cast<int>(static_cast<double>(rect.top()) * scalar));
        rect.setLeft(static_cast<int>(static_cast<double>(rect.left()) * scalar));
        rect.setRight(static_cast<int>(static_cast<double>(rect.right()) * scalar));
        rect.setBottom(static_cast<int>(static_cast<double>(rect.bottom()) * scalar));

        auto pen = QPen(Qt::green);
        pen.setWidth(2);
        p.setPen(pen);
        p.setBackground(QBrush(Qt::NoBrush));
        p.setBackgroundMode(Qt::TransparentMode);
        p.setBrush(QBrush(Qt::NoBrush));
        p.drawRect(rect);
    }

    if (streamBeingWritten)
    {
        p.setPen(QPen(Qt::red));
        p.setBackground(QBrush(Qt::white, Qt::SolidPattern));
        p.setBackgroundMode(Qt::OpaqueMode);
        p.setFont(QFont("Segoe UI", 16, QFont::Bold));
        auto posRec = resizedImage.rect();
        posRec.setTop(posRec.top() + 16);
        p.drawText(posRec, Qt::AlignHCenter | Qt::AlignTop, tr("Recording").toLocal8Bit());
    }

    m_videoFrame->setPixmap(QPixmap::fromImage(resizedImage));
}

void IpFreelyVideoForm::SetTitle(QString const& title)
{
    m_title = title;
    setWindowTitle(m_title);
}

void IpFreelyVideoForm::showEvent(QShowEvent* event)
{
    m_resetSize = true;

    QWidget::showEvent(event);
}
