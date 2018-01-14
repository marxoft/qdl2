/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "imagecache.h"
#include "request.h"

class CachedImage
{

public:
    explicit CachedImage(const QString &p) :
        path(p)
    {
    }
    
    const QString path;
    QPixmap image;    
};

ImageCache* ImageCache::self = 0;

ImageCache::ImageCache() :
    QObject()
{
}

ImageCache::~ImageCache() {
    self = 0;
}

ImageCache* ImageCache::instance() {
    return self ? self : self = new ImageCache;
}

QPixmap ImageCache::image(const QString &path, const QSize &size, Qt::AspectRatioMode aspectRatioMode,
        Qt::TransformationMode transformationMode) {
    CachedImage *ci = m_cache.object(path);
    
    if (!ci) {
        ci = new CachedImage(path);
        m_cache.insert(path, ci);
        getImage(path);
    }
    
    return (!size.isEmpty()) && (!ci->image.isNull()) ? ci->image.scaled(size, aspectRatioMode, transformationMode)
        : ci->image;
}

void ImageCache::getImage(const QString &path) {
    Request *request = new Request(this);
    request->get(path);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void ImageCache::onRequestFinished(Request *request) {    
    if (CachedImage *ci = m_cache.object(request->path())) {
        QPixmap image;
        image.loadFromData(request->result().toByteArray());
        ci->image = image;
        emit imageReady(request->path());
    }
    
    request->deleteLater();
}
