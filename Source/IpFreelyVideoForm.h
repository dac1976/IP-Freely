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
 * \file IpFreelyVideoForm.h
 * \brief File containing declaration of IpFreelyVideoForm form.
 */
#ifndef IPFREELYVIDEOFORM_H
#define IPFREELYVIDEOFORM_H

#include <QWidget>

// Forward declarations.
namespace Ui
{
class IpFreelyVideoForm;
} // namespace Ui

class QImage;
class QShowEvent;
class QLabel;

/*! \brief Class defining a expanded video display form. */
class IpFreelyVideoForm : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief IpFreelyVideoForm constructor.
     * \param[in] parent - (Optional) Pointer to parent widget.
     */
    explicit IpFreelyVideoForm(QWidget* parent = 0);

    /*! \brief IpFreelyVideoForm destructor. */
    ~IpFreelyVideoForm();

    /*!
     * \brief SetVideoFrame sets the current frame of video in the display.
     * \param[in] videoFrame - The frame of video to display.
     * \param[in] fps - The video stream's actual FPS.
     * \param[in] motionBoundingRect - The video stream's detected motion bounding rectangle.
     * \param[in] streamBeingWritten - The video stream is currently having data recorded.
     */
    void SetVideoFrame(QImage const& videoFrame, double const fps, QRect const& motionBoundingRect,
                       bool const streamBeingWritten);

    /*!
     * \brief SetTitle sets title text of the form.
     * \param[in] title - The form's new title string.
     */
    void SetTitle(QString const& title);

protected:
    virtual void showEvent(QShowEvent* event);

private:
    void SetDisplaySize();

private:
    Ui::IpFreelyVideoForm* ui;
    bool                   m_resetSize;
    QLabel*                m_videoFrame;
    QString                m_title;
};

#endif // IPFREELYVIDEOFORM_H
