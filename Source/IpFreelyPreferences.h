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
 * \file IpFreelyPreferencesDialog.h
 * \brief File containing declaration of IpFreelyPreferences class.
 */
#ifndef IPFREELYPREFERENCES_H
#define IPFREELYPREFERENCES_H

#include <string>
#include <cstdint>
#include <vector>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/access.hpp>
#include "Serialization/SerializationIncludes.h"

/*! \brief ipfreely namespace */
namespace ipfreely
{

/*! \brief Class defining application preferences. */
class IpFreelyPreferences final
{
    friend class cereal::access;

public:
    /*!
     * \brief IpFreelyPreferences's default constructor.
     * \param[in] load - Shoul dconstructor load settings from disk on startup.
     */
    explicit IpFreelyPreferences(bool const load = true);

    /*! \brief IpFreelyPreferences's default destructor. */
    ~IpFreelyPreferences() = default;

    /*! \brief IpFreelyPreferences's default copy constructor. */
    IpFreelyPreferences(IpFreelyPreferences const&) = default;

    /*! \brief IpFreelyPreferences's default move constructor. */
    IpFreelyPreferences(IpFreelyPreferences&&) = default;

    /*! \brief IpFreelyPreferences's default copy assignment operator. */
    IpFreelyPreferences& operator=(IpFreelyPreferences const&) = default;

    /*! \brief IpFreelyPreferences's default move assignment operator. */
    IpFreelyPreferences& operator=(IpFreelyPreferences&&) = default;

    /*!
     * \brief SaveFolderPath retrieves the save folder path.
     * \return A string containing the save folder path.
     */
    std::string SaveFolderPath() const noexcept;

    /*!
     * \brief SetSaveFolderPath sets the save folder path.
     * \param[in] saveFolderPath - A string containing the save folder path.
     */
    void SetSaveFolderPath(std::string const& saveFolderPath) noexcept;

    /*!
     * \brief FileDurationInSecs retrieves the file duration.
     * \return The file duration in seconds.
     */
    double FileDurationInSecs() const noexcept;

    /*!
     * \brief SetFileDuration sets the file duration.
     * \param[in] fileDurationInSecs - The file duration in seconds.
     */
    void SetFileDurationInSecs(double const fileDurationInSecs) noexcept;

    /*!
     * \brief ConnectToCamerasOnStartup retrieves the auto connect flag.
     * \return True if we should auto connect, false otherwise.
     */
    bool ConnectToCamerasOnStartup() const noexcept;

    /*!
     * \brief SetConnectToCamerasOnStartup sets the auto connect flag.
     * \param[in] connectToCamerasOnStartup -  Auto connect flag.
     */
    void SetConnectToCamerasOnStartup(bool const connectToCamerasOnStartup) noexcept;

    /*!
     * \brief RecordingSchedule sets the current recording schedule.
     * \return A matrix of flags denoting for which hours of which days we should be recording.
     */
    std::vector<std::vector<bool>> RecordingSchedule() const noexcept;
    /*!
     * \brief SetRecordingSchedule retrieves the current recording schedule.
     * \param[in] schedule - A matrix of flags denoting for which hours of which days we should be
     * recording.
     */
    void SetRecordingSchedule(std::vector<std::vector<bool>> const& schedule);

    /*!
     * \brief MotionTrackingSchedule sets the current motion tracking schedule.
     * \return A matrix of flags denoting for which hours of which days we should be recording.
     */
    std::vector<std::vector<bool>> MotionTrackingSchedule() const noexcept;
    /*!
     * \brief SetMotionTrackingSchedule retrieves the current motion tracking schedule.
     * \param[in] schedule - A matrix of flags denoting for which hours of which days we should be
     * recording.
     */
    void SetMotionTrackingSchedule(std::vector<std::vector<bool>> const& schedule);

    /*!
     * \brief Save the preferences to disk from memory.
     */
    void Save() const;

    /*!
     * \brief Load the preferences from disk to memory.
     */
    void Load();

private:
    template <class Archive> void serialize(Archive& ar, const unsigned int version)
    {
        if (version < 1)
        {
            return;
        }

        ar(CEREAL_NVP(m_saveFolderPath), CEREAL_NVP(m_fileDurationInSecs));

        int32_t temp = m_connectToCamerasOnStartup ? 1 : 0;
        ar(CEREAL_NVP(temp));
        m_connectToCamerasOnStartup = temp == 1;

        ar(CEREAL_NVP(m_schedule), CEREAL_NVP(m_mtSchedule));
    }

private:
    std::string                    m_cfgPath{};
    std::string                    m_saveFolderPath{};
    double                         m_fileDurationInSecs{600.0};
    bool                           m_connectToCamerasOnStartup{false};
    std::vector<std::vector<bool>> m_schedule{7, {true, true, true, true, true, true, true, true,
                                                  true, true, true, true, true, true, true, true,
                                                  true, true, true, true, true, true, true, true}};
    std::vector<std::vector<bool>> m_mtSchedule{
        7, {true, true, true, true, true, true, true, true, true, true, true, true,
            true, true, true, true, true, true, true, true, true, true, true, true}};
};

} // namespace ipfreely

CEREAL_CLASS_VERSION(ipfreely::IpFreelyPreferences, 1);

#endif // IPFREELYPREFERENCES_H
