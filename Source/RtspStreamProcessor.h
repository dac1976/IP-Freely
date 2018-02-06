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
 * \file RtspStreamProcessor.h
 * \brief File containing declaration of RtspStreamProcessor threaded class.
 */
#ifndef RTSPSTREAMPROCESSOR_H
#define RTSPSTREAMPROCESSOR_H

#include <QImage>
#include <string>
#include <cstdint>
#include <vector>
#include <ctime>
#include <opencv/cv.hpp>
#include "Threads/ThreadBase.h"
#include "Threads/SyncEvent.h"

// Forward declarations
namespace cv
{
class VideoCapture;
class Mat;
class VideoWriter;
} // namespace cv

/*! \brief The ipfreely namespace. */
namespace ipfreely
{

/*! \brief Class defining a RTSP stream processor thread. */
class RtspStreamProcessor final : public core_lib::threads::ThreadBase
{
public:
    /*!
     * \brief RtspStreamProcessor constructor.
     * \param[in] name - A name for the stream, used to name output video files.
     * \param[in] completeRtspUrl - The complete URL to the RSTP stream (inc username and password).
     * \param[in] saveFolderPath - A local folder to save captured videos to.
     * \param[in] requiredFileDurationSecs - Duration to use for captured video files.
     * \param[in] recordingSchedule - (Optional) The daily/hourly recording schedule.
     * \param[in] motionSchedule - (Optional) The daily/hourly motion tracking schedule.
     * \param[in] motionSensitivity - (Optional) The sensitivity of the motion tracking in
     * [0.0, 1.0].
     * \param[in] motionTrackInterval - (Optional) Mtion tracker will test grab a new reference
     * frame.
     *
     * The stream processor can be used to receive and thus display RTSP video streams but can also
     * record the stream in DivX format mp4 files to disk. Files are recorded with the given
     * duration. One recording sessino can span multiple back-to-back video files.
     */
    RtspStreamProcessor(std::string const& name, std::string const& completeRtspUrl,
                        std::string const& saveFolderPath, double const requiredFileDurationSecs,
                        std::vector<std::vector<bool>> const& recordingSchedule   = {},
                        std::vector<std::vector<bool>> const& motionSchedule      = {},
                        double const                          motionSensitivity   = 0.5,
                        double const                          motionTrackInterval = 1.0);

    /*! \brief RtspStreamProcessor destructor. */
    virtual ~RtspStreamProcessor();

    /*! \brief RtspStreamProcessor deleted copy constructor. */
    RtspStreamProcessor(RtspStreamProcessor const&) = delete;

    /*! \brief RtspStreamProcessor deleted copy assignment operator. */
    RtspStreamProcessor& operator=(RtspStreamProcessor const&) = delete;

    /*! \brief StartVideoWriting begins recording video to disk. */
    void StartVideoWriting() noexcept;

    /*! \brief StopVideoWriting ends recording video to disk. */
    void StopVideoWriting() noexcept;

    /*!
     * \brief GetEnableVideoWriting reports if stream is being written to disk.
     * \return True if writing, false otherwise.
     */
    bool GetEnableVideoWriting() const noexcept;

    /*!
     * \brief VideoFrameUpdated monitors stream activity.
     * \return A flag denoting if the captured videdo stream is being updated.
     */
    bool VideoFrameUpdated() const noexcept;

    /*!
     * \brief GetAspectRatioAndSize return s the aspect ratio.
     * \param[out] width - Width of video stream's frames.
     * \param[out] height - Height of video stream's frames.
     * \return A double contiaing the aspect ratio e.g. 1.333333 == 4:3.
     */
    double GetAspectRatioAndSize(int& width, int& height) const;

    /*!
     * \brief CurrentVideoFrame gives acces to current video frame.
     * \param[in] getMotionFrame - (Optional) Get motion tracking frame if available.
     * \return A QImage of the current video frame at full stream resolution.
     */
    QImage CurrentVideoFrame(bool const getMotionFrame = false) const;

    /*!
     * \brief CurrentFps gives acces to current stream FPS.
     * \return The streams FPS.
     */
    double CurrentFps() const noexcept;

private:
    virtual void ThreadIteration() noexcept;
    virtual void ProcessTerminationConditions() noexcept;
    void SetEnableVideoWriting(bool enable) noexcept;
    void CheckRecordingSchedule();
    void CreateCaptureObjects();
    void GrabVideoFrame();
    void WriteVideoFrame();
    bool CheckMotionSchedule() const;
    bool TrackMotion();
    void CheckMotionTracking();

private:
    mutable std::mutex                  m_writingMutex{};
    mutable std::mutex                  m_frameMutex{};
    mutable std::mutex                  m_fpsMutex{};
    unsigned int                        m_updatePeriodMillisecs{40};
    double                              m_fps{25.0};
    std::string                         m_name{"cam"};
    std::string                         m_completeRtspUrl{};
    std::string                         m_saveFolderPath{};
    double                              m_requiredFileDurationSecs{0.0};
    std::vector<std::vector<bool>>      m_recordingSchedule{};
    bool                                m_useRecordingSchedule{false};
    std::vector<std::vector<bool>>      m_motionSchedule{};
    bool                                m_useMotionSchedule{false};
    double                              m_motionSensitivity{0.5};
    core_lib::threads::SyncEvent        m_updateEvent{};
    bool                                m_enableVideoWriting{false};
    int                                 m_videoWidth{0};
    int                                 m_videoHeight{0};
    cv::Ptr<cv::VideoCapture>           m_videoCapture{};
    cv::Ptr<cv::Mat>                    m_videoFrame{};
    cv::Ptr<cv::Mat>                    m_refTrackingFrame{};
    cv::Ptr<cv::Mat>                    m_motionVideoFrame{};
    cv::Ptr<cv::VideoWriter>            m_videoWriter{};
    std::vector<std::vector<cv::Point>> m_contours;
    std::vector<cv::Vec4i>              m_hierarchy;
    std::vector<std::vector<cv::Point>> m_contoursPoly;
    std::vector<cv::Rect>               m_boundingRects;
    cv::Size                            m_gaussianBlurKernel{0, 0};
    cv::Scalar                          m_contourColor;
    int                                 m_numFramesInInterval{0};
    int                                 m_trackedFrameCount{0};
    double                              m_fileDurationSecs{0.0};
    bool                                m_videoFrameUpdated{false};
    time_t                              m_currentTime{};
};

} // namespace ipfreely

#endif // RTSPSTREAMPROCESSOR_H
