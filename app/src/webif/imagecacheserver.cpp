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

#include "imagecacheserver.h"
#include "imagecache.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "utils.h"
#include <QBuffer>
#include <QImageWriter>

ImageCacheServer::ImageCacheServer(QObject *parent) :
    QObject(parent),
    m_cache(new ImageCache)
{
    connect(m_cache, SIGNAL(imageReady()), this, SLOT(onImageReady()));
}

ImageCacheServer::~ImageCacheServer() {
    delete m_cache;
    m_cache = 0;
}

bool ImageCacheServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.size() != 2) || (parts.first() != "images")) {
        return false;
    }

    const QString url = QString::fromUtf8(QByteArray::fromBase64(parts.at(1).toUtf8()));

    if (!url.isEmpty()) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const int width = Utils::urlQueryItemValue(request->url(), "width", "-1").toInt();
            const int height = Utils::urlQueryItemValue(request->url(), "height", "-1").toInt();
            const int quality = Utils::urlQueryItemValue(request->url(), "quality", "100").toInt();
            const QSize size(width, height);
            const QImage image = m_cache->image(url, size);

            if (!image.isNull()) {
                QBuffer buffer;
                QImageWriter writer(&buffer, "JPG");
                writer.setQuality(quality);

                if (writer.write(image)) {
                    writeResponse(response, QHttpResponse::STATUS_OK, buffer.data());
                }
                else {
                    writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
                }
            }
            else {
                addResponse(response, ImageProperties(url, size, quality));
            }
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        }
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
    }

    return true;
}

void ImageCacheServer::addResponse(QHttpResponse *response, const ImageProperties &properties) {
    m_hash.insert(response, properties);
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

void ImageCacheServer::removeResponse(QHttpResponse *response) {
    m_hash.remove(response);
    disconnect(response, 0, this, 0);
}

void ImageCacheServer::onImageReady() {
    QHashIterator<QHttpResponse*, ImageProperties> iterator(m_hash);

    while (iterator.hasNext()) {
        iterator.next();
        const QImage image = m_cache->image(iterator.value().url, iterator.value().size);

        if (!image.isNull()) {
            QBuffer buffer;
            QImageWriter writer(&buffer, "JPG");
            writer.setQuality(iterator.value().quality);

            if (writer.write(image)) {
                writeResponse(iterator.key(), QHttpResponse::STATUS_OK, buffer.data());
            }
            else {
                writeResponse(iterator.key(), QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
            }
        }
    }
}

void ImageCacheServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        removeResponse(response);
    }
}
