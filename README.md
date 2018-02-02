# IP Freely (RTSP Stream Viewer and Recorder) #
## Introduction ##
This project implements a hopefully useful cross-platform application to view and record from up to 4 IP camera live RTSP streams. The application also allows you to view a camera's on-board storage (e.g. SD card) and download video content from the camera's on-board storage to your PC (or anywhere else your PC can browse to on your network, e.g. a NAS).

It is licensed under the GNU Lesser General Public License and the relevant documentation for this can be found at the top of each source file and the LICENSE text file.

The code is the work of Duncan Crutchley (<dac1976github@outlook.com>).

Copyright (C) 2018 Duncan Crutchley.

## Key Features ##
* Multi-threaded.
* Cross-platform (Windows and Linux).
* Supports up to 4 user configurable RTSP camera streams displayed in a 2x2 grid.
* Clean intuitive UI.
* If a suitable URL is provided then you can view a camera's on-board storage and download the content to your PC.
* Still snapshot images can be taken from the camera feeds at any time with the click of a button.
* Local MP4 video recordings can be made from the camera sreams at the click of button. 
* Scheduled recording can be setup and enabled on a per camera basis.
* User can view larger expanded view from any of the 4 streams.

## Requirements ##
This application has been developed in Qt Creator and tested for use in Windows 10 and Linux (Ubuntu 17.10). In Windows it is recommended to use the MSVC2015 64bit tool chain from within Qt Creator to build and test the code. In Linux the code has been tested with GCC 7.2.0 but other C++11 compilers should be fine, such as Clang.

The project supplied for the application is for Qt Creator and will work in Linux and Windows. This project can be customised to suit the user's own environment and paths.

You may use this code within other development and build environments but it is up to the user to create the required projects or makefiles for those environments. This should be fairly easy by referring to the Qt project (.pro) file.

This library requires some third-party open source libraries for it to compile and function. These are as follows:

* Boost (tested with 1.65.1+ but newest version recommended): http://www.boost.org/
* Cereal (tested with 1.2.2 but newest version recommended): http://uscilab.github.io/cereal/
* Loki: http://loki-lib.sourceforge.net/
* CoreLibrary (tested with 1.4.4.0): https://github.com/dac1976/CoreLibrary
* Qt Framework (tested with 5.10.0): http://www.qt.io
* Single Application: https://github.com/itay-grudev/SingleApplication
* OpenCV (tested with 3.4.0): https://opencv.org/releases.html

Please note that some of these libraries themselves require other dependencies, so please refer to their documentation.

## Notes ##
I started this project shortly after buying and installing some RSTP compatible IP security cameras at my house. The software that came with the cameras was adequate but not great; relying on ActiveX and Internet Explorer. Instead this project provides a native application to view and record up to cameras' streams. Currently, this application does not give you any control over the IP cameras' on-board settings.

I will fix bugs and improve the code as and when necessary but make no guarantees on how often this happens. I provide no warranty or support for any issues that are encountered while using it. Although if you are really stuck email me at the provided address and if I have the time I will try to help/fix the issue if it's within my power.

The code is commented using Doxygen style comments so check the documentation in the /docs/html/ folder and open index.html in your browser of choice.

## To-do List ##
* Automatic, user-configurable disk space management of recorded videos.
* Motion-tracking triggered recording.
* Email alerts.

