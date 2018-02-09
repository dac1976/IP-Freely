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
 * \file IpFreelyRtspStreamProcessor.h
 * \brief File containing declaration of IpFreelyRtspStreamProcessor threaded class.
 */
#ifndef IPFREELYRTSPSTREAMPROCESSOR_H
#define IPFREELYRTSPSTREAMPROCESSOR_H

#include <QImage>
#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <opencv/cv.hpp>
#include "IpFreelyCameraDatabase.h"
#include "Threads/ThreadBase.h"
#include "Threads/SyncEvent.h"

/*! \brief The ipfreely namespace. */
namespace ipfreely
{

class IpFreelyMotionDetector;

/*! \brief Class defining a RTSP stream processor thread. */
class IpFreelyRtspStreamProcessor final : public core_lib::threads::ThreadBase
{
public:
    /*!
     * \brief IpFreelyRtspStreamProcessor constructor.
     * \param[in] name - A name for the stream, used to name output video files.
     * \param[in] cameraDetails - Camera details we want to stream from.
     * \param[in] saveFolderPath - A local folder to save captured videos to.
     * \param[in] requiredFileDurationSecs - Duration to use for captured video files.
     * \param[in] recordingSchedule - (Optional) The daily/hourly recording schedule.
     * \param[in] motionSchedule - (Optional) The daily/hourly motion detector schedule.
     *
     * The stream processor can be used to receive and thus display RTSP video streams but can also
     * record the stream in DivX format mp4 files to disk. Files are recorded with the given
     * duration. One recording session can span multiple back-to-back video files.
     */
    IpFreelyRtspStreamProcessor(std::string const& name, IpCamera const& cameraDetails,
                                std::string const&                    saveFolderPath,
                                double const                          requiredFileDurationSecs,
                                std::vector<std::vector<bool>> const& recordingSchedule = {},
                                std::vector<std::vector<bool>> const& motionSchedule    = {});

    /*! \brief IpFreelyRtspStreamProcessor destructor. */
    virtual ~IpFreelyRtspStreamProcessor();

    /*! \brief IpFreelyRtspStreamProcessor deleted copy constructor. */
    IpFreelyRtspStreamProcessor(IpFreelyRtspStreamProcessor const&) = delete;

    /*! \brief IpFreelyRtspStreamProcessor deleted copy assignment operator. */
    IpFreelyRtspStreamProcessor& operator=(IpFreelyRtspStreamProcessor const&) = delete;

    /*! \brief StartVideoWriting begins recording video to disk. */
    void StartVideoWriting() noexcept;

    /*! \brief StopVideoWriting ends recording video to disk. */
    void StopVideoWriting() noexcept;

    /*!
     * \brief GetEnableVideoWriting reports if stream is being written to disk.
     * \return True if writing, false otherwise.
     */
    bool GetEnableVideoWriting() const noexcept;

    /*!c
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
     * \param[out] motionRectangle - (Optional) Used to get motion bounding rect.
     * \return A QImage of the current video frame at full stream resolution.
     */
    QImage CurrentVideoFrame(QRect* motionRectangle = nullptr) const;

    /*!
     * \brief CurrentFps gives acces to current stream FPS.
     * \return The streams FPS.
     */
    double CurrentFps() const noexcept;

private:
    static bool IsScheduleEnabled(std::vector<std::vector<bool>> const& schedule);
    static bool VerifySchedule(std::string const&                    scheduleId,
                               std::vector<std::vector<bool>> const& schedule);
    virtual void ThreadIteration() noexcept;
    virtual void ProcessTerminationConditions() noexcept;
    void SetEnableVideoWriting(bool enable) noexcept;
    void CheckRecordingSchedule();
    void CreateCaptureObjects();
    void GrabVideoFrame();
    void WriteVideoFrame();
    bool CheckMotionSchedule() const;
    void InitialiseMotionDetector();
    void CheckMotionDetector();

private:
    mutable std::mutex                      m_writingMutex{};
    mutable std::mutex                      m_frameMutex{};
    mutable std::mutex                      m_motionMutex{};
    std::string                             m_name{"cam"};
    IpCamera                                m_cameraDetails{};
    std::string                             m_saveFolderPath{};
    double                                  m_requiredFileDurationSecs{0.0};
    std::vector<std::vector<bool>>          m_recordingSchedule{};
    std::vector<std::vector<bool>>          m_motionSchedule{};
    core_lib::threads::SyncEvent            m_updateEvent{};
    unsigned int                            m_updatePeriodMillisecs{40};
    double                                  m_fps{25.0};
    bool                                    m_useRecordingSchedule{false};
    bool                                    m_useMotionSchedule{false};
    bool                                    m_enableVideoWriting{false};
    int                                     m_videoWidth{0};
    int                                     m_videoHeight{0};
    cv::VideoCapture                        m_videoCapture{};
    cv::Mat                                 m_videoFrame{};
    cv::Ptr<cv::VideoWriter>                m_videoWriter{};
    double                                  m_fileDurationSecs{0.0};
    bool                                    m_videoFrameUpdated{false};
    time_t                                  m_currentTime{};
    std::shared_ptr<IpFreelyMotionDetector> m_motionDetector;
};

} // namespace ipfreely

#endif // IPFREELYRTSPSTREAMPROCESSOR_H
