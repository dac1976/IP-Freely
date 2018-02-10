#include "IpFreelyVideoFrame.h"
#include "ui_IpFreelyVideoFrame.h"
#include <QMouseEvent>
#include <QImage>
#include <QRubberBand>
#include <QRect>
#include <QRectF>
#include <QSize>

IpFreelyVideoFrame::IpFreelyVideoFrame(int const                   cameraId,
                                       selection_callback_t const& selectionCallback,
                                       QWidget*                    parent)
    : QFrame(parent)
    , ui(new Ui::IpFreelyVideoFrame)
    , m_cameraId(cameraId)
    , m_selectionCallback(selectionCallback)
    , m_rubberBand(nullptr)
    , m_enableSelection(false)
{
    ui->setupUi(this);
}

IpFreelyVideoFrame::~IpFreelyVideoFrame()
{
    delete ui;
}

void IpFreelyVideoFrame::SetVideoFrame(QImage const& videoFrame)
{
    ui->videoFrameLabel->setPixmap(QPixmap::fromImage(videoFrame));
}

void IpFreelyVideoFrame::SetEnableSelection(bool const enable)
{
    m_enableSelection = enable;
}

void IpFreelyVideoFrame::mousePressEvent(QMouseEvent* event)
{
    if (!m_enableSelection)
    {
        return;
    }

    m_origin = event->pos();

    if (!m_rubberBand)
    {
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    }

    m_rubberBand->setGeometry(QRect(m_origin, QSize()));
    m_rubberBand->show();
}

void IpFreelyVideoFrame::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_enableSelection || !m_rubberBand)
    {
        return;
    }

    m_rubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
}

void IpFreelyVideoFrame::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (!m_enableSelection || !m_selectionCallback || !m_rubberBand)
    {
        return;
    }

    auto   selection = m_rubberBand->geometry();
    double t = static_cast<double>(selection.top()) / static_cast<double>(geometry().height());
    double l = static_cast<double>(selection.left()) / static_cast<double>(geometry().width());
    double h = static_cast<double>(selection.height()) / static_cast<double>(geometry().height());
    double w = static_cast<double>(selection.width()) / static_cast<double>(geometry().width());

    m_selectionCallback(m_cameraId, QRectF(l, t, w, h));

    m_rubberBand->deleteLater();
}
