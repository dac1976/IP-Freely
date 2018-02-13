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
 * \file IpFreelyVideoFrame.h
 * \brief File containing declaration of IpFreelyVideoFrame widget.
 */
#ifndef IPFREELYVIDEOFRAME_H
#define IPFREELYVIDEOFRAME_H

#include <QFrame>
#include <QPoint>
#include <functional>

// Forward declarations.
namespace Ui
{
class IpFreelyVideoFrame;
} // namespace Ui

class QMouseEvent;
class QImage;
class QRubberBand;
class QRectF;

/*! \brief Class defining an embeddable frame to display video frames. */
class IpFreelyVideoFrame : public QFrame
{
    Q_OBJECT

    typedef std::function<void(int const, QRectF const&)> selection_callback_t;

public:
    /*!
     * \brief IpFreelyVideoFrame constructor.
     * \param[in] cameraId - A unique integer camera ID.
     * \param[in] selectionCallback - Callback to be fired when dragging of motion region selection
     * box finishes.
     * \param[in] parent - (Optional) Pointer to parent widget.
     */
    IpFreelyVideoFrame(int const cameraId, selection_callback_t const& selectionCallback,
                       QWidget* parent = nullptr);

    /*! \brief IpFreelyVideoFrame destructor. */
    virtual ~IpFreelyVideoFrame();

    /*!
     * \brief SetVideoFrame sets the current video frame to be displayed.
     * \param[in] videoFrame - A QImage containing the video frame.
     */
    void SetVideoFrame(QImage const& videoFrame);

    /*!
     * \brief SetEnableSelection sets the current video frame to be displayed.
     * \param[in] enable - Flag to enable/disable selection rubberband.
     */
    void SetEnableSelection(bool const enable);

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::IpFreelyVideoFrame* ui;
    int                     m_cameraId;
    selection_callback_t    m_selectionCallback;
    QPoint                  m_origin;
    QRubberBand*            m_rubberBand;
    bool                    m_enableSelection;
    int                     m_videoHeight;
    int                     m_videoWidth;
};

#endif // IPFREELYVIDEOFRAME_H
