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

#include "request.h"
#include "json.h"
#include "logger.h"
#include "settings.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#if QT_VERSION >= 0x50000
#include <QUrlQuery>
#endif

QPointer<QNetworkAccessManager> Request::nam = 0;

Request::Request(QObject *parent) :
    QObject(parent),
    m_operation(GetOperation),
    m_status(Null),
    m_statusCode(0)
{
}

Request::~Request() {
    cancel();
}

QString Request::errorString() const {
    return m_errorString;
}

void Request::setErrorString(const QString &e) {
    m_errorString = e;

    if (!e.isEmpty()) {
        Logger::log("Request::error(). " + e);
    }
}

Request::Operation Request::operation() const {
    return m_operation;
}

void Request::setOperation(Request::Operation op) {
    if (op != operation()) {
        m_operation = op;
        emit operationChanged(op);
    }
}

QVariantMap Request::params() const {
    return m_params;
}

void Request::setParams(const QVariantMap &p) {
    m_params = p;
    emit paramsChanged(p);
}

QString Request::path() const {
    return m_path;
}

void Request::setPath(const QString &p) {
    if (p != path()) {
        m_path = p;
        emit pathChanged(p);
    }
}

QVariant Request::postData() const {
    return m_postData;
}

void Request::setPostData(const QVariant &data) {
    if (data != postData()) {
        m_postData = data;
        emit postDataChanged(data);
    }
}

QVariant Request::result() const {
    return m_result;
}

void Request::setResult(const QVariant &r) {
    m_result = r;
}

Request::Status Request::status() const {
    return m_status;
}

void Request::setStatus(Request::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

int Request::statusCode() const {
    return m_statusCode;
}

void Request::setStatusCode(int code) {
    m_statusCode = code;
}

QNetworkAccessManager* Request::networkAccessManager() {
    if (!nam) {
        nam = new QNetworkAccessManager(this);
    }

    return nam;
}

void Request::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (!nam) {
        nam = manager;
    }
}

void Request::start() {
    if (status() == Active) {
        return;
    }

    setStatus(Active);
    setStatusCode(0);
    setErrorString(QString());
    setResult(QVariant());

    QUrl url;
    url.setHost(Settings::serverHost());
    url.setPort(Settings::serverPort());
    url.setScheme("http");
    url.setPath(path());
    QMapIterator<QString, QVariant> iterator(params());
#if QT_VERSION >= 0x050000
    QUrlQuery query;
    
    while (iterator.hasNext()) {
        iterator.next();
        query.addQueryItem(iterator.key(), iterator.value().toString());
    }
    
    url.setQuery(query);
#else
    while (iterator.hasNext()) {
        iterator.next();
        url.addQueryItem(iterator.key(), iterator.value().toString());
    }
#endif
    QNetworkRequest request(url);

    if (Settings::serverAuthenticationEnabled()) {
        request.setRawHeader("Authorization", "Basic " + QString("%1:%2").arg(Settings::serverUsername())
                .arg(Settings::serverPassword()).toUtf8().toBase64());
    }

    QNetworkReply *reply;
    
    switch (operation()) {
    case DeleteOperation:
        reply = networkAccessManager()->deleteResource(request);
        break;
    case PostOperation:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        reply = networkAccessManager()->post(request, QtJson::Json::serialize(postData()));
        break;
    case PutOperation:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        reply = networkAccessManager()->put(request, QtJson::Json::serialize(postData()));
        break;
    default:
        reply = networkAccessManager()->get(request);
        break;
    }

    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(this, SIGNAL(finished(Request*)), reply, SLOT(deleteLater()));
}

void Request::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }
}

void Request::deleteResource(const QString &path) {
    setOperation(DeleteOperation);
    setPath(path);
    setParams(QVariantMap());
    setPostData(QVariant());
    start();
}

void Request::deleteResource(const QString &path, const QVariantMap &params) {
    setOperation(DeleteOperation);
    setPath(path);
    setParams(params);
    setPostData(QVariant());
    start();
}

void Request::get(const QString &path) {
    setOperation(GetOperation);
    setPath(path);
    setParams(QVariantMap());
    setPostData(QVariant());
    start();
}

void Request::get(const QString &path, const QVariantMap &params) {
    setOperation(GetOperation);
    setPath(path);
    setParams(params);
    setPostData(QVariant());
    start();
}

void Request::post(const QString &path, const QVariant &postData) {
    setOperation(PostOperation);
    setPath(path);
    setParams(QVariantMap());
    setPostData(postData);
    start();
}

void Request::post(const QString &path, const QVariantMap &params, const QVariant &postData) {
    setOperation(PostOperation);
    setPath(path);
    setParams(params);
    setPostData(postData);
    start();
}

void Request::put(const QString &path, const QVariant &postData) {
    setOperation(PutOperation);
    setPath(path);
    setParams(QVariantMap());
    setPostData(postData);
    start();
}

void Request::put(const QString &path, const QVariantMap &params, const QVariant &postData) {
    setOperation(PutOperation);
    setPath(path);
    setParams(params);
    setPostData(postData);
    start();
}

void Request::onReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    setStatusCode(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        return;
    default:
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        return;
    }

    if (reply->header(QNetworkRequest::ContentTypeHeader) == "application/json") {
        setResult(QtJson::Json::parse(QString::fromUtf8(reply->readAll())));
    }
    else {
        setResult(reply->readAll());
    }

    setStatus(Finished);
    emit finished(this);
}
