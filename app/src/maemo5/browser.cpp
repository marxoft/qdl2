/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "browser.h"
#include "imagecache.h"

Browser::Browser(QWidget *parent) :
    QTextBrowser(parent),
    m_cache(0)
{
    setOpenLinks(false);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    setFrameStyle(QTextEdit::NoFrame);
    setStyleSheet(QString("color: %1; background: transparent").arg(palette().color(QPalette::Text).name()));
}

Browser::~Browser() {
    if (m_cache) {
        delete m_cache;
        m_cache = 0;
    }
}

QVariant Browser::loadResource(int type, const QUrl &name) {
    if (type != QTextDocument::ImageResource) {
        return QTextBrowser::loadResource(type, name);
    }
    
    if (!m_cache) {
        m_cache = new ImageCache;
        connect(m_cache, SIGNAL(imageReady()), viewport(), SLOT(update()));
    }
    
    return m_cache->image(name);
}
