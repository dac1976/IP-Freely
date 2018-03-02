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
 * \file IpFreelyStreamProcessor.cpp
 * \brief File containing definition of IpFreelyStreamProcessor threaded class.
 */
#include "IpFreelyStreamProcessor.h"
#include <sstream>
#include <cmath>
#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
#include "IpFreelyMotionDetector.h"
#include "Threads/EventThread.h"
#include "StringUtils/StringUtils.h"
#include "DebugLog/DebugLogging.h"

namespace bfs = boost::filesystem;

namespace ipfreely
{

namespace utils
{

inline QImage CvMatToQImage(cv::Mat const& inMat)
{
    switch (inMat.type())
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image(inMat.data,
                     inMat.cols,
                     inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_ARGB32);

        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image(inMat.data,
                     inMat.cols,
                     inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_RGB888);

        return image.rgbSwapped();
    }
    // 8-bit, 1 channel
    case CV_8UC1:
    {
        QImage image(inMat.data,
                     inMat.cols,
                     inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_Grayscale8);

        return image;
    }

    default:
        DEBUG_MESSAGE_EX_ERROR("unsupported cv::Mat format");
        break;
    }

    return QImage();
}

} // namespace utils

static constexpr double MIN_FPS = 1.0;
static constexpr double MAX_FPS = 30.0;

IpFreelyStreamProcessor::IpFreelyStreamProcessor(
    std::string const& name, IpCamera const& cameraDetails, std::string const& saveFolderPath,
    double const requiredFileDurationSecs, std::vector<std::vector<bool>> const& recordingSchedule,
    std::vector<std::vector<bool>> const& motionSchedule)
    : m_name(core_lib::string_utils::RemoveIllegalChars(name))
    , m_cameraDetails(cameraDetails)
    , m_saveFolderPath(saveFolderPath)
    , m_requiredFileDurationSecs(requiredFileDurationSecs)
    , m_recordingSchedule(recordingSchedule)
    , m_motionSchedule(motionSchedule)
{
    m_useRecordingSchedule = VerifySchedule("Recording", m_recordingSchedule);
    m_useMotionSchedule    = VerifySchedule("Motion", m_motionSchedule);

    bfs::path p(m_saveFolderPath);
    p = bfs::system_complete(p);

    if (!bfs::exists(p))
    {
        if (!bfs::create_directories(p))
        {
            std::ostringstream oss;
            oss << "Failed to create directories: " << p.string();
            BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
        }
    }

    CreateVideoCapture();

    auto fps      = m_videoCapture->get(CV_CAP_PROP_FPS);
    m_originalFps = fps;

    if ((fps < MIN_FPS) || (fps > MAX_FPS))
    {
        fps = m_cameraDetails.cameraMaxFps;

        DEBUG_MESSAGE_EX_WARNING(
            "Invalid FPS obtained from stream defaulting to user preference FPS, stream URL: "
            << m_cameraDetails.streamUrl);
    }

    m_fps                   = fps;
    m_updatePeriodMillisecs = static_cast<unsigned int>(1000.0 / m_fps);

    DEBUG_MESSAGE_EX_INFO("Stream at: " << m_cameraDetails.streamUrl << " running with FPS of: "
                                        << m_fps
                                        << ", thread update period (ms): "
                                        << m_updatePeriodMillisecs);

    DEBUG_MESSAGE_EX_INFO("Creating event thread for stream URL: " << m_cameraDetails.streamUrl);

    m_eventThread = std::make_shared<core_lib::threads::EventThread>(
        std::bind(&IpFreelyStreamProcessor::ThreadEventCallback, this), m_updatePeriodMillisecs);
}

IpFreelyStreamProcessor::~IpFreelyStreamProcessor()
{
    // Do nothing.
}

void IpFreelyStreamProcessor::StartVideoWriting() noexcept
{
    if (m_useRecordingSchedule)
    {
        DEBUG_MESSAGE_EX_WARNING(
            "Manual recording disabled because a recording schedule is defined. Camera: "
            << m_name);
        return;
    }

    SetEnableVideoWriting(true);
}

void IpFreelyStreamProcessor::StopVideoWriting() noexcept
{
    if (m_useRecordingSchedule)
    {
        DEBUG_MESSAGE_EX_WARNING(
            "Manual recording disabled because a recording schedule is defined. Camera: "
            << m_name);
        return;
    }

    SetEnableVideoWriting(false);
}

bool IpFreelyStreamProcessor::EnableVideoWriting() const noexcept
{
    bool isWriting = GetEnableVideoWriting();

    if (!isWriting && m_motionDetector)
    {
        isWriting = m_motionDetector->WritingStream();
    }

    return isWriting;
}

bool IpFreelyStreamProcessor::VideoFrameUpdated() const noexcept
{
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_videoFrameUpdated;
}

double IpFreelyStreamProcessor::GetAspectRatioAndSize(int& width, int& height) const
{
    width  = m_videoWidth;
    height = m_videoHeight;
    return static_cast<double>(m_videoWidth) / static_cast<double>(m_videoHeight);
}

QImage IpFreelyStreamProcessor::CurrentVideoFrame(QRect* motionRectangle) const
{
    cv::Mat result;

    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        result = m_videoFrame;
    }

    if (motionRectangle)
    {
        std::lock_guard<std::mutex> lock(m_motionMutex);

        if (m_motionDetector)
        {
            *motionRectangle = m_motionDetector->CurrentMotionRect();
        }
    }

    return utils::CvMatToQImage(result);
}

double IpFreelyStreamProcessor::CurrentFps() const noexcept
{
    return m_fps;
}

bool IpFreelyStreamProcessor::IsScheduleEnabled(std::vector<std::vector<bool>> const& schedule)
{
    bool recordEnabled = false;

    for (auto const& day : schedule)
    {
        for (auto hour : day)
        {
            if (hour)
            {
                recordEnabled = true;
                break;
            }
        }

        if (recordEnabled)
        {
            break;
        }
    }

    return recordEnabled;
}

bool IpFreelyStreamProcessor::VerifySchedule(std::string const&                    scheduleId,
                                             std::vector<std::vector<bool>> const& schedule)
{
    bool scheduleOk = false;

    if (!schedule.empty())
    {
        if (schedule.size() != 7)
        {
            std::ostringstream oss;
            oss << "Incorrect number of days in schedule: " << scheduleId;
            BOOST_THROW_EXCEPTION(std::invalid_argument(oss.str()));
        }

        if (schedule.front().size() != 24)
        {
            std::ostringstream oss;
            oss << "Incorrect number of hours in schedule: " << scheduleId;
            BOOST_THROW_EXCEPTION(std::invalid_argument(oss.str()));
        }

        scheduleOk = IsScheduleEnabled(schedule);
    }

    if (scheduleOk)
    {
        DEBUG_MESSAGE_EX_INFO(scheduleId << " is enabled");
    }
    else
    {
        DEBUG_MESSAGE_EX_WARNING(
            scheduleId
            << " is disabled. Either no days/hours are set or scheduled recording is disabled.");
    }

    return scheduleOk;
}

void IpFreelyStreamProcessor::ThreadEventCallback() noexcept
{
    // Get current time stamp.
    m_currentTime = time(0);

    try
    {
        GrabVideoFrame();
        CheckRecordingSchedule();
        CheckMotionDetector();
        CreateCaptureObjects();
        WriteVideoFrame();
        CheckFps();
    }
    catch (...)
    {
        auto exceptionMsg = boost::current_exception_diagnostic_information();
        DEBUG_MESSAGE_EX_ERROR(exceptionMsg);
    }
}

void IpFreelyStreamProcessor::SetEnableVideoWriting(bool enable) noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    m_enableVideoWriting = enable;
}

bool IpFreelyStreamProcessor::GetEnableVideoWriting() const noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    return m_enableVideoWriting;
}

void IpFreelyStreamProcessor::CheckRecordingSchedule()
{
    if (m_recordingSchedule.empty())
    {
        return;
    }

    auto localTime = std::localtime(&m_currentTime);

    bool needToRecord = (m_recordingSchedule[static_cast<size_t>(
        localTime->tm_wday)])[static_cast<size_t>(localTime->tm_hour)];

    std::lock_guard<std::mutex> lock(m_writingMutex);

    if (m_enableVideoWriting)
    {
        if (!needToRecord)
        {
            m_enableVideoWriting = false;
        }
    }
    else
    {
        if (needToRecord)
        {
            m_enableVideoWriting = true;
        }
    }
}

void IpFreelyStreamProcessor::CreateCaptureObjects()
{
    if (GetEnableVideoWriting())
    {
        if (m_videoWriter)
        {
            if (m_fileDurationSecs < m_requiredFileDurationSecs)
            {
                return;
            }

            m_videoWriter.release();
        }

        m_fileDurationSecs = 0.0;

        auto localTime = std::localtime(&m_currentTime);
        char folderName[9];
        std::strftime(folderName, sizeof(folderName), "%Y%m%d", localTime);

        bfs::path p(m_saveFolderPath);
        p /= folderName;
        p = bfs::system_complete(p);

        if (!bfs::exists(p))
        {
            if (!bfs::create_directories(p))
            {
                DEBUG_MESSAGE_EX_ERROR("Failed to create directories: " << p.string());
                return;
            }
        }

        std::ostringstream oss;
        oss << m_name << "_" << m_currentTime << ".avi";

        p /= oss.str();

        DEBUG_MESSAGE_EX_INFO(
            "Creating new output video file: " << p.string() << ", FPS: " << m_fps);

#if BOOST_OS_WINDOWS
        m_videoWriter = cv::makePtr<cv::VideoWriter>(p.string().c_str(),
                                                     cv::VideoWriter::fourcc('D', 'I', 'V', 'X'),
                                                     m_fps,
                                                     cv::Size(m_videoWidth, m_videoHeight));
#else
        m_videoWriter = cv::makePtr<cv::VideoWriter>(p.string().c_str(),
                                                     cv::VideoWriter::fourcc('X', 'V', 'I', 'D'),
                                                     m_fps,
                                                     cv::Size(m_videoWidth, m_videoHeight));
#endif

        if (!m_videoWriter->isOpened())
        {
            m_videoWriter.release();
            DEBUG_MESSAGE_EX_ERROR("Failed to open VideoWriter object for: " << p.string());
            return;
        }
    }
    else
    {
        if (m_videoWriter)
        {
            DEBUG_MESSAGE_EX_INFO(
                "Video writing disabled, releasing video writer, camera: " << m_name);
            m_videoWriter.release();
        }
    }
}

void IpFreelyStreamProcessor::GrabVideoFrame()
{
    std::lock_guard<std::mutex> lock(m_frameMutex);
    *m_videoCapture >> m_videoFrame;
    m_videoFrameUpdated = true;
}

void IpFreelyStreamProcessor::WriteVideoFrame()
{
    if (m_videoWriter)
    {
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            *m_videoWriter << m_videoFrame;
        }

        m_fileDurationSecs += static_cast<double>(m_updatePeriodMillisecs) / 1000.0;
    }
}

bool IpFreelyStreamProcessor::CheckMotionSchedule() const
{
    if (!m_useMotionSchedule || m_motionSchedule.empty())
    {
        return false;
    }

    auto localTime = std::localtime(&m_currentTime);

    return (m_motionSchedule[static_cast<size_t>(localTime->tm_wday)])[static_cast<size_t>(
        localTime->tm_hour)];
}

void IpFreelyStreamProcessor::InitialiseMotionDetector()
{
    if (!m_motionDetector)
    {
        m_motionDetector = std::make_shared<IpFreelyMotionDetector>(m_name,
                                                                    m_cameraDetails,
                                                                    m_saveFolderPath,
                                                                    m_requiredFileDurationSecs,
                                                                    m_fps,
                                                                    m_videoWidth,
                                                                    m_videoHeight);
    }
}

void IpFreelyStreamProcessor::CheckMotionDetector()
{
    bool enableMotionDetector = CheckMotionSchedule();

    std::lock_guard<std::mutex> lockM(m_motionMutex);

    if (!enableMotionDetector)
    {
        m_motionDetector.reset();
        return;
    }

    InitialiseMotionDetector();

    std::lock_guard<std::mutex> lockF(m_frameMutex);
    m_motionDetector->AddNextFrame(m_videoFrame);
}

void IpFreelyStreamProcessor::CreateVideoCapture()
{
    if (m_videoCapture)
    {
        m_videoCapture.release();
    }

    bool isId;
    auto comleteStreamUrl = m_cameraDetails.CompleteStreamUrl(isId);

    if (isId)
    {
        m_videoCapture = cv::makePtr<cv::VideoCapture>(std::stoi(comleteStreamUrl));
    }
    else
    {
        m_videoCapture = cv::makePtr<cv::VideoCapture>(comleteStreamUrl.c_str());
    }

    if (!m_videoCapture->isOpened())
    {
        std::ostringstream oss;
        oss << "Failed to open VideoCapture object, url: " << m_cameraDetails.streamUrl;
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    m_videoWidth  = static_cast<int>(m_videoCapture->get(CV_CAP_PROP_FRAME_WIDTH));
    m_videoHeight = static_cast<int>(m_videoCapture->get(CV_CAP_PROP_FRAME_HEIGHT));
}

void IpFreelyStreamProcessor::CheckFps()
{
    auto fps = m_videoCapture->get(CV_CAP_PROP_FPS);

    if (std::abs(fps - m_originalFps) > 0.1)
    {
        m_originalFps = fps;
        DEBUG_MESSAGE_EX_WARNING(
            "Detected change in FPS for stream: " << m_cameraDetails.streamUrl);

        if ((fps < MIN_FPS) || (fps > MAX_FPS))
        {
            fps = m_cameraDetails.cameraMaxFps;

            DEBUG_MESSAGE_EX_WARNING(
                "Invalid FPS obtained from stream defaulting to user preference FPS, stream URL: "
                << m_cameraDetails.streamUrl);
        }

        // If the FPS has changed then recreate the video capture object.
        CreateVideoCapture();

        m_fps                   = fps;
        m_updatePeriodMillisecs = static_cast<unsigned int>(1000.0 / m_fps);

        DEBUG_MESSAGE_EX_INFO("Stream at: " << m_cameraDetails.streamUrl << " running with FPS of: "
                                            << m_fps
                                            << ", thread update period (ms): "
                                            << m_updatePeriodMillisecs);

        if (m_videoWriter)
        {
            DEBUG_MESSAGE_EX_INFO("Releasing video writer due to FPS change, stream URL: "
                                  << m_cameraDetails.streamUrl);
            m_videoWriter.release();
        }

        if (m_motionDetector)
        {
            DEBUG_MESSAGE_EX_INFO("Recreating motion detector with new FPS, stream URL: "
                                  << m_cameraDetails.streamUrl);
            m_motionDetector.reset();
            m_motionDetector = std::make_shared<IpFreelyMotionDetector>(m_name,
                                                                        m_cameraDetails,
                                                                        m_saveFolderPath,
                                                                        m_requiredFileDurationSecs,
                                                                        m_fps,
                                                                        m_videoWidth,
                                                                        m_videoHeight);
        }

        DEBUG_MESSAGE_EX_INFO(
            "Recreating event thread for stream URL: " << m_cameraDetails.streamUrl);

        m_eventThread.reset();
        m_eventThread = std::make_shared<core_lib::threads::EventThread>(
            std::bind(&IpFreelyStreamProcessor::ThreadEventCallback, this),
            m_updatePeriodMillisecs);
    }
}

} // namespace ipfreely
