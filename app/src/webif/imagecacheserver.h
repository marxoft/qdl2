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

#ifndef IMAGECACHESERVER_H
#define IMAGECACHESERVER_H

#include <QObject>
#include <QHash>
#include <QSize>

class ImageCache;
class QHttpRequest;
class QHttpResponse;

class ImageCacheServer : public QObject
{
    Q_OBJECT

public:
    explicit ImageCacheServer(QObject *parent = 0);
    ~ImageCacheServer();

    bool handleRequest(QHttpRequest *request, QHttpResponse *response);

private Q_SLOTS:
    void onImageReady();
    void onResponseDone();

private:
    struct ImageProperties {
        ImageProperties() :
            url(QString()),
            size(QSize()),
            quality(100)
        {
        }

        ImageProperties(const QString &u, const QSize &s = QSize(), int q = 100) :
            url(u),
            size(s),
            quality(q)
        {
        }

        QString url;
        QSize size;
        int quality;
    };

    void addResponse(QHttpResponse *response, const ImageProperties &properties);
    void removeResponse(QHttpResponse *response);

    ImageCache *m_cache;

    QHash<QHttpResponse*, ImageProperties> m_hash;
};

#endif // IMAGECACHESERVER_H
