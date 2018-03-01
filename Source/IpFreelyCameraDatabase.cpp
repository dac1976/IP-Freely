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
 * \file IpFreelyCameraDatabase.cpp
 * \brief File containing definition of the IP camera database code.
 */
#include "IpFreelyCameraDatabase.h"
#include <sstream>
#include <fstream>
#include <utility>
#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/algorithm/string.hpp>
#include "Serialization/SerializeToVector.h"
#include "DebugLog/DebugLogging.h"

namespace bfs = boost::filesystem;

namespace ipfreely
{

static constexpr size_t RTSP_OFFSET  = 7;
static constexpr size_t HTTP_OFFSET  = 7;
static constexpr size_t HTTPS_OFFSET = 8;

namespace utils
{

std::string CompleteUrl(std::string const& url, std::string const& username,
                        std::string const& password, size_t const offset)
{
    if (username.empty() || password.empty())
    {
        return url;
    }
    else
    {
        std::string        urlType  = url.substr(0, offset);
        std::string        pathStub = url.substr(offset, std::string::npos);
        std::ostringstream completeUrl;
        completeUrl << urlType << username << ":" << password << "@" << pathStub;
        return completeUrl.str();
    }
}

} // namespace utils

std::string IpCamera::CompleteStreamUrl(bool& isId) const noexcept
{
    isId = false;
    std::string url;

    try
    {
        std::stoi(streamUrl);
        url  = streamUrl;
        isId = true;
    }
    catch (...)
    {
        std::string urlType = streamUrl.substr(0, RTSP_OFFSET);

        if (boost::to_upper_copy(urlType) == "RTSP://")
        {
            url = utils::CompleteUrl(streamUrl, username, password, RTSP_OFFSET);
        }
        else
        {
            urlType = streamUrl.substr(0, HTTPS_OFFSET);

            if (boost::to_upper_copy(urlType) == "HTTPS://")
            {
                url = utils::CompleteUrl(streamUrl, username, password, HTTPS_OFFSET);
            }
            else
            {
                urlType = streamUrl.substr(0, HTTP_OFFSET);

                if (boost::to_upper_copy(urlType) == "HTTP://")
                {
                    url = utils::CompleteUrl(streamUrl, username, password, HTTP_OFFSET);
                }
                else
                {
                    BOOST_THROW_EXCEPTION(std::invalid_argument("invalid stream url"));
                }
            }
        }
    }

    return url;
}

std::string IpCamera::CompleteStorageHttpUrl(bool const isHttps) const noexcept
{
    return utils::CompleteUrl(
        storageHttpUrl, username, password, isHttps ? HTTPS_OFFSET : HTTP_OFFSET);
}

bool IpCamera::IsValid() const noexcept
{
    return !streamUrl.empty() && (camId != eCamId::noCam);
}

IpFreelyCameraDatabase::IpFreelyCameraDatabase(bool const load)
{
    auto path = bfs::initial_path();
    path /= "IpFreely.db";
    path = bfs::system_complete(path);

    m_dbPath = path.string();

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
            if (!bfs::exists(m_dbPath))
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

void IpFreelyCameraDatabase::AddCamera(IpCamera const& camera)
{
    if (DoesCameraExist(camera.camId))
    {
        std::ostringstream oss;
        oss << "Camera already exists, ID: " << static_cast<int>(camera.camId);
        BOOST_THROW_EXCEPTION(std::invalid_argument(oss.str()));
    }
    else
    {
        m_cameras[camera.camId] = camera;
    }
}

void IpFreelyCameraDatabase::UpdateCamera(IpCamera const& camera) noexcept
{
    m_cameras[camera.camId] = camera;
}

void IpFreelyCameraDatabase::RemoveCamera(eCamId const camId) noexcept
{
    if (DoesCameraExist(camId))
    {
        m_cameras.erase(camId);
    }
}

size_t IpFreelyCameraDatabase::GetCameraCount() const noexcept
{
    return m_cameras.size();
}

bool IpFreelyCameraDatabase::DoesCameraExist(eCamId const camId) const noexcept
{
    return m_cameras.count(camId) > 0;
}

bool IpFreelyCameraDatabase::FindCamera(eCamId const camId, IpCamera& camera) const noexcept
{
    auto iter = m_cameras.find(camId);

    if (iter == m_cameras.end())
    {
        return false;
    }

    camera = iter->second;

    return true;
}

void IpFreelyCameraDatabase::Save() const
{
    if (bfs::exists(m_dbPath))
    {
        if (!bfs::remove(m_dbPath))
        {
            std::ostringstream oss;
            oss << "failed to remove existing file: " << m_dbPath;
            BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
        }
    }
    else
    {
        auto parentPath = bfs::path(m_dbPath).parent_path();

        if (!bfs::exists(parentPath))
        {
            if (!bfs::create_directories(parentPath))
            {
                std::ostringstream oss;
                oss << "failed to create directories for file: " << m_dbPath;
                BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
            }
        }
    }

    std::ofstream ofs(m_dbPath.c_str());

    if (!ofs)
    {
        std::ostringstream oss;
        oss << "failed to create std::ofstream to: " << m_dbPath;
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    core_lib::serialize::archives::out_port_bin_t oa(ofs);
    auto                                          camDb = *this;
    oa(CEREAL_NVP(camDb));
}

void IpFreelyCameraDatabase::Load()
{
    IpFreelyCameraDatabase camDb(false);

    if (!bfs::exists(m_dbPath))
    {
        *this = std::move(camDb);
        return;
    }

    std::ifstream ifs(m_dbPath.c_str());

    if (!ifs)
    {
        std::ostringstream oss;
        oss << "failed to create std::ifstream to: " << m_dbPath;
        BOOST_THROW_EXCEPTION(std::runtime_error(oss.str()));
    }

    core_lib::serialize::archives::in_port_bin_t ia(ifs);
    ia(CEREAL_NVP(camDb));
    *this = std::move(camDb);
}

QRect CreateQRectFromVideoFrameDims(int const videoFrameWidth, int const videoFrameHeight,
                                    IpCamera::region_t const& motionRegion)
{
    QRect r;
    r.setTop(static_cast<int>(static_cast<double>(videoFrameHeight) * motionRegion.first.second));
    r.setLeft(static_cast<int>(static_cast<double>(videoFrameWidth) * motionRegion.first.first));
    r.setRight(
        static_cast<int>(static_cast<double>(r.left()) +
                         (static_cast<double>(videoFrameWidth) * motionRegion.second.first)));
    r.setBottom(
        static_cast<int>(static_cast<double>(r.top()) +
                         (static_cast<double>(videoFrameHeight) * motionRegion.second.second)));
    return r;
}

} // namespace ipfreely
