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
 * \file main.cpp
 * \brief File containing definition of main entry point.
 */
#include <boost/predef.h>

#if BOOST_OS_WINDOWS
#include <Windows.h>
#endif

#include <sstream>
#include <memory>
#include <string>
#include <cstring>
#include <QApplication>
#include <QLocalSocket>
#include <boost/exception/all.hpp>
#include "DebugLog/DebugLogging.h"
#include "singleapplication.h"
#include "IpFreelyMainWindow.h"

#if BOOST_OS_WINDOWS
// Link to version.dll using the lib from the Windows SDK.
#pragma comment(lib, "version")

std::string GetAppVersion(const std::string& appFilePath)
{
    std::wstring appFilePathW(appFilePath.begin(), appFilePath.end());
    const DWORD  blockSize = GetFileVersionInfoSize(appFilePathW.c_str(), NULL);

    std::string                              appVersion;
    auto                                     deleter = [](BYTE const* p) { delete[] p; };
    std::unique_ptr<BYTE, decltype(deleter)> block(new BYTE[blockSize], deleter);
    LPVOID                                   pBlock = reinterpret_cast<LPVOID>(block.get());

    if (GetFileVersionInfo(appFilePathW.c_str(), NULL, blockSize, pBlock))
    {
        LPVOID lpBuffer = NULL;
        UINT   len      = 0;

        if (VerQueryValue(pBlock, TEXT("\\"), &lpBuffer, &len))
        {
            const VS_FIXEDFILEINFO* fileInfo = reinterpret_cast<const VS_FIXEDFILEINFO*>(lpBuffer);

            std::stringstream ssVersion;
            ssVersion << ((fileInfo->dwFileVersionMS & 0xFFFF0000) >> 16) << "."
                      << (fileInfo->dwFileVersionMS & 0x0000FFFF) << "."
                      << ((fileInfo->dwFileVersionLS & 0xFFFF0000) >> 16) << "."
                      << (fileInfo->dwFileVersionLS & 0x0000FFFF);

            appVersion = ssVersion.str();
        }
    }

    return appVersion;
}
#else
#define IPFREELY_VERSION "1.1.4.0"
#endif

int main(int argc, char* argv[])
{
    int  retCode        = EXIT_SUCCESS;
    bool logInitialised = false;

    try
    {
        SingleApplication a(argc, argv);

#if BOOST_OS_WINDOWS
        QString appVersion =
            QString::fromStdString(GetAppVersion(a.applicationFilePath().toStdString()));
#else
        QString appVersion = IPFREELY_VERSION;
#endif
        a.setApplicationVersion(appVersion);

        DEBUG_MESSAGE_INSTANTIATE_EX(
            appVersion.toStdString(), "", "IpFreely", core_lib::log::BYTES_IN_MEBIBYTE * 25);

        logInitialised = true;

        IpFreelyMainWindow w(appVersion);
        DEBUG_MESSAGE_EX_INFO("Showing main form.");
        w.show();

        DEBUG_MESSAGE_EX_INFO("Executing application message loop.");
        retCode = a.exec();
    }
    catch (...)
    {
        auto exceptionMsg = boost::current_exception_diagnostic_information();

        if (logInitialised)
        {
            DEBUG_MESSAGE_EX_FATAL(exceptionMsg);
        }

        qFatal(exceptionMsg.c_str());
        retCode = EXIT_FAILURE;
    }

    if (logInitialised)
    {
        DEBUG_MESSAGE_EX_INFO("Application closing");
    }

    return retCode;
}
