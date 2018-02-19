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
 * \file IpFreelyDiskSpaceManager.h
 * \brief File containing declaration of IpFreelyDiskSpaceManager threaded class.
 */
#ifndef IPFREELYDISKSPACEMANAGER_H
#define IPFREELYDISKSPACEMANAGER_H

#include <string>
#include <list>
#include <memory>

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

/*! \brief Class defining disk space manager thread. */
class IpFreelyDiskSpaceManager final
{
public:
    /*!
     * \brief IpFreelyDiskSpaceManager constructor.
     * \param[in] saveFolderPath - A local folder to save captured videos to.
     * \param[in] maxNumDaysToStore - Maximum number of days of data to store.
     * \param[in] maxPercentUsedSpace - Maximum disk space percentage to be used.
     */
    IpFreelyDiskSpaceManager(std::string const& saveFolderPath, int const maxNumDaysToStore,
                             int const maxPercentUsedSpace);

    /*! \brief IpFreelyDiskSpaceManager destructor. */
    virtual ~IpFreelyDiskSpaceManager();

    /*! \brief IpFreelyDiskSpaceManager deleted copy constructor. */
    IpFreelyDiskSpaceManager(IpFreelyDiskSpaceManager const&) = delete;

    /*! \brief IpFreelyDiskSpaceManager deleted copy assignment operator. */
    IpFreelyDiskSpaceManager& operator=(IpFreelyDiskSpaceManager const&) = delete;

private:
    void ThreadEventCallback() noexcept;
    void CheckUsedDiskSpace();
    void CheckNumDaysDataStored();
    bool DeleteOldestRecording();

private:
    std::string                                     m_saveFolderPath{};
    int                                             m_maxNumDaysToStore{7};
    int                                             m_maxPercentUsedSpace{90};
    std::list<std::wstring>                         m_subDirs;
    std::shared_ptr<core_lib::threads::EventThread> m_eventThread;
};

} // namespace ipfreely

#endif // IPFREELYDISKSPACEMANAGER_H
