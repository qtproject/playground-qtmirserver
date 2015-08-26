/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#ifndef SESSION_H
#define SESSION_H

// std
#include <memory>

// local
#include "session_interface.h"

// Qt
#include <QObject>
#include <QTimer>


namespace mir {
    namespace scene {
        class PromptSessionManager;
    }
}

namespace qtmir {

class Application;

class Session : public SessionInterface
{
    Q_OBJECT
public:
    explicit Session(const std::shared_ptr<mir::scene::Session>& session,
                     const std::shared_ptr<mir::scene::PromptSessionManager>& promptSessionManager,
                     QObject *parent = 0);
    virtual ~Session();

    Q_INVOKABLE void release() override;

    //getters
    QString name() const override;
    unity::shell::application::ApplicationInfoInterface* application() const override;
    MirSurfaceItemInterface* surface() const override;
    SessionInterface* parentSession() const override;
    State state() const override;
    bool fullscreen() const override;
    bool live() const override;

    void setApplication(unity::shell::application::ApplicationInfoInterface* item) override;
    void setSurface(MirSurfaceItemInterface* surface) override;

    void suspend() override;
    void resume() override;
    void stop() override;

    void addChildSession(SessionInterface* session) override;
    void insertChildSession(uint index, SessionInterface* session) override;
    void removeChildSession(SessionInterface* session) override;
    void foreachChildSession(std::function<void(SessionInterface* session)> f) const override;

    std::shared_ptr<mir::scene::Session> session() const override;

    std::shared_ptr<mir::scene::PromptSession> activePromptSession() const override;
    void foreachPromptSession(std::function<void(const std::shared_ptr<mir::scene::PromptSession>&)> f) const override;

    SessionModel* childSessions() const override;

    void setFullscreen(bool fullscreen) override;
    void setLive(const bool) override;
    void appendPromptSession(const std::shared_ptr<mir::scene::PromptSession>& session) override;
    void removePromptSession(const std::shared_ptr<mir::scene::PromptSession>& session) override;

public Q_SLOTS:
    // it's public to ease testing
    void doSuspend();

private Q_SLOTS:
    void updateFullscreenProperty();
    void onFirstSurfaceFrameDrawn();

private:
    void setParentSession(Session* session);
    void setState(State state);
    void doResume();

    void stopPromptSessions();

    std::shared_ptr<mir::scene::Session> m_session;
    Application* m_application;
    MirSurfaceItemInterface* m_surface;
    SessionInterface* m_parentSession;
    SessionModel* m_children;
    bool m_fullscreen;
    State m_state;
    bool m_live;
    bool m_released;
    QTimer* m_suspendTimer;
    QList<std::shared_ptr<mir::scene::PromptSession>> m_promptSessions;
    std::shared_ptr<mir::scene::PromptSessionManager> const m_promptSessionManager;
};

} // namespace qtmir

#endif // SESSION_H
