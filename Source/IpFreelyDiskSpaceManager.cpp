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
 * \file IpFreelyDiskSpaceManager.cpp
 * \brief File containing definition of IpFreelyDiskSpaceManager threaded class.
 */
#include "IpFreelyDiskSpaceManager.h"
#include <QStorageInfo>
#include <sstream>
#include <algorithm>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include "Threads/EventThread.h"
#include "DebugLog/DebugLogging.h"
#include "FileUtils/FileUtils.h"
#include "StringUtils/StringUtils.h"

namespace bfs = boost::filesystem;

namespace ipfreely
{

static constexpr unsigned int UPDATE_PERIOD_MS = 60000;

IpFreelyDiskSpaceManager::IpFreelyDiskSpaceManager(std::string const& saveFolderPath,
                                                   int const          maxNumDaysToStore,
                                                   int const          maxPercentUsedSpace)
    : m_saveFolderPath(saveFolderPath)
    , m_maxNumDaysToStore(maxNumDaysToStore)
    , m_maxPercentUsedSpace(maxPercentUsedSpace)
{
    bfs::path p(m_saveFolderPath);
    p = bfs::system_complete(p);

    if (!bfs::exists(p))
    {
        std::ostringstream oss;
        oss << "Directory not found: " << p.string();
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    DEBUG_MESSAGE_EX_INFO(
        "Started disk space manager for disk containing save folder:" << m_saveFolderPath);

    m_eventThread = std::make_shared<core_lib::threads::EventThread>(
        std::bind(&IpFreelyDiskSpaceManager::ThreadEventCallback, this), UPDATE_PERIOD_MS);
}

IpFreelyDiskSpaceManager::~IpFreelyDiskSpaceManager()
{
    // Do nothing.
}

void IpFreelyDiskSpaceManager::ThreadEventCallback() noexcept
{
    try
    {
        // Get list of recording sub-directories.
        m_subDirs = core_lib::file_utils::ListSubDirectories(
            core_lib::string_utils::StringToWString(m_saveFolderPath));

        // Perform checks.
        CheckUsedDiskSpace();
        CheckNumDaysDataStored();
    }
    catch (...)
    {
        auto exceptionMsg = boost::current_exception_diagnostic_information();
        DEBUG_MESSAGE_EX_ERROR(exceptionMsg);
    }
}

void IpFreelyDiskSpaceManager::CheckUsedDiskSpace()
{
    while (true)
    {
        QStorageInfo info(QString::fromStdString(m_saveFolderPath));

        auto percentUsed =
            static_cast<int>(100.0 * (1.0 - (static_cast<double>(info.bytesAvailable()) /
                                             static_cast<double>(info.bytesTotal()))));

        if (percentUsed > m_maxPercentUsedSpace)
        {
            DEBUG_MESSAGE_EX_INFO("Percentage disk space used is too great ("
                                  << percentUsed << "%), will attempt to delete oldest data.");

            if (!DeleteOldestRecording())
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void IpFreelyDiskSpaceManager::CheckNumDaysDataStored()
{
    while (m_subDirs.size() > static_cast<size_t>(m_maxNumDaysToStore))
    {
        DEBUG_MESSAGE_EX_INFO("Too many days data found on disk. Expected: "
                              << m_maxNumDaysToStore << " but found: " << m_subDirs.size()
                              << ". Will attempt to delete oldest data.");

        if (!DeleteOldestRecording())
        {
            break;
        }
    }
}

bool IpFreelyDiskSpaceManager::DeleteOldestRecording()
{
    if (m_subDirs.size() <= 1)
    {
        DEBUG_MESSAGE_EX_WARNING(
            "No data will be deleted only the current day's sub-directory was found.");
        return false;
    }

    m_subDirs.sort();
    auto oldestSubDirItr = m_subDirs.begin();

    bfs::path p(m_saveFolderPath);
    p /= *oldestSubDirItr;
    p = bfs::system_complete(p);

    if (bfs::exists(p))
    {
        if (bfs::remove_all(p))
        {
            DEBUG_MESSAGE_EX_INFO(
                "Successfully deleted data recording sub-directory: " << p.string());
        }
        else
        {
            DEBUG_MESSAGE_EX_ERROR("Failed to delete data recording subdirectory: " << p.string());
        }
    }

    m_subDirs.erase(oldestSubDirItr);

    return true;
}

} // namespace ipfreely
