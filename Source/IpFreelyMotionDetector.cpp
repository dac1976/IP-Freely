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
 * \file IpFreelyMotionDetector.cpp
 * \brief File containing definition of IpFreelyMotionDetector threaded class.
 */
#include "IpFreelyMotionDetector.h"
#include <sstream>
#include <cmath>
#include <cstdint>
#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
#include "StringUtils/StringUtils.h"
#include "DebugLog/DebugLogging.h"

namespace bfs = boost::filesystem;

namespace ipfreely
{

static constexpr int    MESSAGE_ID           = 1;
static constexpr double DIFF_MAX_VALUE       = 255.0;
static constexpr int    IDEAL_FRAME_HEIGHT   = 600;
static constexpr size_t HOLD_ON_OFF_SECS     = 10;
static constexpr int    BOUNDING_RECT_MARGIN = 1;

#if defined(MOTION_DETECTOR_DEBUG)
static constexpr int CONTOUR_LINE_THICKNESS = 2;
#endif

IpFreelyMotionDetector::IpFreelyMotionDetector(std::string const& name,
                                               IpCamera const&    cameraDetails,
                                               std::string const& saveFolderPath,
                                               double const       requiredFileDurationSecs,
                                               double const fps, int const originalWidth,
                                               int const originalHeight)
    : m_name(core_lib::string_utils::RemoveIllegalChars(name))
    , m_cameraDetails(cameraDetails)
    , m_saveFolderPath(saveFolderPath)
    , m_requiredFileDurationSecs(requiredFileDurationSecs)
    , m_fps(fps)
    , m_originalWidth(originalWidth)
    , m_originalHeight(originalHeight)
    , m_updatePeriodMillisecs(static_cast<unsigned int>(1000.0 / m_fps))
    , m_erosionKernel(cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)))
    , m_holdOffFrameCountLimit(static_cast<size_t>(std::ceil(m_fps)) * HOLD_ON_OFF_SECS)
    , m_msgQueueThread(std::bind(&IpFreelyMotionDetector::MessageDecoder, std::placeholders::_1),
                       core_lib::threads::eOnDestroyOptions::processRemainingItems)
{
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

    Initialise();

    DEBUG_MESSAGE_EX_INFO("Started motion detector for stream at: "
                          << m_cameraDetails.streamUrl
                          << ", required file duration (in seconds) set to: "
                          << m_requiredFileDurationSecs);

    m_msgQueueThread.RegisterMessageHandler(
        MESSAGE_ID,
        std::bind(&IpFreelyMotionDetector::MessageHandler, this, std::placeholders::_1));
}

void IpFreelyMotionDetector::AddNextFrame(cv::Mat const& videoFrame)
{
    m_msgQueueThread.Push(std::make_shared<cv::Mat>(videoFrame));
}

QRect IpFreelyMotionDetector::CurrentMotionRect() const noexcept
{
    std::lock_guard<std::mutex> lock(m_motionMutex);
    return QRect(m_motionBoundingRect.tl().x,
                 m_motionBoundingRect.tl().y,
                 m_motionBoundingRect.width,
                 m_motionBoundingRect.height);
}

bool IpFreelyMotionDetector::WritingStream() const noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    return m_writingStream;
}

void IpFreelyMotionDetector::Initialise()
{
#if defined(MOTION_DETECTOR_DEBUG)
    cv::namedWindow("motion");
#endif

    if (m_cameraDetails.shrinkVideoFrames && (m_originalHeight > IDEAL_FRAME_HEIGHT))
    {
        m_motionFrameScalar =
            static_cast<double>(IDEAL_FRAME_HEIGHT) / static_cast<double>(m_originalHeight);

        DEBUG_MESSAGE_EX_INFO("Shrinking video frames for motion detection.");
    }
    else
    {
        DEBUG_MESSAGE_EX_INFO("Full-size video frames for motion detection.");
    }

    double const motionFrameArea = static_cast<double>(m_originalHeight * m_originalWidth) *
                                   m_motionFrameScalar * m_motionFrameScalar;

    m_minImageChangeArea =
        static_cast<int>(motionFrameArea * m_cameraDetails.minMotionAreaPercentFactor);

    switch (m_cameraDetails.motionDectorMode)
    {
    case eMotionDetectorMode::lowSensitivity:
        DEBUG_MESSAGE_EX_INFO("Motion tracking (low sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::mediumSensitivity:
        DEBUG_MESSAGE_EX_INFO(
            "Motion tracking (medium sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::highSensitivity:
        DEBUG_MESSAGE_EX_INFO("Motion tracking (high sensitivity) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::manual:
        DEBUG_MESSAGE_EX_INFO("Motion tracking (manual settings) enabled for camera: " << m_name);
        break;
    case eMotionDetectorMode::off:
        // Do nothing - but remove compiler warning
        return;
    }
}

void IpFreelyMotionDetector::InitialiseFrames()
{
    if (!m_initialiseFrames)
    {
        return;
    }

    m_initialiseFrames = false;

    if (m_cameraDetails.shrinkVideoFrames)
    {
        cv::resize(*m_originalFrame,
                   m_prevGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }
    else
    {
        m_prevGreyFrame = *m_originalFrame;
    }

    cv::cvtColor(m_prevGreyFrame, m_prevGreyFrame, cv::COLOR_BGR2GRAY);

    if (m_cameraDetails.shrinkVideoFrames)
    {
        cv::resize(*m_originalFrame,
                   m_currentGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }
    else
    {
        m_currentGreyFrame = *m_originalFrame;
    }

    cv::cvtColor(m_currentGreyFrame, m_currentGreyFrame, cv::COLOR_BGR2GRAY);
}

void IpFreelyMotionDetector::UpdateNextFrame()
{
    if (m_cameraDetails.shrinkVideoFrames)
    {
        cv::resize(m_nextGreyFrame,
                   m_nextGreyFrame,
                   {},
                   m_motionFrameScalar,
                   m_motionFrameScalar,
                   cv::INTER_AREA);
    }
    else
    {
        m_nextGreyFrame = *m_originalFrame;
    }

    cv::cvtColor(m_nextGreyFrame, m_nextGreyFrame, cv::COLOR_BGR2GRAY);
}

bool IpFreelyMotionDetector::DetectMotion()
{
    // This algorithm is inspired by an example given here:
    // https://github.com/cedricve/motion-detection
    // However I have taken this basic idea and added
    // some extra layers to the algorithm to smooth out
    // detection. The motion region now gets a smoothing rolling
    // average applied to it between frames to make it
    // less janky. I've also added a check to filter
    // out motion regions less than a configurable percentage
    // of the frame's total area.

    // Calculate differences between the images and do AND-operation
    // then threshold image, low differences are ignored (ex. contrast
    // change due to sunlight).
    cv::Mat diff1, diff2, motion;
    cv::absdiff(m_prevGreyFrame, m_nextGreyFrame, diff1);
    cv::absdiff(m_nextGreyFrame, m_currentGreyFrame, diff2);
    cv::bitwise_and(diff1, diff2, motion);
    cv::threshold(
        motion, motion, m_cameraDetails.pixelThreshold, DIFF_MAX_VALUE, cv::THRESH_BINARY);
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
    if (stddev[0] < m_cameraDetails.maxMotionStdDev)
    {
        size_t numChanges = 0;

        // Loop over image and detect changes. This is better
        // for CPU performance compared to using OpenCV's
        // contour fitting algorithms.
        for (int j = 0; j < motion.rows; j += 2)
        {
            for (int i = 0; i < motion.cols; i += 2)
            {
                // check if at pixel (j,i) intensity is equal to 255
                // this means that the pixel is different in the sequence
                // of images (prev_frame, current_frame, next_frame)
                if (static_cast<int>(motion.at<uint8_t>(j, i)) == 255)
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
            if (min_x - BOUNDING_RECT_MARGIN > 0)
            {
                min_x -= BOUNDING_RECT_MARGIN;
            }

            if (min_y - BOUNDING_RECT_MARGIN > 0)
            {
                min_y -= BOUNDING_RECT_MARGIN;
            }

            if (max_x + BOUNDING_RECT_MARGIN < (motion.cols - 1))
            {
                max_x += BOUNDING_RECT_MARGIN;
            }

            if (max_y + BOUNDING_RECT_MARGIN < (motion.rows - 1))
            {
                max_y += BOUNDING_RECT_MARGIN;
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
        // Create a motion bounding rectangle scaled to original
        // video frame's size.
        cv::Point tl1(static_cast<int>(static_cast<double>(min_x) / m_motionFrameScalar),
                      static_cast<int>(static_cast<double>(min_y) / m_motionFrameScalar));
        cv::Point br1(static_cast<int>(static_cast<double>(max_x) / m_motionFrameScalar),
                      static_cast<int>(static_cast<double>(max_y) / m_motionFrameScalar));

        auto minBoundingRect = cv::Rect(tl1, br1);

        // To make the bounding rectangle appear less janky we'll
        // combine it with the previous bounding rectangle using a
        // smoothing rolling average controlled by the smoothing factor.
        std::lock_guard<std::mutex> lock(m_motionMutex);

        double l = (static_cast<double>(m_motionBoundingRect.tl().x) *
                    m_cameraDetails.motionAreaAveFactor) +
                   (static_cast<double>(minBoundingRect.tl().x) *
                    (1.0 - m_cameraDetails.motionAreaAveFactor));
        double t = (static_cast<double>(m_motionBoundingRect.tl().y) *
                    m_cameraDetails.motionAreaAveFactor) +
                   (static_cast<double>(minBoundingRect.tl().y) *
                    (1.0 - m_cameraDetails.motionAreaAveFactor));
        double r = (static_cast<double>(m_motionBoundingRect.br().x) *
                    m_cameraDetails.motionAreaAveFactor) +
                   (static_cast<double>(minBoundingRect.br().x) *
                    (1.0 - m_cameraDetails.motionAreaAveFactor));
        double b = (static_cast<double>(m_motionBoundingRect.br().y) *
                    m_cameraDetails.motionAreaAveFactor) +
                   (static_cast<double>(minBoundingRect.br().y) *
                    (1.0 - m_cameraDetails.motionAreaAveFactor));

        cv::Point tl2(static_cast<int>(l), static_cast<int>(t));
        cv::Point br2(static_cast<int>(r), static_cast<int>(b));

        m_motionBoundingRect = cv::Rect(tl2, br2);
    }
    else
    {
        // We have no current motion but rather than instantly removing the bounding
        // rectangle instead shrink it down to zero area gradually over a few iterations.
        std::lock_guard<std::mutex> lock(m_motionMutex);

        double l = m_motionBoundingRect.tl().x +
                   static_cast<int>(static_cast<double>(m_motionBoundingRect.width) * 0.5);
        double t = m_motionBoundingRect.tl().y +
                   static_cast<int>(static_cast<double>(m_motionBoundingRect.height) * 0.5);
        double w =
            (static_cast<double>(m_motionBoundingRect.width) * m_cameraDetails.motionAreaAveFactor);
        double h = (static_cast<double>(m_motionBoundingRect.height) * 0.25);

        m_motionBoundingRect = cv::Rect(
            static_cast<int>(l), static_cast<int>(t), static_cast<int>(w), static_cast<int>(h));
    }

    return CheckForIntersections();
}

bool IpFreelyMotionDetector::CheckForIntersections()
{
    if (m_motionBoundingRect.area() == 0)
    {
        return false;
    }

    if (m_cameraDetails.motionRegions.empty())
    {
        return true;
    }

    QRect mr(m_motionBoundingRect.tl().x,
             m_motionBoundingRect.tl().y,
             m_motionBoundingRect.width,
             m_motionBoundingRect.height);

    bool motionIntersection{false};

    for (auto const& region : m_cameraDetails.motionRegions)
    {
        auto r = ipfreely::CreateQRectFromVideoFrameDims(m_originalWidth, m_originalHeight, region);

        if (mr.intersects(r))
        {
            motionIntersection = true;

            DEBUG_MESSAGE_EX_INFO("Motion detector intersection found for camera stream URL: "
                                  << m_cameraDetails.streamUrl << ", region details: L = "
                                  << region.first.first << ", T = " << region.first.second
                                  << ", W = " << region.second.first
                                  << ", H = " << region.second.second);

            break;
        }
    }

    return motionIntersection;
}

void IpFreelyMotionDetector::RotateFrames()
{
    m_prevGreyFrame    = m_currentGreyFrame;
    m_currentGreyFrame = m_nextGreyFrame;
}

int IpFreelyMotionDetector::MessageDecoder(video_frame_t const& /*msg*/)
{
    return MESSAGE_ID;
}

bool IpFreelyMotionDetector::MessageHandler(video_frame_t& msg)
{
    m_originalFrame = msg;

    // Get current time stamp.
    m_currentTime = time(nullptr);

    InitialiseFrames();
    UpdateNextFrame();

    bool recording      = m_videoWriter.get() != nullptr;
    bool motionDetected = false;

    if (DetectMotion())
    {
        motionDetected = true;

        // Reset hold-off count if we've detected motion.
        m_holdOffFrameCount = 0;
    }
    else
    {
        // If recording in progress increment hold-off count.
        if (recording)
        {
            ++m_holdOffFrameCount;
        }
    }

    // If hold-off count reached then reset count and reset writer
    // and finally set recording flag to false.
    if (m_holdOffFrameCount == m_holdOffFrameCountLimit)
    {
        DEBUG_MESSAGE_EX_INFO("Motion detector hold-off period finished, camera stream URL: "
                              << m_cameraDetails.streamUrl);

        m_holdOffFrameCount = 0;
        m_videoWriter.release();
        recording = false;
        SetWritingStream(false);
    }

    if (motionDetected || recording)
    {
        CreateCaptureObjects();
    }

    WriteVideoFrame();
    RotateFrames();

    return true;
}

void IpFreelyMotionDetector::CreateCaptureObjects()
{
    if (m_videoWriter)
    {
        if (m_fileDurationSecs < m_requiredFileDurationSecs)
        {
            return;
        }

        DEBUG_MESSAGE_EX_INFO(
            "Motion detector file duration reached for current video file, camera stream URL: "
            << m_cameraDetails.streamUrl << ", file writer being closed.");

        m_videoWriter.release();
        SetWritingStream(false);
    }

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
            std::ostringstream oss;
            oss << "Failed to create directories: " << p.string();
            BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
        }
    }

    std::ostringstream oss;
    oss << m_name << "_motion_" << m_currentTime << ".avi";

    p /= oss.str();

    DEBUG_MESSAGE_EX_INFO("Creating new output video file: " << p.string() << ", FPS: " << m_fps);

    m_fileDurationSecs = 0.0;

#if BOOST_OS_WINDOWS
    m_videoWriter = cv::makePtr<cv::VideoWriter>(p.string().c_str(),
                                                 cv::VideoWriter::fourcc('D', 'I', 'V', 'X'),
                                                 m_fps,
                                                 cv::Size(m_originalWidth, m_originalHeight));
#else
    m_videoWriter = cv::makePtr<cv::VideoWriter>(p.string().c_str(),
                                                 cv::VideoWriter::fourcc('X', 'V', 'I', 'D'),
                                                 m_fps,
                                                 cv::Size(m_originalWidth, m_originalHeight));
#endif

    if (!m_videoWriter->isOpened())
    {
        m_videoWriter.release();
        DEBUG_MESSAGE_EX_ERROR("Failed to open VideoWriter object: " << p.string());
        return;
    }

    SetWritingStream(true);
}

void IpFreelyMotionDetector::WriteVideoFrame()
{
    if (m_videoWriter)
    {
        *m_videoWriter << *m_originalFrame;
        m_fileDurationSecs += static_cast<double>(m_updatePeriodMillisecs) / 1000.0;
    }
}

void IpFreelyMotionDetector::SetWritingStream(bool const writing) noexcept
{
    std::lock_guard<std::mutex> lock(m_writingMutex);
    m_writingStream = writing;
}

} // namespace ipfreely
