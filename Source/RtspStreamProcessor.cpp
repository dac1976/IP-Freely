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
 * \file RtspStreamProcessor.cpp
 * \brief File containing definition of RtspStreamProcessor threaded class.
 */
#include "RtspStreamProcessor.h"
#include <sstream>
#include <chrono>
#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image(inMat.data,
                     inMat.cols,
                     inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_Grayscale8);
#else
        static QVector<QRgb> sColorTable;

        // only create our color table the first time
        if (sColorTable.isEmpty())
        {
            sColorTable.resize(256);

            for (int i = 0; i < 256; ++i)
            {
                sColorTable[i] = qRgb(i, i, i);
            }
        }

        QImage image(inMat.data,
                     inMat.cols,
                     inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_Indexed8);

        image.setColorTable(sColorTable);
#endif

        return image;
    }

    default:
        DEBUG_MESSAGE_EX_ERROR("unsupported cv::Mat format");
        break;
    }

    return QImage();
}

} // namespace utils

RtspStreamProcessor::RtspStreamProcessor(
    std::string const& name, std::string const& completeRtspUrl, std::string const& saveFolderPath,
    double const requiredFileDurationSecs, std::vector<std::vector<bool>> const& recordingSchedule,
    std::vector<std::vector<bool>> const& motionSchedule, double const motionSensitivity,
    double const motionTrackInterval)
    : ThreadBase()
    , m_name(core_lib::string_utils::RemoveIllegalChars(name))
    , m_completeRtspUrl(completeRtspUrl)
    , m_saveFolderPath(saveFolderPath)
    , m_requiredFileDurationSecs(requiredFileDurationSecs)
    , m_recordingSchedule(recordingSchedule)
    , m_motionSchedule(motionSchedule)
    , m_motionSensitivity(motionSensitivity)
    , m_videoCapture(cv::makePtr<cv::VideoCapture>(completeRtspUrl.c_str()))
    , m_videoFrame(cv::makePtr<cv::Mat>())
{
    if (!m_recordingSchedule.empty())
    {
        if (m_recordingSchedule.size() != 7)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of days in schedule."));
        }

        if (m_recordingSchedule.front().size() != 24)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of hours in schedule."));
        }

        m_useRecordingSchedule = true;
        DEBUG_MESSAGE_EX_INFO("Recording schedule enabled.");
    }

    if (!m_motionSchedule.empty())
    {
        if (m_motionSchedule.size() != 7)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of days in schedule."));
        }

        if (m_motionSchedule.front().size() != 24)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of hours in schedule."));
        }

        if ((m_motionSensitivity > 0.0) && (m_motionSensitivity <= 1.0))
        {
            m_useMotionSchedule = true;
            DEBUG_MESSAGE_EX_INFO("Motion tracking schedule enabled.");
        }
        else
        {
            DEBUG_MESSAGE_EX_INFO("Motion tracking sensitivity invalid, tracking disabled.");
        }
    }

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

    // Give stream a chance to have properly connected.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (!m_videoCapture->isOpened())
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open VideoCapture object."));
    }

    auto     frameType = static_cast<int>(m_videoCapture->get(cv::CAP_PROP_FORMAT));
    cv::Size videoSize = cv::Size(static_cast<int>(m_videoCapture->get(CV_CAP_PROP_FRAME_WIDTH)),
                                  static_cast<int>(m_videoCapture->get(CV_CAP_PROP_FRAME_HEIGHT)));
    m_videoWidth       = videoSize.width;
    m_videoHeight      = videoSize.height;
    m_fps              = m_videoCapture->get(CV_CAP_PROP_FPS);
    m_updatePeriodMillisecs = static_cast<unsigned int>(1000.0 / m_fps);

    if (m_useMotionSchedule)
    {
        m_greyFrame =
            cv::makePtr<cv::Mat>(cv::Size(static_cast<int>(m_videoWidth * motionSensitivity),
                                          static_cast<int>(m_videoHeight * motionSensitivity)),
                                 frameType);

        m_lastFrame           = cv::makePtr<cv::Mat>();
        m_deltaFrame          = cv::makePtr<cv::Mat>();
        m_numFramesInInterval = static_cast<int>(motionTrackInterval * m_fps);
    }

    DEBUG_MESSAGE_EX_INFO("Stream at: " << m_completeRtspUrl << " running with FPS of: " << m_fps
                                        << ", thread update period (ms): "
                                        << m_updatePeriodMillisecs);
}

RtspStreamProcessor::~RtspStreamProcessor()
{
    Stop();
}

void RtspStreamProcessor::StartVideoWriting() noexcept
{
    if (m_useRecordingSchedule)
    {
        DEBUG_MESSAGE_EX_WARNING(
            "Manual recording disabled because a schedule is defined. Camera: " << m_name);
        return;
    }

    SetEnableVideoWriting(true);
}

void RtspStreamProcessor::StopVideoWriting() noexcept
{
    if (m_useRecordingSchedule)
    {
        DEBUG_MESSAGE_EX_WARNING(
            "Manual recording disabled because a schedule is defined. Camera: " << m_name);
        return;
    }

    SetEnableVideoWriting(false);
}

bool RtspStreamProcessor::GetEnableVideoWriting() const noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    return m_enableVideoWriting;
}

bool RtspStreamProcessor::VideoFrameUpdated() const noexcept
{
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_videoFrameUpdated;
}

double RtspStreamProcessor::GetAspectRatioAndSize(int& width, int& height) const
{
    width  = m_videoWidth;
    height = m_videoHeight;
    return static_cast<double>(m_videoWidth) / static_cast<double>(m_videoHeight);
}

QImage RtspStreamProcessor::CurrentVideoFrame(bool const getMotionFrame) const
{
    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (getMotionFrame && m_motionVideoFrame)
    {
        return utils::CvMatToQImage(*m_motionVideoFrame);
    }
    else
    {
        return utils::CvMatToQImage(*m_videoFrame);
    }
}

double RtspStreamProcessor::CurrentFps() const noexcept
{
    std::lock_guard<std::mutex> lock(m_fpsMutex);
    return m_fps;
}

void RtspStreamProcessor::ThreadIteration() noexcept
{
    if (m_updateEvent.WaitForTime(m_updatePeriodMillisecs))
    {
        return;
    }

    // Get current time stamp.
    m_currentTime = time(0);

    try
    {
        CheckRecordingSchedule();
        CreateCaptureObjects();
        GrabVideoFrame();
        WriteVideoFrame();
        CheckMotionTracking();
    }
    catch (...)
    {
        auto exceptionMsg = boost::current_exception_diagnostic_information();
        DEBUG_MESSAGE_EX_ERROR(exceptionMsg);
    }
}

void RtspStreamProcessor::ProcessTerminationConditions() noexcept
{
    m_updateEvent.Signal();
}

void RtspStreamProcessor::SetEnableVideoWriting(bool enable) noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    m_enableVideoWriting = enable;
}

void RtspStreamProcessor::CheckRecordingSchedule()
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

void RtspStreamProcessor::CreateCaptureObjects()
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

        std::ostringstream oss;
        oss << m_name << "_" << m_currentTime << ".mp4";
        bfs::path p(m_saveFolderPath);
        p /= oss.str();
        p = bfs::system_complete(p);

        DEBUG_MESSAGE_EX_INFO("Creating new output video file: " << p.string());

        m_videoWriter = cv::makePtr<cv::VideoWriter>(p.string().c_str(),
                                                     cv::VideoWriter::fourcc('D', 'I', 'V', 'X'),
                                                     m_fps,
                                                     cv::Size(m_videoWidth, m_videoHeight));

        if (!m_videoWriter->isOpened())
        {
            m_videoWriter.release();
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open VideoWriter object"));
        }

        m_fileDurationSecs = 0.0;
    }
    else
    {
        m_videoWriter.release();
    }
}

void RtspStreamProcessor::GrabVideoFrame()
{
    std::lock_guard<std::mutex> lock(m_frameMutex);
    *m_videoCapture >> *m_videoFrame;
    m_videoFrameUpdated = true;
}

void RtspStreamProcessor::WriteVideoFrame()
{
    if (m_videoWriter)
    {
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            *m_videoWriter << *m_videoFrame;
        }

        m_fileDurationSecs += static_cast<double>(m_updatePeriodMillisecs) / 1000.0;
    }
}

bool RtspStreamProcessor::CheckMotionSchedule() const
{
    if (!m_useMotionSchedule || m_motionSchedule.empty())
    {
        return false;
    }

    auto localTime = std::localtime(&m_currentTime);

    return (m_motionSchedule[static_cast<size_t>(localTime->tm_wday)])[static_cast<size_t>(
        localTime->tm_hour)];
}

void RtspStreamProcessor::TrackMotion()
{
    // Scale down image according to sensitivty setting.
    cv::resize(*m_videoFrame, *m_greyFrame, m_greyFrame->size(), 0.0, 0.0, cv::INTER_AREA);

    // Convert to greyscale.
    cv::cvtColor(*m_greyFrame, *m_greyFrame, CV_BGR2GRAY);

    // Add Gaussian blur.
    cv::GaussianBlur(*m_greyFrame, *m_greyFrame, cv::Size(21, 21), 0);

    if (m_trackedFrameCount == 0)
    {
        ++m_trackedFrameCount;

        // initialize the last reference frame
        *m_lastFrame = *m_greyFrame;

        return;
    }
    else if (m_trackedFrameCount == m_numFramesInInterval)
    {
        m_trackedFrameCount = 0;
    }

    ++m_trackedFrameCount;

    // Create difference frame.
    cv::absdiff(*m_lastFrame, *m_greyFrame, *m_deltaFrame);
    cv::threshold(*m_deltaFrame, *m_deltaFrame, 50, 255, cv::THRESH_BINARY);

    // Dilate to fill-in holes and find contours.
    int iterations = 2;
    cv::dilate(*m_deltaFrame, *m_deltaFrame, cv::Mat(), cv::Point(-1, -1), iterations);

    // Approximate contours to polygons
    cv::findContours(*m_deltaFrame,
                     m_contours,
                     m_hierarchy,
                     CV_RETR_TREE,
                     CV_CHAIN_APPROX_SIMPLE,
                     cv::Point(0, 0));

    // Resize polygons to original frame's scale.
    for (auto& c : m_contours)
    {
        for (auto& p : c)
        {
            p.x = static_cast<int>(static_cast<double>(p.x) / m_motionSensitivity);
            p.y = static_cast<int>(static_cast<double>(p.y) / m_motionSensitivity);
        }
    }

    m_contoursPoly.resize(m_contours.size());

    for (size_t i = 0; i < m_contours.size(); i++)
    {
        cv::approxPolyDP(cv::Mat(m_contours[i]), m_contoursPoly[i], 3, true);
    }

    // Create result frame if needed.
    std::lock_guard<std::mutex> lock(m_frameMutex);

    if (!m_motionVideoFrame)
    {
        m_motionVideoFrame = cv::makePtr<cv::Mat>(*m_videoFrame);
    }

    // Draw polygonal contour on result frame.
    cv::Scalar brightGreenColour = cv::Scalar(0, 255, 0);

    for (size_t i = 0; i < m_contours.size(); i++)
    {
        cv::drawContours(*m_motionVideoFrame,
                         m_contoursPoly,
                         static_cast<int>(i),
                         brightGreenColour,
                         2,
                         8,
                         {},
                         0,
                         {});
    }
}

void RtspStreamProcessor::CheckMotionTracking()
{
    if (!CheckMotionSchedule())
    {
        m_trackedFrameCount = 0;
        return;
    }

    TrackMotion();

    // TODO: if motion detected then trigger screen capture/writing of video
    // around motion, plus email alert.
}

} // namespace ipfreely
