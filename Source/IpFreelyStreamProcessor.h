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
 * \file IpFreelyStreamProcessor.h
 * \brief File containing declaration of IpFreelyStreamProcessor threaded class.
 */
#ifndef IPFREELYSTREAMPROCESSOR_H
#define IPFREELYSTREAMPROCESSOR_H

#include <QImage>
#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "IpFreelyCameraDatabase.h"

namespace core_lib
{
namespace threads
{

class EventThread;

} // namespace threads
} // namespace core_lib

/*! \brief The ipfreely namespace. */
namespace ipfreely
{

class IpFreelyMotionDetector;

/*! \brief Class defining a RTSP stream processor. */
class IpFreelyStreamProcessor final
{
public:
    /*!
     * \brief IpFreelyStreamProcessor constructor.
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
    IpFreelyStreamProcessor(std::string const& name, IpCamera const& cameraDetails,
                            std::string const&                    saveFolderPath,
                            double const                          requiredFileDurationSecs,
                            std::vector<std::vector<bool>> const& recordingSchedule = {},
                            std::vector<std::vector<bool>> const& motionSchedule    = {});

    /*! \brief IpFreelyStreamProcessor destructor. */
    ~IpFreelyStreamProcessor() = default;

    /*! \brief IpFreelyStreamProcessor deleted copy constructor. */
    IpFreelyStreamProcessor(IpFreelyStreamProcessor const&) = delete;

    /*! \brief IpFreelyStreamProcessor deleted copy assignment operator. */
    IpFreelyStreamProcessor& operator=(IpFreelyStreamProcessor const&) = delete;

    /*! \brief StartVideoWriting begins recording video to disk. */
    void StartVideoWriting() noexcept;

    /*! \brief StopVideoWriting ends recording video to disk. */
    void StopVideoWriting() noexcept;

    /*!
     * \brief VideoWritingEnabled reports if stream is being written to disk.
     * \return True if writing, false otherwise.
     */
    bool VideoWritingEnabled() const noexcept;

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
     * \brief OriginalFps gives acces to camera stream's reported FPS.
     * \return The stream's reported FPS.
     */
    double OriginalFps() const noexcept;

    /*!
     * \brief CurrentFps gives acces to current stream's recording FPS.
     * \return The stream's recording FPS.
     */
    double CurrentFps() const noexcept;

private:
    static bool IsScheduleEnabled(std::vector<std::vector<bool>> const& schedule);
    static bool VerifySchedule(std::string const&                    scheduleId,
                               std::vector<std::vector<bool>> const& schedule);
    void        ThreadEventCallback() noexcept;
    void        SetEnableVideoWriting(bool enable) noexcept;
    bool        GetEnableVideoWriting() const noexcept;
    void        CheckRecordingSchedule();
    void        CreateCaptureObjects();
    void        GrabVideoFrame();
    void        WriteVideoFrame();
    bool        CheckMotionSchedule() const;
    void        InitialiseMotionDetector();
    void        CheckMotionDetector();
    void        CreateVideoCapture();
    bool        ComputeFps();
    void        CheckFps();

private:
    mutable std::mutex                              m_writingMutex{};
    mutable std::mutex                              m_frameMutex{};
    mutable std::mutex                              m_motionMutex{};
    std::string                                     m_name{"cam"};
    IpCamera                                        m_cameraDetails{};
    std::string                                     m_saveFolderPath{};
    double                                          m_requiredFileDurationSecs{0.0};
    std::vector<std::vector<bool>>                  m_recordingSchedule{};
    std::vector<std::vector<bool>>                  m_motionSchedule{};
    unsigned int                                    m_updatePeriodMillisecs{0};
    double                                          m_originalFps{0.0};
    double                                          m_fps{0.0};
    bool                                            m_useRecordingSchedule{false};
    bool                                            m_useMotionSchedule{false};
    bool                                            m_enableVideoWriting{false};
    int                                             m_videoWidth{0};
    int                                             m_videoHeight{0};
    cv::Ptr<cv::VideoCapture>                       m_videoCapture{};
    cv::Mat                                         m_videoFrame{};
    QImage                                          m_currentFrame{};
    QRect                                           m_motionRectangle{};
    cv::Ptr<cv::VideoWriter>                        m_videoWriter{};
    double                                          m_fileDurationSecs{0.0};
    bool                                            m_videoFrameUpdated{false};
    time_t                                          m_currentTime{};
    std::shared_ptr<IpFreelyMotionDetector>         m_motionDetector;
    std::shared_ptr<core_lib::threads::EventThread> m_eventThread;
};

} // namespace ipfreely

#endif // IPFREELYSTREAMPROCESSOR_H
