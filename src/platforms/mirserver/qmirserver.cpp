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

// Qt
#include <QObject>
#include <QCoreApplication>
#include <QDebug>

// local
#include "mirserver.h"
#include "qmirserver.h"
#include "qmirserver_p.h"


QMirServer::QMirServer(const QStringList &arguments, QObject *parent)
    : QObject(parent)
    , d_ptr(new QMirServerPrivate())
{
    Q_D(QMirServer);

    // convert arguments back into argc-argv form that Mir wants
    int argc = arguments.size();
    char **argv = new char*[argc + 1];
    for (int i = 0; i < argc; i++) {
        argv[i] = new char[strlen(arguments.at(i).toStdString().c_str())+1];
        memcpy(argv[i], arguments.at(i).toStdString().c_str(), strlen(arguments.at(i).toStdString().c_str())+1);
    }
    argv[argc] = '\0';

    d->server = QSharedPointer<MirServer>(new MirServer(argc, const_cast<const char**>(argv)));

    d->serverThread = new MirServerThread(d->server);

    connect(d->serverThread, &MirServerThread::stopped, this, &QMirServer::stopped);
}

QMirServer::~QMirServer()
{
    stop();
}

bool QMirServer::start()
{
    Q_D(QMirServer);

    d->serverThread->start(QThread::TimeCriticalPriority);

    if (!d->serverThread->waitForMirStartup())
    {
        qCritical() << "ERROR: QMirServer - Mir failed to start";
        return false;
    }

    Q_EMIT started();
    return true;
}

void QMirServer::stop()
{
    Q_D(QMirServer);

    if (d->serverThread->isRunning()) {
        d->serverThread->stop();
        if (!d->serverThread->wait(10000)) {
            // do something to indicate fail during shutdown
            qCritical() << "ERROR: QMirServer - Mir failed to shut down correctly, terminating it";
            d->serverThread->terminate();
        }
    }
}

bool QMirServer::isRunning() const
{
    Q_D(const QMirServer);
    return d->serverThread->isRunning();
}

QWeakPointer<MirServer> QMirServer::mirServer() const
{
    Q_D(const QMirServer);
    return d->server.toWeakRef();
}
