/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// local
#include "application.h"
#include "mirbuffersgtexture.h"
#include "session.h"
#include "mirsurfaceitem.h"
#include "logging.h"
#include "ubuntukeyboardinfo.h"

// mirserver
#include "surfaceobserver.h"

// common
#include <debughelpers.h>

// Qt
#include <QDebug>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <private/qsgdefaultimagenode_p.h>
#include <QSGTextureProvider>
#include <QTimer>

// Mir
#include <mir/geometry/rectangle.h>
#include <mir/events/event_builders.h>
#include <mir_toolkit/event.h>
#include <mir/shell/shell.h>

namespace mg = mir::graphics;

namespace qtmir {

namespace {

// Would be better if QMouseEvent had nativeModifiers
MirInputEventModifiers
getMirModifiersFromQt(Qt::KeyboardModifiers mods)
{
    MirInputEventModifiers m_mods = mir_input_event_modifier_none;
    if (mods & Qt::ShiftModifier)
        m_mods |= mir_input_event_modifier_shift;
    if (mods & Qt::ControlModifier)
        m_mods |= mir_input_event_modifier_ctrl;
    if (mods & Qt::AltModifier)
        m_mods |= mir_input_event_modifier_alt;
    if (mods & Qt::MetaModifier)
        m_mods |= mir_input_event_modifier_meta;

    return m_mods;
}

mir::EventUPtr makeMirEvent(QMouseEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = std::chrono::milliseconds(qtEvent->timestamp());
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());

    MirPointerButtons buttons = 0;
    if (qtEvent->buttons() & Qt::LeftButton)
        buttons |= mir_pointer_button_primary;
    if (qtEvent->buttons() & Qt::RightButton)
        buttons |= mir_pointer_button_secondary;
    if (qtEvent->buttons() & Qt::MidButton)
        buttons |= mir_pointer_button_tertiary;

    return mir::events::make_event(0 /*DeviceID */, timestamp, modifiers, action,
                                   buttons, qtEvent->x(), qtEvent->y(), 0, 0);
}

mir::EventUPtr makeMirEvent(QHoverEvent *qtEvent, MirPointerAction action)
{
    auto timestamp = std::chrono::milliseconds(qtEvent->timestamp());

    MirPointerButtons buttons = 0;

    return mir::events::make_event(0 /*DeviceID */, timestamp, mir_input_event_modifier_none, action,
                                   buttons, qtEvent->posF().x(), qtEvent->posF().y(), 0, 0);
}

mir::EventUPtr makeMirEvent(QKeyEvent *qtEvent)
{
    MirKeyboardAction action = mir_keyboard_action_down;
    switch (qtEvent->type())
    {
    case QEvent::KeyPress:
        action = mir_keyboard_action_down;
        break;
    case QEvent::KeyRelease:
        action = mir_keyboard_action_up;
        break;
    default:
        break;
    }
    if (qtEvent->isAutoRepeat())
        action = mir_keyboard_action_repeat;

    return mir::events::make_event(0 /* DeviceID */, std::chrono::milliseconds(qtEvent->timestamp()),
                           action, qtEvent->nativeVirtualKey(),
                           qtEvent->nativeScanCode(),
                           qtEvent->nativeModifiers());
}

mir::EventUPtr makeMirEvent(Qt::KeyboardModifiers qmods,
                            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                            Qt::TouchPointStates /* qtTouchPointStates */,
                            ulong qtTimestamp)
{
    auto modifiers = getMirModifiersFromQt(qmods);
    auto ev = mir::events::make_event(0, std::chrono::milliseconds(qtTimestamp),
                                      modifiers);

    for (int i = 0; i < qtTouchPoints.count(); ++i) {
        auto touchPoint = qtTouchPoints.at(i);
        auto id = touchPoint.id();

        MirTouchAction action = mir_touch_action_change;
        if (touchPoint.state() == Qt::TouchPointReleased)
        {
            action = mir_touch_action_up;
        }
        if (touchPoint.state() == Qt::TouchPointPressed)
        {
            action = mir_touch_action_down;
        }

        MirTouchTooltype tooltype = mir_touch_tooltype_finger;
        if (touchPoint.flags() & QTouchEvent::TouchPoint::Pen)
            tooltype = mir_touch_tooltype_stylus;

        mir::events::add_touch(*ev, id, action, tooltype,
                               touchPoint.pos().x(), touchPoint.pos().y(),
                               touchPoint.pressure(),
                               touchPoint.rect().width(),
                               touchPoint.rect().height(),
                               0 /* size */);
    }

    return ev;
}

} // namespace {

class QMirSurfaceTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    QMirSurfaceTextureProvider() : t(0) { }
    ~QMirSurfaceTextureProvider() { delete t; }

    QSGTexture *texture() const {
        if (t)
            t->setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return t;
    }

    bool smooth;
    MirBufferSGTexture *t;

public Q_SLOTS:
    void invalidate()
    {
        delete t;
        t = 0;
    }
};

MirSurfaceItem::MirSurfaceItem(std::shared_ptr<mir::scene::Surface> surface,
                               SessionInterface* session,
                               MirShell *shell,
                               std::shared_ptr<SurfaceObserver> observer,
                               QQuickItem *parent)
    : MirSurfaceItemInterface(parent)
    , m_surface(surface)
    , m_session(session)
    , m_shell(shell)
    , m_firstFrameDrawn(false)
    , m_live(true)
    , m_orientationAngle(Angle0)
    , m_textureProvider(nullptr)
    , m_lastTouchEvent(nullptr)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::MirSurfaceItem";

    m_surfaceObserver = observer;
    if (observer) {
        connect(observer.get(), &SurfaceObserver::framesPosted, this, &MirSurfaceItem::surfaceDamaged);
        connect(observer.get(), &SurfaceObserver::attributeChanged, this, &MirSurfaceItem::onAttributeChanged);
        observer->setListener(this);
    }

    setSmooth(true);
    setFlag(QQuickItem::ItemHasContents, true); //so scene graph will render this item
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
        Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
        Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
        Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
        Qt::ExtraButton12 | Qt::ExtraButton13);
    setAcceptHoverEvents(true);

    // fetch surface geometry
    setImplicitSize(static_cast<qreal>(m_surface->size().width.as_float()),
                    static_cast<qreal>(m_surface->size().height.as_float()));

    if (!UbuntuKeyboardInfo::instance()) {
        new UbuntuKeyboardInfo;
    }

    // Ensure C++ (MirSurfaceManager) retains ownership of this object
    // TODO: Investigate if having the Javascript engine have ownership of this object
    // might create a less error-prone API design (concern: QML forgets to call "release()"
    // for a surface, and thus Mir will not release the surface buffers etc.)
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    connect(&m_frameDropperTimer, &QTimer::timeout,
            this, &MirSurfaceItem::dropPendingBuffer);
    // Rationale behind the frame dropper and its interval value:
    //
    // We want to give ample room for Qt scene graph to have a chance to fetch and render
    // the next pending buffer before we take the drastic action of dropping it (so don't set
    // it anywhere close to our target render interval).
    //
    // We also want to guarantee a minimal frames-per-second (fps) frequency for client applications
    // as they get stuck on swap_buffers() if there's no free buffer to swap to yet (ie, they
    // are all pending consumption by the compositor, us). But on the other hand, we don't want
    // that minimal fps to be too high as that would mean this timer would be triggered way too often
    // for nothing causing unnecessary overhead as actually dropping frames from an app should
    // in practice rarely happen.
    m_frameDropperTimer.setInterval(200);
    m_frameDropperTimer.setSingleShot(false);

    m_updateMirSurfaceSizeTimer.setSingleShot(true);
    m_updateMirSurfaceSizeTimer.setInterval(1);
    connect(&m_updateMirSurfaceSizeTimer, &QTimer::timeout, this, &MirSurfaceItem::updateMirSurfaceSize);
    connect(this, &QQuickItem::widthChanged, this, &MirSurfaceItem::scheduleMirSurfaceSizeUpdate);
    connect(this, &QQuickItem::heightChanged, this, &MirSurfaceItem::scheduleMirSurfaceSizeUpdate);

    // FIXME - setting surface unfocused immediately breaks camera & video apps, but is
    // technically the correct thing to do (surface should be unfocused until shell focuses it)
    //m_surface->configure(mir_surface_attrib_focus, mir_surface_unfocused);
    connect(this, &QQuickItem::activeFocusChanged, this, &MirSurfaceItem::updateMirSurfaceFocus);

    if (m_session) {
        connect(m_session.data(), &Session::stateChanged, this, &MirSurfaceItem::onSessionStateChanged);
    }
}

MirSurfaceItem::~MirSurfaceItem()
{
    if (m_session) {
        m_session->setSurface(nullptr);
    }

    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::~MirSurfaceItem - this=" << this;
    QMutexLocker locker(&m_mutex);
    m_surface->remove_observer(m_surfaceObserver);
    if (m_textureProvider)
        m_textureProvider->deleteLater();

    delete m_lastTouchEvent;
}

// For QML to destroy this surface
void MirSurfaceItem::release()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::release - this=" << this;

    if (m_session) {
        m_session->setSurface(nullptr);
    }
    deleteLater();
}

SessionInterface* MirSurfaceItem::session() const
{
    return m_session.data();
}

MirSurfaceItem::Type MirSurfaceItem::type() const
{
    return static_cast<MirSurfaceItem::Type>(m_surface->type());
}

MirSurfaceItem::State MirSurfaceItem::state() const
{
    return static_cast<MirSurfaceItem::State>(m_surface->state());
}

MirSurfaceItem::OrientationAngle MirSurfaceItem::orientationAngle() const
{
    return m_orientationAngle;
}

void MirSurfaceItem::setOrientationAngle(MirSurfaceItem::OrientationAngle angle)
{
    qCDebug(QTMIR_SURFACES, "MirSurfaceItem::setOrientationAngle(%d)", angle);

    if (m_orientationAngle == angle)
        return;

    MirOrientation mirOrientation;

    switch (angle) {
    case Angle0:
        mirOrientation = mir_orientation_normal;
        break;
    case Angle90:
        mirOrientation = mir_orientation_right;
        break;
    case Angle180:
        mirOrientation = mir_orientation_inverted;
        break;
    case Angle270:
        mirOrientation = mir_orientation_left;
        break;
    default:
        qCWarning(QTMIR_SURFACES, "Unsupported orientation angle: %d", angle);
        return;
    }

    m_surface->set_orientation(mirOrientation);

    m_orientationAngle = angle;
    Q_EMIT orientationAngleChanged(angle);
}

QString MirSurfaceItem::name() const
{
    //FIXME - how to listen to change in this property?
    return QString::fromStdString(m_surface->name());
}

bool MirSurfaceItem::live() const
{
    return m_live;
}

// Called from the rendering (scene graph) thread
QSGTextureProvider *MirSurfaceItem::textureProvider() const
{
    const_cast<MirSurfaceItem *>(this)->ensureProvider();
    return m_textureProvider;
}

void MirSurfaceItem::ensureProvider()
{
    if (!m_textureProvider) {
        m_textureProvider = new QMirSurfaceTextureProvider();
        connect(window(), SIGNAL(sceneGraphInvalidated()),
                m_textureProvider, SLOT(invalidate()), Qt::DirectConnection);
    }
}

void MirSurfaceItem::surfaceDamaged()
{
    if (!m_firstFrameDrawn) {
        m_firstFrameDrawn = true;
        Q_EMIT firstFrameDrawn();
    }

    scheduleTextureUpdate();
}

bool MirSurfaceItem::updateTexture()    // called by rendering thread (scene graph)
{
    QMutexLocker locker(&m_mutex);
    ensureProvider();
    bool textureUpdated = false;

    const void* const userId = (void*)123;
    auto renderables = m_surface->generate_renderables(userId);

    if (m_surface->buffers_ready_for_compositor(userId) > 0 && renderables.size() > 0) {
        if (!m_textureProvider->t) {
            m_textureProvider->t = new MirBufferSGTexture(renderables[0]->buffer());
        } else {
            // Avoid holding two buffers for the compositor at the same time. Thus free the current
            // before acquiring the next
            m_textureProvider->t->freeBuffer();
            m_textureProvider->t->setBuffer(renderables[0]->buffer());
        }
        textureUpdated = true;
    }

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        QTimer::singleShot(0, this, SLOT(update()));
        // restart the frame dropper so that we have enough time to render the next frame.
        // queued since the timer lives in a different thread
        QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
    }

    m_textureProvider->smooth = smooth();

    return textureUpdated;
}

QSGNode *MirSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)    // called by render thread
{
    if (!m_surface) {
        delete oldNode;
        return 0;
    }

    bool textureUpdated = updateTexture();
    if (!m_textureProvider->t) {
        delete oldNode;
        return 0;
    }

    QSGDefaultImageNode *node = static_cast<QSGDefaultImageNode*>(oldNode);
    if (!node) {
        node = new QSGDefaultImageNode;
        node->setTexture(m_textureProvider->t);

        node->setMipmapFiltering(QSGTexture::None);
        node->setHorizontalWrapMode(QSGTexture::ClampToEdge);
        node->setVerticalWrapMode(QSGTexture::ClampToEdge);
        node->setSubSourceRect(QRectF(0, 0, 1, 1));
    } else {
        if (textureUpdated) {
            node->markDirty(QSGNode::DirtyMaterial);
        }
    }

    node->setTargetRect(QRectF(0, 0, width(), height()));
    node->setInnerTargetRect(QRectF(0, 0, width(), height()));

    node->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    node->setAntialiasing(antialiasing());

    node->update();

    return node;
}

void MirSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    if (type() == InputMethod) {
        // FIXME: Hack to get the VKB use case working while we don't have the proper solution in place.
        if (isMouseInsideUbuntuKeyboard(event)) {
            auto ev = makeMirEvent(event, mir_pointer_action_button_down);
            m_surface->consume(*ev);
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        auto ev = makeMirEvent(event, mir_pointer_action_button_down);
        m_surface->consume(*ev);
        event->accept();
    }
}

void MirSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_button_up);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurfaceItem::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
}

void MirSurfaceItem::hoverEnterEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_enter);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurfaceItem::hoverLeaveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_leave);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurfaceItem::hoverMoveEvent(QHoverEvent *event)
{
    auto ev = makeMirEvent(event, mir_pointer_action_motion);
    m_surface->consume(*ev);
    event->accept();
}

void MirSurfaceItem::keyPressEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(*ev);
    qtEvent->accept();
}

void MirSurfaceItem::keyReleaseEvent(QKeyEvent *qtEvent)
{
    auto ev = makeMirEvent(qtEvent);
    m_surface->consume(*ev);
    qtEvent->accept();
}

QString MirSurfaceItem::appId() const
{
    QString appId;
    if (session() && session()->application()) {
        appId = session()->application()->appId();
    } else {
        appId.append("-");
    }
    return appId;
}

void MirSurfaceItem::endCurrentTouchSequence(ulong timestamp)
{
    Q_ASSERT(m_lastTouchEvent);
    Q_ASSERT(m_lastTouchEvent->type != QEvent::TouchEnd);
    Q_ASSERT(m_lastTouchEvent->touchPoints.count() > 0);

    TouchEvent touchEvent = *m_lastTouchEvent;
    touchEvent.timestamp = timestamp;

    // Remove all already released touch points
    int i = 0;
    while (i < touchEvent.touchPoints.count()) {
        if (touchEvent.touchPoints[i].state() == Qt::TouchPointReleased) {
            touchEvent.touchPoints.removeAt(i);
        } else {
            ++i;
        }
    }

    // And release the others one by one as Mir expects one press/release per event
    while (touchEvent.touchPoints.count() > 0) {
        touchEvent.touchPoints[0].setState(Qt::TouchPointReleased);

        touchEvent.updateTouchPointStatesAndType();

        auto ev = makeMirEvent(touchEvent.modifiers, touchEvent.touchPoints,
                               touchEvent.touchPointStates, touchEvent.timestamp);
        m_surface->consume(*ev);

        *m_lastTouchEvent = touchEvent;

        touchEvent.touchPoints.removeAt(0);
    }
}

void MirSurfaceItem::validateAndDeliverTouchEvent(int eventType,
            ulong timestamp,
            Qt::KeyboardModifiers mods,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates)
{
    if (eventType == QEvent::TouchBegin && m_lastTouchEvent && m_lastTouchEvent->type != QEvent::TouchEnd) {
        qCWarning(QTMIR_SURFACES) << qPrintable(QString("MirSurfaceItem(%1) - Got a QEvent::TouchBegin while "
            "there's still an active/unfinished touch sequence.").arg(appId()));
        // Qt forgot to end the last touch sequence. Let's do it ourselves.
        endCurrentTouchSequence(timestamp);
    }

    auto ev = makeMirEvent(mods, touchPoints, touchPointStates, timestamp);
    m_surface->consume(*ev);

    if (!m_lastTouchEvent) {
        m_lastTouchEvent = new TouchEvent;
    }
    m_lastTouchEvent->type = eventType;
    m_lastTouchEvent->timestamp = timestamp;
    m_lastTouchEvent->touchPoints = touchPoints;
    m_lastTouchEvent->touchPointStates = touchPointStates;
}

void MirSurfaceItem::touchEvent(QTouchEvent *event)
{
    bool accepted = processTouchEvent(event->type(),
            event->timestamp(),
            event->modifiers(),
            event->touchPoints(),
            event->touchPointStates());
    event->setAccepted(accepted);
}

bool MirSurfaceItem::processTouchEvent(
        int eventType,
        ulong timestamp,
        Qt::KeyboardModifiers mods,
        const QList<QTouchEvent::TouchPoint> &touchPoints,
        Qt::TouchPointStates touchPointStates)
{
    bool accepted = true;
    if (type() == InputMethod && eventType == QEvent::TouchBegin) {
        // FIXME: Hack to get the VKB use case working while we don't have the proper solution in place.
        if (hasTouchInsideUbuntuKeyboard(touchPoints)) {
            validateAndDeliverTouchEvent(eventType, timestamp, mods, touchPoints, touchPointStates);
        } else {
            accepted = false;
        }

    } else {
        // NB: If we are getting QEvent::TouchUpdate or QEvent::TouchEnd it's because we've
        // previously accepted the corresponding QEvent::TouchBegin
        validateAndDeliverTouchEvent(eventType, timestamp, mods, touchPoints, touchPointStates);
    }
    return accepted;
}

bool MirSurfaceItem::hasTouchInsideUbuntuKeyboard(const QList<QTouchEvent::TouchPoint> &touchPoints)
{
    UbuntuKeyboardInfo *ubuntuKeyboardInfo = UbuntuKeyboardInfo::instance();

    for (int i = 0; i < touchPoints.count(); ++i) {
        QPoint pos = touchPoints.at(i).pos().toPoint();
        if (pos.x() >= ubuntuKeyboardInfo->x()
                && pos.x() <= (ubuntuKeyboardInfo->x() + ubuntuKeyboardInfo->width())
                && pos.y() >= ubuntuKeyboardInfo->y()
                && pos.y() <= (ubuntuKeyboardInfo->y() + ubuntuKeyboardInfo->height())) {
            return true;
        }
    }
    return false;
}

bool MirSurfaceItem::isMouseInsideUbuntuKeyboard(const QMouseEvent *event)
{
    UbuntuKeyboardInfo *ubuntuKeyboardInfo = UbuntuKeyboardInfo::instance();

    const QPointF &pos = event->localPos();

    return pos.x() >= ubuntuKeyboardInfo->x()
        && pos.x() <= (ubuntuKeyboardInfo->x() + ubuntuKeyboardInfo->width())
        && pos.y() >= ubuntuKeyboardInfo->y()
        && pos.y() <= (ubuntuKeyboardInfo->y() + ubuntuKeyboardInfo->height());
}

void MirSurfaceItem::setType(const Type &type)
{
    if (this->type() != type) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_type, static_cast<int>(type));
    }
}

void MirSurfaceItem::setState(const State &state)
{
    if (this->state() != state) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_state, static_cast<int>(state));
    }
}

void MirSurfaceItem::setLive(bool live)
{
    if (m_live != live) {
        m_live = live;
        Q_EMIT liveChanged(m_live);
    }
}

void MirSurfaceItem::onAttributeChanged(const MirSurfaceAttrib attribute, const int /*value*/)
{
    switch (attribute) {
    case mir_surface_attrib_type:
        Q_EMIT typeChanged();
        break;
    case mir_surface_attrib_state:
        Q_EMIT stateChanged();
        break;
    default:
        break;
    }
}

void MirSurfaceItem::scheduleMirSurfaceSizeUpdate()
{
    if (clientIsRunning() && !m_updateMirSurfaceSizeTimer.isActive()) {
        m_updateMirSurfaceSizeTimer.start();
    }
}

void MirSurfaceItem::updateMirSurfaceSize()
{
    int mirWidth = m_surface->size().width.as_int();
    int mirHeight = m_surface->size().height.as_int();

    int qmlWidth = (int)width();
    int qmlHeight = (int)height();

    bool mirSizeIsDifferent = qmlWidth != mirWidth || qmlHeight != mirHeight;

    const char *didResize = clientIsRunning() && mirSizeIsDifferent ? "surface resized" : "surface NOT resized";
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::updateMirSurfaceSize"
            << "surface =" << this
            << ", old (" << mirWidth << "," << mirHeight << ")"
            << ", new (" << qmlWidth << "," << qmlHeight << ")"
            << didResize;

    if (clientIsRunning() && mirSizeIsDifferent) {
        mir::geometry::Size newMirSize(qmlWidth, qmlHeight);
        m_surface->resize(newMirSize);
        setImplicitSize(qmlWidth, qmlHeight);
    }
}

void MirSurfaceItem::updateMirSurfaceFocus(bool focused)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::updateMirSurfaceFocus" << focused;

    // Temporary hotfix for http://pad.lv/1483752
    if (session() && session()->childSessions()->rowCount() > 0) {
        // has child trusted session, ignore any focus change attempts
        return;
    }

    if (focused) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_focused);
    } else {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_unfocused);
    }
}

void MirSurfaceItem::dropPendingBuffer()
{
    QMutexLocker locker(&m_mutex);

    const void* const userId = (void*)123;  // TODO: Multimonitor support

    int framesPending = m_surface->buffers_ready_for_compositor(userId);
    if (framesPending > 0) {
        // The line below looks like an innocent, effect-less, getter. But as this
        // method returns a unique_pointer, not holding its reference causes the
        // buffer to be destroyed/released straight away.
        for (auto const & item : m_surface->generate_renderables(userId))
            item->buffer();
        qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::dropPendingBuffer()"
            << "surface =" << this
            << "buffer dropped."
            << framesPending-1
            << "left.";
    }
}

void MirSurfaceItem::stopFrameDropper()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::stopFrameDropper surface = " << this;
    QMutexLocker locker(&m_mutex);
    m_frameDropperTimer.stop();
}

void MirSurfaceItem::startFrameDropper()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::startFrameDropper surface = " << this;
    QMutexLocker locker(&m_mutex);
    if (!m_frameDropperTimer.isActive()) {
        m_frameDropperTimer.start();
    }
}

void MirSurfaceItem::scheduleTextureUpdate()
{
    QMutexLocker locker(&m_mutex);

    // Notify QML engine that this needs redrawing, schedules call to updatePaintItem
    update();
    // restart the frame dropper so that we have enough time to render the next frame.
    m_frameDropperTimer.start();
}

void MirSurfaceItem::setSession(SessionInterface *session)
{
    m_session = session;
}

void MirSurfaceItem::onSessionStateChanged(SessionInterface::State state)
{
    switch (state) {
        case SessionInterface::State::Running:
            syncSurfaceSizeWithItemSize();
            break;
        default:
            break;
    }
}

void MirSurfaceItem::syncSurfaceSizeWithItemSize()
{
    int mirWidth = m_surface->size().width.as_int();
    int mirHeight = m_surface->size().width.as_int();

    if ((int)width() != mirWidth || (int)height() != mirHeight) {
        qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::syncSurfaceSizeWithItemSize()";
        mir::geometry::Size newMirSize((int)width(), (int)height());
        m_surface->resize(newMirSize);
        setImplicitSize(width(), height());
    }
}

bool MirSurfaceItem::clientIsRunning() const
{
    return (m_session &&
            (m_session->state() == Session::State::Running
             || m_session->state() == Session::State::Starting))
        || !m_session;
}

void MirSurfaceItem::TouchEvent::updateTouchPointStatesAndType()
{
    touchPointStates = 0;
    for (int i = 0; i < touchPoints.count(); ++i) {
        touchPointStates |= touchPoints.at(i).state();
    }

    if (touchPointStates == Qt::TouchPointReleased) {
        type = QEvent::TouchEnd;
    } else if (touchPointStates == Qt::TouchPointPressed) {
        type = QEvent::TouchBegin;
    } else {
        type = QEvent::TouchUpdate;
    }
}

} // namespace qtmir

#include "mirsurfaceitem.moc"
