# IP Freely (IP/Web camera stream viewer and recorder) #
## Introduction ##
This project implements a hopefully useful cross-platform application to view and record from up to 4 IP cameras' live RTSP and HTTP(S) streams. You can also connect to local web cameras. The application also allows you to view a camera's on-board storage (e.g. SD card) and download video content from the camera's on-board storage to your PC (or anywhere else your PC can browse to on your network, e.g. a NAS).

It is licensed under the GNU Lesser General Public License 3.0 and the relevant documentation for this can be found at the top of each source file and the LICENSE text file.

The code is the work of Duncan Crutchley (<dac1976github@outlook.com>).

Copyright (C) 2018 Duncan Crutchley.

## Background ##
I started this project shortly after buying and installing some RTSP compatible IP security cameras at my house. The software that came with the cameras was adequate but not great; relying on ActiveX and Internet Explorer. Instead this project provides a native application to view and record up to 4 cameras' streams. Currently, this application does not give you any control over the IP cameras' on-board settings.

## Key Features (Current Version 1.1.5.0) ##
* Clean and intuitive UI, hopefully!
* Multi-threaded.
* Cross-platform (Windows and Linux).
* Supports up to 4 user configurable IP or web camera streams displayed in a 2x2 grid.
* If a suitable URL is provided then you can view a camera's on-board storage (e.g. SD card) and download the content to your PC.
* The user can view a larger expanded view from any of the 4 streams.
* Still snapshot images can be taken from the camera feeds at any time with the click of a button.
* Local AVI (DivX on Windows, XDiv on Linux) video recordings can be made from the camera streams at the click of button.
* Scheduled recording can be setup and enabled on a per camera basis, with the schedule allowing selection of days and active hours in the day.
* Motion detection can be setup with user-configurable scheduling (similar to scheduled recordings). 
* Per camera user definable motion detection regions.
* Per camera motion detection algorithm sensitivity (off, low sensitivity, medium sensitivity, high sensitivity and manual settings).
* Built-in disk space manager. User can configure how many days recordings to keep and a maximum percentage of used disk space. The disk manager periodically i nthe background will remove oldest data first and ensures used space always falls within defined limits.
* (Planned) Motion triggered email send email alerts. 
* (Planned) Built-in web server to display some basic features, such as periodically updated snapshots from the camera feeds.

## Screen-shots ##
Taken from release 1.1.5.0.

### Preferences ###

![alt text][pic04]

![alt text][pic05]

![alt text][pic08]

### Camera Setup ###

![alt text][pic03]

### Main Screen ###

![alt text][pic01]

![alt text][pic09]

![alt text][pic02]

![alt text][pic06]

![alt text][pic07]

[pic01]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic01.png "Main screen displaying one camera."
[pic02]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic02.png "Expanded view of camera 1's feed."
[pic03]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic03.png "Camera setup dialog."
[pic04]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic04.png "General preferences."
[pic05]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic05.png "Recording schedule preferences."
[pic06]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic06.png "Camera storage browser."
[pic07]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic07.png "Camera stream recording."
[pic08]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic08.png "Motion detection schedule."
[pic09]: https://github.com/dac1976/IP-Freely/blob/master/Images/pic09.png "Motion region setup."

## Build Requirements ##
This application has been developed in Qt Creator and tested for use in Windows 10 and Linux (Ubuntu 17.10). In Windows it is recommended to use the MSVC2015 64bit tool chain from within Qt Creator to build and test the code. In Linux the code has been tested with GCC 7.2.0 but other C++11 compilers should be fine, such as Clang.

The project supplied for the application is for Qt Creator and will work in Linux and Windows. This project can be customized to suit the user's own environment and paths.

You may use this code within other supported development and build environments but it is up to the user to create the required projects or makefiles for those environments. This should be fairly easy by referring to the Qt project (.pro) file.

This library requires some third-party open source libraries for it to compile and function. These are as follows:
* Boost (tested with 1.65.1 and 1.66.0 but newest version recommended): http://www.boost.org/
* Cereal (tested with 1.2.2 but newest version recommended): http://uscilab.github.io/cereal/
* Loki: http://loki-lib.sourceforge.net/
* CoreLibrary (requires with 1.4.6.0 but newest version recommended): https://github.com/dac1976/CoreLibrary
* Qt Framework (tested with 5.10 and 5.10.1 but should work with any 5.X version): http://www.qt.io
* Single Application: https://github.com/itay-grudev/SingleApplication
* OpenCV (tested with 3.4.0 but other 3.X versions should work): https://opencv.org/releases.html

Please note that some of these libraries themselves require other dependencies, so please refer to their documentation.

## Notes ##
I will fix bugs and improve the code as and when necessary but make no guarantees on how often this happens. I provide no warranty or support for any issues that are encountered while using it. Although if you are really stuck email me at the provided address and if I have the time I will try to help/fix the issue if it's within my power.

## To-do List ##
See my Trello [board](https://trello.com/b/jBeGcZ0k).
