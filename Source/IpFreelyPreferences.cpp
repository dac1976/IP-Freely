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
#include "IpFreelyPreferences.h"
#include <sstream>
#include <fstream>
#include <utility>
#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
#include "Serialization/SerializeToVector.h"
#include "DebugLog/DebugLogging.h"

namespace bfs = boost::filesystem;

namespace ipfreely
{

IpFreelyPreferences::IpFreelyPreferences(bool const load)
{
    auto path        = bfs::initial_path();
    m_saveFolderPath = path.string();

    path /= "IpFreely.cfg";
    path = bfs::system_complete(path);

    m_cfgPath = path.string();

    if (load)
    {
        try
        {
            Load();
        }
        catch (...)
        {
            auto exceptionMsg = boost::current_exception_diagnostic_information();
            DEBUG_MESSAGE_EX_ERROR(exceptionMsg);
        }

        try
        {
            if (!bfs::exists(m_cfgPath))
            {
                Save();
                return;
            }
        }
        catch (...)
        {
            auto exceptionMsg = boost::current_exception_diagnostic_information();
            DEBUG_MESSAGE_EX_ERROR(exceptionMsg);
        }
    }
}

std::string IpFreelyPreferences::SaveFolderPath() const noexcept
{
    return m_saveFolderPath;
}

void IpFreelyPreferences::SetSaveFolderPath(std::string const& saveFolderPath) noexcept
{
    bfs::path p(saveFolderPath);
    p                = bfs::system_complete(p);
    m_saveFolderPath = p.string();
}

double IpFreelyPreferences::FileDurationInSecs() const noexcept
{
    return m_fileDurationInSecs;
}

void IpFreelyPreferences::SetFileDurationInSecs(double const fileDurationInSecs) noexcept
{
    m_fileDurationInSecs = fileDurationInSecs;
}

bool IpFreelyPreferences::ConnectToCamerasOnStartup() const noexcept
{
    return m_connectToCamerasOnStartup;
}

void IpFreelyPreferences::SetConnectToCamerasOnStartup(
    bool const connectToCamerasOnStartup) noexcept
{
    m_connectToCamerasOnStartup = connectToCamerasOnStartup;
}

std::vector<std::vector<bool>> IpFreelyPreferences::RecordingSchedule() const noexcept
{
    return m_schedule;
}

void IpFreelyPreferences::SetRecordingSchedule(std::vector<std::vector<bool>> const& schedule)
{
    if (schedule.size() != 7)
    {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of days in schedule."));
    }

    if (schedule.front().size() != 24)
    {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of hours in schedule."));
    }

    m_schedule = schedule;
}

std::vector<std::vector<bool>> IpFreelyPreferences::MotionTrackingSchedule() const noexcept
{
    return m_mtSchedule;
}

void IpFreelyPreferences::SetMotionTrackingSchedule(std::vector<std::vector<bool>> const& schedule)
{
    if (schedule.size() != 7)
    {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of days in schedule."));
    }

    if (schedule.front().size() != 24)
    {
        BOOST_THROW_EXCEPTION(std::invalid_argument("Incorrect number of hours in schedule."));
    }

    m_mtSchedule = schedule;
}

void IpFreelyPreferences::Save() const
{
    if (bfs::exists(m_cfgPath))
    {
        if (!bfs::remove(m_cfgPath))
        {
            std::ostringstream oss;
            oss << "failed to remove existing file: " << m_cfgPath;
            BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
        }
    }
    else
    {
        auto parentPath = bfs::path(m_cfgPath).parent_path();

        if (!bfs::exists(parentPath))
        {
            if (!bfs::create_directories(parentPath))
            {
                std::ostringstream oss;
                oss << "failed to create directories for file: " << m_cfgPath;
                BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
            }
        }
    }

    std::ofstream ofs(m_cfgPath.c_str());

    if (!ofs)
    {
        std::ostringstream oss;
        oss << "failed to create std::ofstream to: " << m_cfgPath;
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    core_lib::serialize::archives::out_port_bin_t oa(ofs);
    auto                                          prefs = *this;
    oa(CEREAL_NVP(prefs));
}

void IpFreelyPreferences::Load()
{
    IpFreelyPreferences prefs(false);

    if (!bfs::exists(m_cfgPath))
    {
        *this = std::move(prefs);
        return;
    }

    std::ifstream ifs(m_cfgPath.c_str());

    if (!ifs)
    {
        std::ostringstream oss;
        oss << "failed to create std::ifstream to: " << m_cfgPath;
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    core_lib::serialize::archives::in_port_bin_t ia(ifs);
    ia(CEREAL_NVP(prefs));
    *this = std::move(prefs);
}

} // namespace ipfreely
