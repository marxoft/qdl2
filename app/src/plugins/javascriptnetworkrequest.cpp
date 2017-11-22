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

#include "javascriptnetworkrequest.h"
#include <QScriptEngine>

JavaScriptNetworkRequest::JavaScriptNetworkRequest(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptNetworkRequest::url() const {
    if (const QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        return request->url().toString();
    }
    
    return QString();
}

void JavaScriptNetworkRequest::setUrl(const QString &u) {
    if (QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        request->setUrl(u);
    }
}

QVariantMap JavaScriptNetworkRequest::headers() const {
    if (const QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        QVariantMap h;
        
        foreach (const QByteArray &header, request->rawHeaderList()) {
            h[QString::fromUtf8(header)] = QString::fromUtf8(request->rawHeader(header));
        }
        
        return h;
    }
    
    return QVariantMap();
}

void JavaScriptNetworkRequest::setHeaders(const QVariantMap &h) {
    if (QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        QMapIterator<QString, QVariant> iterator(h);
        
        while (iterator.hasNext()) {
            iterator.next();
            request->setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
        }
    }
}

QVariant JavaScriptNetworkRequest::header(const QString &name) const {
    if (const QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        return request->rawHeader(name.toUtf8());
    }
    
    return QVariant();
}

void JavaScriptNetworkRequest::setHeader(const QString &name, const QVariant &value) {
    if (QNetworkRequest* request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        request->setRawHeader(name.toUtf8(), value.toByteArray());
    }
}
