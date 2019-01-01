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
 * \file IpFreelyMainWindow.h
 * \brief File containing declaration of IpFreelyMainWindow form.
 */
#ifndef IPFREELYMAINWINDOW_H
#define IPFREELYMAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPoint>
#include <memory>
#include <map>
#include "IpFreelyPreferences.h"
#include "IpFreelyCameraDatabase.h"

// Forward declarations.
namespace Ui
{
class IpFreelyMainWindow;
} // namespace Ui

namespace ipfreely
{
class IpFreelyStreamProcessor;
class IpFreelyDiskSpaceManager;
} // namespace ipfreely

class QToolButton;
class QTimer;
class QResizeEvent;
class QCloseEvent;
class QGroupBox;
class IpFreelyVideoFrame;
class IpFreelyVideoForm;
class QRectF;

/*! \brief Class defining the main window's form. */
class IpFreelyMainWindow : public QMainWindow
{
    Q_OBJECT

    typedef std::shared_ptr<ipfreely::IpFreelyStreamProcessor> stream_proc_t;

public:
    /*!
     * \brief IpFreelyMainWindow constructor.
     * \param[in] parent - (Optional) Pointer to parent widget.
     */
    explicit IpFreelyMainWindow(QString const& appVersion, QWidget* parent = nullptr);

    /*! \brief IpFreelyVideoForm destructor. */
    ~IpFreelyMainWindow();

private slots:
    void on_actionClose_triggered();
    void on_actionPreferences_triggered();
    void on_actionAbout_triggered();
    void on_settings1ToolButton_clicked();
    void on_connect1ToolButton_clicked();
    void on_motionDetectorRegions1ToolButton_toggled(bool checked);
    void on_removeMotionRegions1ToolButton_clicked();
    void on_record1ToolButton_clicked();
    void on_snapshot1ToolButton_clicked();
    void on_expand1ToolButton_clicked();
    void on_storage1ToolButton_clicked();
    void on_settings2ToolButton_clicked();
    void on_connect2ToolButton_clicked();
    void on_motionDetectorRegions2ToolButton_toggled(bool checked);
    void on_removeMotionRegions2ToolButton_clicked();
    void on_record2ToolButton_clicked();
    void on_snapshot2ToolButton_clicked();
    void on_expand2ToolButton_clicked();
    void on_storage2ToolButton_clicked();
    void on_settings3ToolButton_clicked();
    void on_connect3ToolButton_clicked();
    void on_motionDetectorRegions3ToolButton_toggled(bool checked);
    void on_removeMotionRegions3ToolButton_clicked();
    void on_record3ToolButton_clicked();
    void on_snapshot3ToolButton_clicked();
    void on_expand3ToolButton_clicked();
    void on_storage3ToolButton_clicked();
    void on_settings4ToolButton_clicked();
    void on_connect4ToolButton_clicked();
    void on_motionDetectorRegions4ToolButton_toggled(bool checked);
    void on_removeMotionRegions4ToolButton_clicked();
    void on_record4ToolButton_clicked();
    void on_snapshot4ToolButton_clicked();
    void on_expand4ToolButton_clicked();
    void on_storage4ToolButton_clicked();
    void on_updateFeedsTimer();

protected:
    virtual void closeEvent(QCloseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

private:
    void     SetDisplaySize();
    void     ConnectButtons();
    void     CheckStartupConnections();
    void     SetupCameraInDb(ipfreely::eCamId const camId, QToolButton* connectBtn);
    void     ConnectionHandler(ipfreely::IpCamera const& camera, QToolButton* connectBtn,
                               QToolButton* motionRegionsBtn, QToolButton* removeRegionsBtn,
                               QToolButton* recordBtn, QToolButton* snapshotBtn, QToolButton* expandBtn,
                               QToolButton* storageBtn);
    void     RecordActionHandler(ipfreely::eCamId const camId, QToolButton* recordBtn);
    QWidget* GetParentFrame(ipfreely::eCamId const camId) const;
    void     UpdateCamFeedFrame(ipfreely::eCamId const camId, QImage const& videoFrame,
                                QRect const& motionBoundingRect, bool const streamProcIsWriting);
    void     SaveImageSnapshot(ipfreely::eCamId const camId);
    void     SetFpsInTitle(ipfreely::eCamId const camId, double fps, double originalFps);
    void     ShowExpandedVideoForm(ipfreely::eCamId const camId);
    void     ViewStorage(ipfreely::IpCamera const& camera);
    void     VideoFrameAreaSelection(int const cameraId, QRectF const& percentageSelection);
    void     EnableMotionRegionsSetup(ipfreely::eCamId const camId, bool const enable,
                                      QToolButton* removeRegionsBtn, QToolButton* setMotionRegionsBtn);
    void     RemoveMotionRegions(ipfreely::eCamId const camId);
    void     ReconnectCamera(ipfreely::eCamId const camId);

private:
    Ui::IpFreelyMainWindow*                                   ui;
    QString                                                   m_appVersion;
    ipfreely::IpFreelyPreferences                             m_prefs;
    ipfreely::IpFreelyCameraDatabase                          m_camDb;
    QTimer*                                                   m_updateFeedsTimer;
    int                                                       m_numConnections;
    std::shared_ptr<IpFreelyVideoForm>                        m_videoForm;
    ipfreely::eCamId                                          m_videoFormId;
    std::map<ipfreely::eCamId, IpFreelyVideoFrame*>           m_camFeeds;
    std::map<ipfreely::eCamId, ipfreely::IpCamera::regions_t> m_camMotionRegions;
    std::map<ipfreely::eCamId, bool>                          m_motionAreaSetupEnabled;
    std::map<ipfreely::eCamId, stream_proc_t>                 m_streamProcessors;
    std::shared_ptr<ipfreely::IpFreelyDiskSpaceManager>       m_diskSpaceMgr;
};

#endif // IPFREELYMAINWINDOW_H
