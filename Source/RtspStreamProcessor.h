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
#include <opencv2/core/cvstd.hpp>
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
     * \param[in] recordingSchedule - (Optional) The daily/hourly recording schedule..
     *
     * The stream processor can be used to receive and thus display RTSP video streams but can also
     * record the stream in DivX format mp4 files to disk. Files are recorded with the given
     * duration. One recording sessino can span multiple back-to-back video files.
     */
    RtspStreamProcessor(std::string const& name, std::string const& completeRtspUrl,
                        std::string const& saveFolderPath, double const requiredFileDurationSecs,
                        std::vector<std::vector<bool>> const& recordingSchedule = {});

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
     * \brief CurrentVideoFrame gives acces to current video frame.
     * \return A QImage of the current video frame at full stream resolution.
     */
    QImage CurrentVideoFrame() const;

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
    void CheckFps();
    void SetFps(double const fps) noexcept;

private:
    mutable std::mutex             m_writingMutex{};
    mutable std::mutex             m_frameMutex{};
    mutable std::mutex             m_fpsMutex{};
    unsigned int                   m_updatePeriodMillisecs{40};
    double                         m_fps{25.0};
    std::string                    m_name{"cam"};
    std::string                    m_completeRtspUrl{};
    std::string                    m_saveFolderPath{};
    double                         m_requiredFileDurationSecs{0.0};
    std::vector<std::vector<bool>> m_recordingSchedule{};
    bool                           m_useRecordingSchedule{false};
    core_lib::threads::SyncEvent   m_updateEvent{};
    bool                           m_enableVideoWriting{false};
    cv::Ptr<cv::VideoCapture>      m_videoCapture{};
    cv::Ptr<cv::Mat>               m_videoFrame{};
    cv::Ptr<cv::VideoWriter>       m_videoWriter{};
    double                         m_fileDurationSecs{0.0};
    bool                           m_videoFrameUpdated{false};
    time_t                         m_currentTime{};
};

} // namespace ipfreely

#endif // RTSPSTREAMPROCESSOR_H
