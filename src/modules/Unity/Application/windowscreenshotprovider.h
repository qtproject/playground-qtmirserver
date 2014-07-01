/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef WINDOW_SCREENSHOT_PROVIDER_H_
#define WINDOW_SCREENSHOT_PROVIDER_H_

#include <QQuickImageProvider>

namespace qtmir {

class WindowScreenshotProvider : public QQuickImageProvider
{
public:
    WindowScreenshotProvider();

    // id is ignored for now
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

} // namespace qtmir

#endif // WINDOW_SCREENSHOT_PROVIDER_H_
