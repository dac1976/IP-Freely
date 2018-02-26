#-------------------------------------------------
#
# Project created by QtCreator 2018-01-20T22:41:57
#
#-------------------------------------------------

QT       += core gui network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IpFreely
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += core_lib_dll

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# On Windows we do this, assumes we'll be using MS VC 2015.
win32 {
    # disable incremental linking with debug builds
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO

    # Due to exporting from DLL we might get suprious warnings of
    # type 4251, 4275 and 4100 so disable them.
    QMAKE_CXXFLAGS += /wd4251 /wd4275 /wd4100
    DEFINES += _CRT_SECURE_NO_WARNINGS=1

    INCLUDEPATH += $$(OPENCV_DIR)/../../include \
        $$(THIRD_PARTY_LIBS) \
        $$(THIRD_PARTY_LIBS)\singleapplication

    CONFIG(debug, debug|release) {
      LIBS += -L$$(OPENCV_DIR)/lib \
              -lopencv_world340d
      QMAKE_POST_LINK  = $$PWD/../WindowsBatchFiles/CopyDependencies_64Bit_Debug.bat
    } else {
      LIBS += -L$$(OPENCV_DIR)/lib \
              -lopencv_world340
      QMAKE_POST_LINK  = $$PWD/../WindowsBatchFiles/CopyDependencies_64Bit_Release.bat
    }

    SOURCES += \
        $$(THIRD_PARTY_LIBS)/singleapplication/singleapplication.cpp

    HEADERS += \
        $$(THIRD_PARTY_LIBS)/singleapplication/singleapplication.h

    # Set version info of our app and its icon.
    RC_FILE = IpFreely.rc

    # Other files that we want to view in the editor.
    DISTFILES += IpFreely.rc \
                 $$PWD/../WindowsBatchFiles/CopyDependencies_64Bit_Debug.bat \
                 $$PWD/../WindowsBatchFiles/CopyDependencies_64Bit_Release.bat
}
# On non-windows, assumed to be Linux, we do this.
else {
    # Make sure we enable C++14 support.
    QMAKE_CXXFLAGS += -std=c++14

    # Set version info for library.
    VERSION = 1.1.4

    INCLUDEPATH += /usr/local/include \
        /home/duncan/projects/ThirdParty \
        /home/duncan/projects/ThirdParty/singleapplication

    LIBS += -L/usr/local/lib   \
            -lopencv_core      \
            -lopencv_imgcodecs \
            -lopencv_imgproc   \
            -lopencv_video     \
            -lopencv_videoio #\
            #-lopencv_highgui

    SOURCES += \
        /home/duncan/projects/ThirdParty/singleapplication/singleapplication.cpp

    HEADERS += \
        /home/duncan/projects/ThirdParty/singleapplication/singleapplication.h
}

SOURCES += \
    main.cpp \
    IpFreelyMainWindow.cpp \
    IpFreelyCameraDatabase.cpp \
    IpFreelyVideoForm.cpp \
    IpFreelyAbout.cpp \
    IpFreelyPreferencesDialog.cpp \
    IpFreelyPreferences.cpp \
    IpFreelyCameraSetupDialog.cpp \
    IpFreelyDownloadWidget.cpp \
    IpFreelySdCardViewerDialog.cpp \
    IpFreelyStreamProcessor.cpp \
    IpFreelyMotionDetector.cpp \
    IpFreelyVideoFrame.cpp \
    IpFreelyDiskSpaceManager.cpp

HEADERS += \
    IpFreelyMainWindow.h \
    IpFreelyCameraDatabase.h \
    IpFreelyVideoForm.h \
    IpFreelyAbout.h \
    IpFreelyPreferencesDialog.h \
    IpFreelyPreferences.h \
    IpFreelyCameraSetupDialog.h \
    IpFreelyDownloadWidget.h \
    IpFreelySdCardViewerDialog.h \
    IpFreelyStreamProcessor.h \
    IpFreelyMotionDetector.h \
    IpFreelyVideoFrame.h \
    IpFreelyDiskSpaceManager.h

FORMS += \
    IpFreelyMainWindow.ui \
    IpFreelyVideoForm.ui \
    IpFreelyAbout.ui \
    IpFreelyPreferencesDialog.ui \
    IpFreelyCameraSetupDialog.ui \
    IpFreelyDownloadWidget.ui \
    IpFreelySdCardViewerDialog.ui \
    IpFreelyVideoFrame.ui

RESOURCES += \
    ipfreely.qrc
