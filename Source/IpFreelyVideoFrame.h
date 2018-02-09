#ifndef IPFREELYVIDEOFRAME_H
#define IPFREELYVIDEOFRAME_H

#include <QFrame>
#include <QPoint>
#include <functional>

namespace Ui
{
class IpFreelyVideoFrame;
}

class QMouseEvent;
class QImage;
class QRubberBand;
class QRectF;

class IpFreelyVideoFrame : public QFrame
{
    Q_OBJECT

    typedef std::function<void(int const, QRectF const&)> selection_callback_t;

public:
    IpFreelyVideoFrame(int const cameraId, selection_callback_t const& selectionCallback,
                       QWidget* parent = nullptr);
    virtual ~IpFreelyVideoFrame();

    void SetVideoFrame(QImage const& videoFrame);
    void SetEnableSelection(bool const enable);

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::IpFreelyVideoFrame* ui;
    int                     m_cameraId;
    selection_callback_t    m_selectionCallback;
    QPoint                  m_origin;
    QRubberBand*            m_rubberBand;
    bool                    m_enableSelection;
};

#endif // IPFREELYVIDEOFRAME_H
