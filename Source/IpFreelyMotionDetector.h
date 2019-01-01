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
 * \file IpFreelyMotionDetector.h
 * \brief File containing declaration of IpFreelyMotionDetector threaded class.
 */
#ifndef IPFREELYMOTIONDETECTOR_H
#define IPFREELYMOTIONDETECTOR_H

#include <QRect>
#include <string>
#include <memory>
#include <ctime>
#include <opencv2/opencv.hpp>
#include "Threads/MessageQueueThread.h"
#include "IpFreelyCameraDatabase.h"

/*! \brief The ipfreely namespace. */
namespace ipfreely
{

/*! \brief Class defining a motion detector. */
class IpFreelyMotionDetector final
{
    /*! \brief Typedef to queue object. */
    typedef std::shared_ptr<cv::Mat> video_frame_t;

public:
    /*!
     * \brief IpFreelyMotionDetector constructor.
     * \param[in] name - A name for the stream, used to name output video files.
     * \param[in] cameraDetails - Camera details we want to stream from.
     * \param[in] saveFolderPath - A local folder to save captured videos to.
     * \param[in] requiredFileDurationSecs - Duration to use for captured video files.
     * \param[in] fps - The video's FPS.
     * \param[in] originalWidth - The video's original width.
     * \param[in] originalHeight - The video's original height.
     *
     * The stream processor can be used to receive and thus display RTSP video streams but can also
     * record the stream in DivX format mp4 files to disk. Files are recorded with the given
     * duration. One recording session can span multiple back-to-back video files.
     */
    IpFreelyMotionDetector(std::string const& name, IpCamera const& cameraDetails,
                           std::string const& saveFolderPath, double const requiredFileDurationSecs,
                           double const fps, int const originalWidth, int const originalHeight);

    /*! \brief IpFreelyMotionDetector destructor. */
    ~IpFreelyMotionDetector() = default;

    /*! \brief IpFreelyMotionDetector deleted copy constructor. */
    IpFreelyMotionDetector(IpFreelyMotionDetector const&) = delete;

    /*! \brief IpFreelyMotionDetector deleted copy assignment operator. */
    IpFreelyMotionDetector& operator=(IpFreelyMotionDetector const&) = delete;

    /*!
     * \brief AddNextFrame add next video frame to motion detector queue.
     * \param[in] videoFrame - Next video frame to process.
     */
    void AddNextFrame(cv::Mat const& videoFrame);

    /*!
     * \brief CurrentMotionRect gives acces to motion bounding rectangle.
     * \return A QRect defining the motion boudning rectangle.
     */
    QRect CurrentMotionRect() const noexcept;

    /*!
     * \brief WritingStream reports if motion detector is currently writing the viedo stream to
     * disk.
     * \return |True if writong, false otherwise.
     */
    bool WritingStream() const noexcept;

private:
    void       Initialise();
    void       InitialiseFrames();
    void       UpdateNextFrame();
    bool       DetectMotion();
    bool       CheckForIntersections();
    void       RotateFrames();
    static int MessageDecoder(video_frame_t const& msg);
    bool       MessageHandler(video_frame_t& msg);
    void       CreateCaptureObjects();
    void       WriteVideoFrame();
    void       SetWritingStream(bool const writing) noexcept;

private:
    mutable std::mutex                                        m_motionMutex{};
    mutable std::mutex                                        m_writingMutex{};
    mutable std::mutex                                        m_fpsMutex{};
    std::string                                               m_name{"cam"};
    IpCamera                                                  m_cameraDetails{};
    std::string                                               m_saveFolderPath{};
    double                                                    m_requiredFileDurationSecs{0.0};
    double                                                    m_fps{25.0};
    int                                                       m_originalWidth{0};
    int                                                       m_originalHeight{0};
    unsigned int                                              m_updatePeriodMillisecs{40};
    cv::Mat                                                   m_erosionKernel{};
    cv::Scalar                                                m_rectangleColor{0, 255, 0};
    video_frame_t                                             m_originalFrame;
    size_t                                                    m_holdOffFrameCountLimit{0};
    size_t                                                    m_holdOffFrameCount{0};
    double                                                    m_motionFrameScalar{1.0};
    int                                                       m_minImageChangeArea{0};
    size_t                                                    m_imageChangesThreshold{0};
    bool                                                      m_initialiseFrames{true};
    cv::Mat                                                   m_prevGreyFrame{};
    cv::Mat                                                   m_currentGreyFrame{};
    cv::Mat                                                   m_nextGreyFrame{};
    cv::Rect                                                  m_motionBoundingRect{0, 0, 0, 0};
    double                                                    m_fileDurationSecs{0.0};
    time_t                                                    m_currentTime{};
    cv::Ptr<cv::VideoWriter>                                  m_videoWriter{};
    bool                                                      m_writingStream{false};
    core_lib::threads::MessageQueueThread<int, video_frame_t> m_msgQueueThread;
};

} // namespace ipfreely

#endif // IPFREELYMOTIONDETECTOR_H
