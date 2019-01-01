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
 * \file IpFreelyCameraDatabase.h
 * \brief File containing declaration of the IP camera database code.
 */
#ifndef IPFREELYCAMERADATABASE_H
#define IPFREELYCAMERADATABASE_H

#include <string>
#include <map>
#include <vector>
#include <QRect>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/access.hpp>
#include "Serialization/SerializationIncludes.h"

/*! \brief The ipfreely namespace. */
namespace ipfreely
{

/*! \brief Camera ID enumeration. */
enum class eCamId
{
    noCam,
    cam1,
    cam2,
    cam3,
    cam4
};

/*! \brief Motion detector mode. */
enum class eMotionDetectorMode
{
    off,
    lowSensitivity,
    mediumSensitivity,
    highSensitivity,
    manual
};

/*! \brief Minimum allowed recording FPS. */
static constexpr double MIN_FPS = 1.0;

/*! \brief Maximum allowed recording FPS. */
static constexpr double MAX_FPS = 60.0;

/*! \brief Camera's details structure. */
struct IpCamera final
{
    /*! \brief Camera's video stream's RTSP or HTTP(S) URL or local ID. */
    std::string streamUrl{};

    /*! \brief Camera's onboard SD card URL. */
    std::string storageHttpUrl{};

    /*! \brief Camera' description text string. */
    std::string description{};

    /*! \brief Camera's access username. */
    std::string username{};

    /*! \brief Camera's access password. */
    std::string password{};

    /*! \brief Camera's ID. */
    eCamId camId{eCamId::noCam};

    /*! \brief Enabled scheduled recording mode, when enabled this disables maual recording. */
    bool enableScheduledRecording{false};

    /*! \brief Motion detector mode for this camera. */
    eMotionDetectorMode motionDectorMode{eMotionDetectorMode::off};

    /*! \brief Shrink camera's video frames for motion detector. */
    bool shrinkVideoFrames{false};

    /*! \brief Threshold to filter out background motion noise. */
    double pixelThreshold{0.0};

    /*! \brief Max motionstd deviation to filter out large ambient changes in motion. */
    double maxMotionStdDev{0};

    /*! \brief Minimum motion area required for motion to count. */
    double minMotionAreaPercentFactor{0.0};

    /*! \brief Motion area averaging factor. */
    double motionAreaAveFactor{0.0};

    /*! \brief Typedef to a point pair. */
    typedef std::pair<double, double> point_t;

    /*! \brief Typedef to a region pair. */
    typedef std::pair<point_t /*left,top*/, point_t /*width,height*/> region_t;

    /*! \brief Typedef to a a vector of region pairs. */
    typedef std::vector<region_t> regions_t;

    /*! \brief Vector of motion detection regions. */
    regions_t motionRegions{};

    /*! \brief Camera's maximum prefered recording FPS as defined in its settings. */
    double cameraMaxFps{25.0};

    /*! \brief Enabled scheduled motion recording mode. */
    bool enabledMotionRecording{false};

    /*! \brief IpCamera's default constructor. */
    IpCamera() = default;

    /*! \brief IpCamera's default destructor. */
    ~IpCamera() = default;

    /*! \brief IpCamera's default copy constructor. */
    IpCamera(IpCamera const&) = default;

    /*! \brief IpCamera's default move constructor. */
    IpCamera(IpCamera&&) = default;

    /*! \brief IpCamera's default copy assignment operator. */
    IpCamera& operator=(IpCamera const&) = default;

    /*! \brief IpCamera's default move assignment operator. */
    IpCamera& operator=(IpCamera&&) = default;

    /*!
     * \brief CompleteStreamUrl gives the full RTSP or HTTP(S) stream URL or simply the ID as a
     * string id using a local webcam.
     * \param[out] isId - True if URL is actually a numeric ID, false otherwise.
     * \return The full URL string.
     *
     * In case of RTSP stream it returns complete url with the format:
     * rtsp://<username>:<password>@<path stub>
     * by splitting IpCamera::rtspUrl into: "rtsp://" and "<path stub>"
     * then combining these parts with IpCamera::username and
     * IpCamera::password.
     */
    std::string CompleteStreamUrl(bool& isId) const noexcept;

    /*!
     * \brief CompleteStorageHttpUrl gives the full HTTP(S) SD card URL.
     * \return The full URL string.
     *
     * Returns complete HTTP(S) url with the format:
     * http://<username>:<password>@<path stub>
     * by splitting IpCamera::rtspUrl into: "rtsp://" and "<path stub>"
     * then combining these parts with IpCamera::username and
     * IpCamera::password.
     */
    std::string CompleteStorageHttpUrl(bool const isHttps = false) const noexcept;

    /*!
     * \brief IsValid tests if settings are viable.
     * \return True if valid, false otherwise.
     * */
    bool IsValid() const noexcept;

    /*!
     * \brief serialize read/writes  the member data to a streamable archive.
     * \param[in] ar - The archive.
     * \param[in] version - The data version number.
     */
    template <class Archive> void serialize(Archive& ar, const unsigned int version)
    {
        if (version < 1)
        {
            return;
        }

        if (version < 6)
        {
            std::string rtspUrl;
            ar(CEREAL_NVP(rtspUrl));
            streamUrl = rtspUrl;
        }
        else
        {
            ar(CEREAL_NVP(streamUrl));
        }

        ar(CEREAL_NVP(storageHttpUrl),
           CEREAL_NVP(description),
           CEREAL_NVP(username),
           CEREAL_NVP(password),
           CEREAL_NVP(camId));

        int temp = enableScheduledRecording ? 1 : 0;
        ar(CEREAL_NVP(temp));
        enableScheduledRecording = temp == 1;

        if (version > 1)
        {
            // Added with version 2.
            ar(CEREAL_NVP(motionDectorMode));

            // Added with version 2.
            temp = shrinkVideoFrames ? 1 : 0;
            ar(CEREAL_NVP(temp));
            shrinkVideoFrames = temp == 1;
        }

        if (version > 2)
        {
            // Added with version 3.
            ar(CEREAL_NVP(pixelThreshold),
               CEREAL_NVP(maxMotionStdDev),
               CEREAL_NVP(minMotionAreaPercentFactor),
               CEREAL_NVP(motionAreaAveFactor));
        }

        if (version > 3)
        {
            // Added with version 4.
            ar(CEREAL_NVP(motionRegions));
        }

        if (version > 4)
        {
            // Added with version 5.
            ar(CEREAL_NVP(cameraMaxFps));
        }

        if (version > 6)
        {
            // Added with version 7.
            temp = enabledMotionRecording ? 1 : 0;
            ar(CEREAL_NVP(temp));
            enabledMotionRecording = temp == 1;
        }
    }
};

/*! \brief Cameras' database class. */
class IpFreelyCameraDatabase final
{
    friend class cereal::access;

public:
    /*!
     * \brief IpFreelyCameraDatabase's default constructor.
     * \param[in] load - Should constructor load settings from disk on startup.
     */
    explicit IpFreelyCameraDatabase(bool const load = true);

    /*! \brief IpFreelyCameraDatabase's default destructor. */
    ~IpFreelyCameraDatabase() = default;

    /*! \brief IpFreelyCameraDatabase's default copy constructor. */
    IpFreelyCameraDatabase(IpFreelyCameraDatabase const&) = default;

    /*! \brief IpFreelyCameraDatabase's default move constructor. */
    IpFreelyCameraDatabase(IpFreelyCameraDatabase&&) = default;

    /*! \brief IpFreelyCameraDatabase's default copy assignment operator. */
    IpFreelyCameraDatabase& operator=(IpFreelyCameraDatabase const&) = default;

    /*! \brief IpFreelyCameraDatabase's default move assignment operator. */
    IpFreelyCameraDatabase& operator=(IpFreelyCameraDatabase&&) = default;

    /*!
     * \brief AddCamera adds new camera details to database.
     * \param[in] camera - A camera details object.
     *
     * Throws std::invalid_argument on failure, e.g. duplicate
     * camera ID.
     */
    void AddCamera(IpCamera const& camera);

    /*!
     * \brief UpdateCamera update exsiting camera details in database.
     * \param[in] camera - A camera details object.
     */
    void UpdateCamera(IpCamera const& camera) noexcept;

    /*!
     * \brief RemoveCamera removes a camera with a ID.
     * \param[in] camId - A camera ID.
     */
    void RemoveCamera(eCamId const camId) noexcept;

    /*!
     * \brief GetCameraCount reports the number of cameras in the database.
     * \return The number of cameras in the database.
     */
    size_t GetCameraCount() const noexcept;

    /*!
     * \brief DoesCameraExist checks if a camera with the given ID.
     * \param[in] camId - A camera ID.
     * \return True of the camera exists, false otherwise..
     */
    bool DoesCameraExist(eCamId const camId) const noexcept;

    /*!
     * \brief FindCamera find a camera with the given ID.
     * \param[in] camId - A camera ID.
     * \param[out] camera - A copy of the camera object if found.
     * \return True of the camera exists, false otherwise.
     */
    bool FindCamera(eCamId const camId, IpCamera& camera) const noexcept;

    /*!
     * \brief Save the database file to disk from memory.
     */
    void Save() const;

    /*!
     * \brief Load the database file from disk to memory.
     */
    void Load();

private:
    template <class Archive> void serialize(Archive& ar, const unsigned int version)
    {
        if (version < 1)
        {
            return;
        }

        ar(CEREAL_NVP(m_cameras));
    }

private:
    std::string                m_dbPath{};
    std::map<eCamId, IpCamera> m_cameras{};
};

/*!
 * \brief CreateQRectFromVideoFrameDims create a motion region QRect as subregion of
 * a video frame.
 * \param[in] videoFrameWidth - Video frame's width.
 * \param[in] videoFrameHeight - Video frame's height.
 * \param[in] motionRegion - User defined motion detection region.
 * \return QRect defining motion area.
 */
QRect CreateQRectFromVideoFrameDims(int const videoFrameWidth, int const videoFrameHeight,
                                    IpCamera::region_t const& motionRegion);

} // namespace ipfreely

CEREAL_CLASS_VERSION(ipfreely::IpCamera, 7);
CEREAL_CLASS_VERSION(ipfreely::IpFreelyCameraDatabase, 1);

#endif // IPFREELYCAMERADATABASE_H
