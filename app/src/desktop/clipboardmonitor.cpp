/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clipboardmonitor.h"
#include "logger.h"
#include "servicepluginmanager.h"
#include <QApplication>
#include <QClipboard>

ClipboardMonitor* ClipboardMonitor::self = 0;

ClipboardMonitor::ClipboardMonitor() :
    QObject(),
    m_enabled(false)
{
}

ClipboardMonitor::~ClipboardMonitor() {
    self = 0;
}

ClipboardMonitor* ClipboardMonitor::instance() {
    return self ? self : self = new ClipboardMonitor;
}

bool ClipboardMonitor::isEnabled() const {
    return m_enabled;
}

void ClipboardMonitor::setEnabled(bool enabled) {
    if (enabled != isEnabled()) {
        m_enabled = enabled;
        emit enabledChanged(enabled);

        if (enabled) {
            connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));
        }
        else {
            disconnect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));
        }
    }
}

QStringList ClipboardMonitor::urls() const {
    return m_urls;
}

void ClipboardMonitor::clear() {
    m_urls.clear();
}

void ClipboardMonitor::onClipboardDataChanged() {
    QStringList urls = QApplication::clipboard()->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);

    for (int i = urls.size() - 1; i >= 0; i--) {
        if (ServicePluginManager::instance()->urlIsSupported(urls.at(i))) {
            Logger::log("ClipboardMonitor::onClipboardDataChanged(): URL added: " + urls.at(i));
        }
        else {
            urls.removeAt(i);
        }
    }

    if (!urls.isEmpty()) {
        m_urls << urls;
        emit urlsAdded(urls);
    }
}
