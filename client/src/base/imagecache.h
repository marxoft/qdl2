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

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QObject>
#include <QCache>
#include <QPixmap>

class CachedImage;
class Request;

class ImageCache : public QObject
{
    Q_OBJECT
    
public:
    ~ImageCache();
    
    QPixmap image(const QString &path, const QSize &size = QSize(), Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
            Qt::TransformationMode transformationMode = Qt::SmoothTransformation);

    static ImageCache* instance();
    
private Q_SLOTS:
    void onRequestFinished(Request *request);
    
Q_SIGNALS:
    void imageReady(const QString &path);
    
private:
    ImageCache();

    void getImage(const QString &path);
    
    QCache<QString, CachedImage> m_cache;

    static ImageCache *self;
};
    
#endif // IMAGECACHE_H
