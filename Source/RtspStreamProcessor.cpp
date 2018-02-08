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

static constexpr double DIFF_THRESHOLD                  = 50.0;
static constexpr double DIFF_MAX_VALUE                  = 255.0;
static constexpr int    IDEAL_FRAME_HEIGHT              = 600;
static constexpr int    CONTOUR_LINE_THICKNESS          = 2;
static constexpr double LOW_SENSITIVITY_STDDEV          = 10.0;
static constexpr double MEDIUM_SENSITIVITY_STDDEV       = 20.0;
static constexpr double HIGH_SENSITIVITY_STDDEV         = 40.0;
static constexpr double LOW_SENSITIVITY_AREA_PERCENT    = 0.05;
static constexpr double MEDIUM_SENSITIVITY_AREA_PERCENT = 0.025;
static constexpr double HIGH_SENSITIVITY_AREA_PERCENT   = 0.01;
static constexpr double BOUNDING_RECT_SMOOTHING_FACTOR  = 0.1;

RtspStreamProcessor::RtspStreamProcessor(
    std::string const& name, std::string const& completeRtspUrl, std::string const& saveFolderPath,
    double const requiredFileDurationSecs, std::vector<std::vector<bool>> const& recordingSchedule,
    std::vector<std::vector<bool>> const& motionSchedule,
    eMotionDetectorMode const motionSensitivity, bool const shrinkFramesForMotionDetection)
    : ThreadBase()
    , m_name(core_lib::string_utils::RemoveIllegalChars(name))
    , m_completeRtspUrl(completeRtspUrl)
    , m_saveFolderPath(saveFolderPath)
    , m_requiredFileDurationSecs(requiredFileDurationSecs)
    , m_recordingSchedule(recordingSchedule)
    , m_motionSchedule(motionSchedule)
    , m_shrinkFramesForMotionDetection(shrinkFramesForMotionDetection)
    , m_videoCapture(completeRtspUrl.c_str())
    , m_erosionKernel(cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)))
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

    if (!m_motionSchedule.empty() && (motionSensitivity != eMotionDetectorMode::off))
    {
        if (m_motionSchedule.size() != 7)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of days in schedule."));
        }

        if (m_motionSchedule.front().size() != 24)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of hours in schedule."));
        }

        m_useMotionSchedule = true;
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

    if (!m_videoCapture.isOpened())
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open VideoCapture object."));
    }

    m_videoWidth  = static_cast<int>(m_videoCapture.get(CV_CAP_PROP_FRAME_WIDTH));
    m_videoHeight = static_cast<int>(m_videoCapture.get(CV_CAP_PROP_FRAME_HEIGHT));
    m_fps         = m_videoCapture.get(CV_CAP_PROP_FPS);

    if (m_fps < 0.1)
    {
        m_fps = 25.0;
    }

    m_updatePeriodMillisecs = static_cast<unsigned int>(1000.0 / m_fps);

    if (m_useMotionSchedule)
    {
        InitialiseMotionDetector(motionSensitivity);
    }
    else
    {
        DEBUG_MESSAGE_EX_INFO("Motion tracking disabled for camera: " << name);
    }

    DEBUG_MESSAGE_EX_INFO("Stream at: " << m_completeRtspUrl << " running with FPS of: " << m_fps
                                        << ", thread update period (ms): "
                                        << m_updatePeriodMillisecs);

    Start();
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

QImage RtspStreamProcessor::CurrentVideoFrame(QRect* motionRectangle) const
{
    cv::Mat result;

    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        result = m_videoFrame;
    }

    if (motionRectangle)
    {
        std::lock_guard<std::mutex> lock(m_motionMutex);
        *motionRectangle = QRect(m_motionBoundingRect.tl().x,
                                 m_motionBoundingRect.tl().y,
                                 m_motionBoundingRect.width,
                                 m_motionBoundingRect.height);
    }

    /*if (getMotionFrame && (boundingRect.area() > 0))
    {
        cv::rectangle(result,
                      m_motionBoundingRect.tl(),
                      m_motionBoundingRect.br(),
                      m_rectangleColor,
                      CONTOUR_LINE_THICKNESS,
                      cv::LINE_8);

        return utils::CvMatToQImage(result);
    }
    else
    {
        return utils::CvMatToQImage(result);
    }*/

    return utils::CvMatToQImage(result);
}

double RtspStreamProcessor::CurrentFps() const noexcept
{
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
        GrabVideoFrame();
        CheckRecordingSchedule();
        CheckMotionDetector();
        CreateCaptureObjects();
        WriteVideoFrame();
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
    m_videoCapture >> m_videoFrame;
    m_videoFrameUpdated = true;
}

void RtspStreamProcessor::WriteVideoFrame()
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

void RtspStreamProcessor::InitialiseMotionDetector(eMotionDetectorMode const motionSensitivity)
{
#if defined(MOTION_DETECTOR_DEBUG)
    cv::namedWindow("motion");
#endif

    if (m_shrinkFramesForMotionDetection && (m_videoHeight > IDEAL_FRAME_HEIGHT))
    {
        m_motionFrameScalar =
            static_cast<double>(IDEAL_FRAME_HEIGHT) / static_cast<double>(m_videoHeight);

        DEBUG_MESSAGE_EX_INFO("Shrinking video frames for motion detection.");
    }
    else
    {
        DEBUG_MESSAGE_EX_INFO("Full-size video frames for motion detection.");
    }

    double const motionFrameArea = static_cast<double>(m_videoHeight * m_videoWidth) *
                                   m_motionFrameScalar * m_motionFrameScalar;

    // m_maxImageDeviation: Lower values means less sensitivity to movement.
    //                      Greater values means more sensitive to movement.
    //
    // m_minImageChangeArea: Lower values mean smaller areas of motion trigger detection.
    //                       Greater values mean larger areas of motion are required to trigger
    //                       detection.
    switch (motionSensitivity)
    {
    case eMotionDetectorMode::lowSensitivity:
        m_maxImageDeviation  = LOW_SENSITIVITY_STDDEV;
        m_minImageChangeArea = static_cast<int>(motionFrameArea * LOW_SENSITIVITY_AREA_PERCENT);
        DEBUG_MESSAGE_EX_INFO("Motion tracking (low sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::mediumSensitivity:
        m_maxImageDeviation  = MEDIUM_SENSITIVITY_STDDEV;
        m_minImageChangeArea = static_cast<int>(motionFrameArea * MEDIUM_SENSITIVITY_AREA_PERCENT);
        DEBUG_MESSAGE_EX_INFO(
            "Motion tracking (medium sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::highSensitivity:
        m_maxImageDeviation  = HIGH_SENSITIVITY_STDDEV;
        m_minImageChangeArea = static_cast<int>(motionFrameArea * HIGH_SENSITIVITY_AREA_PERCENT);
        DEBUG_MESSAGE_EX_INFO("Motion tracking (high sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::off:
        // Do nothing - but remove compiler warning
        return;
    }

    m_videoCapture >> m_prevGreyFrame;
    m_videoCapture >> m_currentGreyFrame;

    if (m_shrinkFramesForMotionDetection)
    {
        cv::resize(m_prevGreyFrame,
                   m_prevGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }

    cv::cvtColor(m_prevGreyFrame, m_prevGreyFrame, CV_BGR2GRAY);

    if (m_shrinkFramesForMotionDetection)
    {
        cv::resize(m_currentGreyFrame,
                   m_currentGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }

    cv::cvtColor(m_currentGreyFrame, m_currentGreyFrame, CV_BGR2GRAY);
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

bool RtspStreamProcessor::DetectMotion()
{
    // This algorithm is based on an example given here:
    // https://github.com/cedricve/motion-detection

    // Our return flag.
    bool motionDetected = false;

    // Calculate differences between the images and do AND-operation
    // then threshold image, low differences are ignored (ex. contrast
    // change due to sunlight).
    cv::Mat diff1, diff2, motion;
    cv::absdiff(m_prevGreyFrame, m_nextGreyFrame, diff1);
    cv::absdiff(m_nextGreyFrame, m_currentGreyFrame, diff2);
    cv::bitwise_and(diff1, diff2, motion);
    cv::threshold(motion, motion, DIFF_THRESHOLD, DIFF_MAX_VALUE, CV_THRESH_BINARY);
    cv::erode(motion, motion, m_erosionKernel);

    // Now work out the std dev of the motion frame.
    cv::Scalar mean, stddev;
    cv::meanStdDev(motion, mean, stddev);

    // Initialise motion bounding rectangle variables.
    cv::Rect maxBoundingRect;
    int      min_x = motion.cols;
    int      max_x = 0;
    int      min_y = motion.rows;
    int      max_y = 0;

    // This check guards against there being too much motion all at once,
    // e.g. changes related to rain, snow, sunlight flares etc.
    if (stddev[0] < m_maxImageDeviation)
    {
        size_t numChanges = 0;

        // Loop over image and detect changes. This is much better
        // for CPU performance compared to using OpenCV's contour
        // fitting algorithms.
        for (int j = 0; j < motion.rows; j += 2)
        {
            for (int i = 0; i < motion.cols; i += 2)
            {
                // check if at pixel (j,i) intensity is equal to 255
                // this means that the pixel is different in the sequence
                // of images (prev_frame, current_frame, next_frame)
                if (static_cast<int>(motion.at<uchar>(j, i)) == 255)
                {
                    ++numChanges;

                    // Track the boundary of the motion related changes.
                    if (min_x > i)
                    {
                        min_x = i;
                    }

                    if (max_x < i)
                    {
                        max_x = i;
                    }

                    if (min_y > j)
                    {
                        min_y = j;
                    }

                    if (max_y < j)
                    {
                        max_y = j;
                    }
                }
            }
        }

        // If we have some changes create a bounding rectangle
        // that encompasses all the motion detected.
        if (numChanges > 0)
        {
            if (min_x - 10 > 0)
            {
                min_x -= 10;
            }

            if (min_y - 10 > 0)
            {
                min_y -= 10;
            }

            if (max_x + 10 < motion.cols - 1)
            {
                max_x += 10;
            }

            if (max_y + 10 < motion.rows - 1)
            {
                max_y += 10;
            }

            cv::Point x(min_x, min_y);
            cv::Point y(max_x, max_y);
            maxBoundingRect = cv::Rect(x, y);
        }

#if defined(MOTION_DETECTOR_DEBUG)
        // Draw bounding rectangle on motion frame.
        cv::rectangle(motion,
                      maxBoundingRect.tl(),
                      maxBoundingRect.br(),
                      cv::Scalar(255, 255, 255),
                      CONTOUR_LINE_THICKNESS,
                      cv::LINE_8);
#endif
    }

#if defined(MOTION_DETECTOR_DEBUG)
    imshow("motion", motion);
#endif

    // Is the area of motion larger than our threshold. This means
    // we ignore small, most likely insignificnt motion.
    if (maxBoundingRect.area() > m_minImageChangeArea)
    {
        // Flag that we have motion.
        motionDetected = true;

        // Create a motin bounding rectangle sclaed to original
        // video frame's size.
        cv::Point tl1(static_cast<int>(static_cast<double>(min_x) / m_motionFrameScalar),
                      static_cast<int>(static_cast<double>(min_y) / m_motionFrameScalar));
        cv::Point br1(static_cast<int>(static_cast<double>(max_x) / m_motionFrameScalar),
                      static_cast<int>(static_cast<double>(max_y) / m_motionFrameScalar));

        auto minBoundingRect = cv::Rect(tl1, br1);

        // To make the bounding rectangle appear less jerky we'll
        // combine it with the previous bounding rectangle used a
        // smoothed rolling average controlled by the smoothing factor.
        std::lock_guard<std::mutex> lock(m_motionMutex);

        double left =
            (static_cast<double>(m_motionBoundingRect.tl().x) * BOUNDING_RECT_SMOOTHING_FACTOR) +
            (static_cast<double>(minBoundingRect.tl().x) * (1.0 - BOUNDING_RECT_SMOOTHING_FACTOR));
        double top =
            (static_cast<double>(m_motionBoundingRect.tl().y) * BOUNDING_RECT_SMOOTHING_FACTOR) +
            (static_cast<double>(minBoundingRect.tl().y) * (1.0 - BOUNDING_RECT_SMOOTHING_FACTOR));
        double right =
            (static_cast<double>(m_motionBoundingRect.br().x) * BOUNDING_RECT_SMOOTHING_FACTOR) +
            (static_cast<double>(minBoundingRect.br().x) * (1.0 - BOUNDING_RECT_SMOOTHING_FACTOR));
        double bottom =
            (static_cast<double>(m_motionBoundingRect.br().y) * BOUNDING_RECT_SMOOTHING_FACTOR) +
            (static_cast<double>(minBoundingRect.br().y) * (1.0 - BOUNDING_RECT_SMOOTHING_FACTOR));

        cv::Point tl2(static_cast<int>(left), static_cast<int>(top));
        cv::Point br2(static_cast<int>(right), static_cast<int>(bottom));

        m_motionBoundingRect = cv::Rect(tl2, br2);
    }
    else
    {
        // We have no current motion but rather than instantly removign the bounding
        // rectangle instead we maitain its top-left position but shrink it down
        // to zero area using the bounding rectangle.
        std::lock_guard<std::mutex> lock(m_motionMutex);

        double w =
            (static_cast<double>(m_motionBoundingRect.width) * BOUNDING_RECT_SMOOTHING_FACTOR);
        double h =
            (static_cast<double>(m_motionBoundingRect.height) * BOUNDING_RECT_SMOOTHING_FACTOR);

        m_motionBoundingRect = cv::Rect(m_motionBoundingRect.tl().x,
                                        m_motionBoundingRect.tl().y,
                                        static_cast<int>(w),
                                        static_cast<int>(h));

        // If we still have a bounding rect keep motion detected flag set.
        motionDetected = m_motionBoundingRect.area() > 0;
    }

    return motionDetected;
}

void RtspStreamProcessor::UpdateNextFrame()
{
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_nextGreyFrame = m_videoFrame;
    }

    if (m_shrinkFramesForMotionDetection)
    {
        cv::resize(m_nextGreyFrame,
                   m_nextGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }

    cv::cvtColor(m_nextGreyFrame, m_nextGreyFrame, CV_BGR2GRAY);
}

void RtspStreamProcessor::RotateFrames()
{
    m_prevGreyFrame    = m_currentGreyFrame;
    m_currentGreyFrame = m_nextGreyFrame;
}

void RtspStreamProcessor::CheckMotionDetector()
{
    if (!CheckMotionSchedule())
    {
        return;
    }

    UpdateNextFrame();

    // TODO: if motion detected then trigger screen capture/writing of video
    // around motion, plus email alert.
    if (DetectMotion())
    {
    }
    else
    {
    }

    RotateFrames();
}

} // namespace ipfreely
