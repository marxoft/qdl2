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

#include "imagecache.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

class CachedImage
{

public:
    explicit CachedImage(const QUrl &u) :
        url(u)
    {
    }
    
    const QUrl url;
    QImage image;    
};

QThread* ImageCache::thread = 0;
QQueue<QUrl> ImageCache::queue;
QCache<QUrl, CachedImage> ImageCache::cache;

int ImageCache::requestCount = 0;
int ImageCache::refCount = 0;

const int ImageCache::MAX_REQUESTS = 8;

ImageCache::ImageCache() :
    QObject(),
    m_manager(new QNetworkAccessManager)
{
    refCount++;
    
    if (!thread) {
        thread = new QThread;
        thread->start();
    }
    
    moveToThread(thread);
}

ImageCache::~ImageCache() {
    delete m_manager;
    m_manager = 0;
    refCount--;
    
    if (refCount == 0) {
        thread->quit();
        thread->deleteLater();
        thread = 0;
    }
}

QImage ImageCache::image(const QUrl &url, const QSize &size, Qt::AspectRatioMode aspectRatioMode,
                         Qt::TransformationMode transformationMode) {
    CachedImage *ci = cache.object(url);
    
    if (!ci) {
        ci = new CachedImage(url);
        cache.insert(url, ci);
    
        if (requestCount < MAX_REQUESTS) {
            getImage(url);
        }
        else {
            queue.enqueue(url);
        }
    }
    
    return (!size.isEmpty()) && (!ci->image.isNull()) ? ci->image.scaled(size, aspectRatioMode, transformationMode)
        : ci->image;
}

void ImageCache::getImage(const QUrl &url) {
    requestCount++;
    ImageRequest *request = new ImageRequest(m_manager, url);
    connect(request, SIGNAL(finished(ImageRequest*)), this, SLOT(onRequestFinished(ImageRequest*)));
}

void ImageCache::onRequestFinished(ImageRequest *request) {    
    if (CachedImage *ci = cache.object(request->url)) {
        QImage image;
        image.loadFromData(request->reply->readAll());
        ci->image = image;
        emit imageReady();
    }
    
    request->deleteLater();
    requestCount--;
    
    if ((!queue.isEmpty()) && (requestCount < MAX_REQUESTS)) {
        getImage(queue.dequeue());
    }
}

const QByteArray ImageRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

ImageRequest::ImageRequest(QNetworkAccessManager *manager, const QUrl &u) :
    QObject()
{
    url = u;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

ImageRequest::~ImageRequest() {
    reply->deleteLater();
    reply = 0;
}

void ImageRequest::onReplyFinished() {
    emit finished(this);
}
