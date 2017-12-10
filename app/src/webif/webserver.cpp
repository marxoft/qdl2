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

#include "webserver.h"
#include "applicationserver.h"
#include "categoryserver.h"
#include "decaptchapluginconfigserver.h"
#include "definitions.h"
#include "downloadrequestserver.h"
#include "fileserver.h"
#include "imagecacheserver.h"
#include "recaptchapluginconfigserver.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpserver.h"
#include "searchserver.h"
#include "searchpluginconfigserver.h"
#include "servicepluginconfigserver.h"
#include "settingsserver.h"
#include "transferserver.h"
#include "urlcheckserver.h"
#include "urlretrievalserver.h"

WebServer* WebServer::self = 0;

WebServer::WebServer() :
    QObject(),
    m_server(0),
    m_imageServer(0),
    m_searchServer(0),
    m_port(8080),
    m_authenticationEnabled(false),
    m_status(Idle)
{
}

WebServer::~WebServer() {
    self = 0;
}

WebServer* WebServer::instance() {
    return self ? self : self = new WebServer;
}

int WebServer::port() const {
    return m_port;
}

void WebServer::setPort(int p) {
    if (p != port()) {
        m_port = p;
        emit portChanged();
        
        if (isRunning()) {
            stop();
            start();
        }
    }
}

bool WebServer::authenticationEnabled() const {
    return m_authenticationEnabled;
}

void WebServer::setAuthenticationEnabled(bool enabled) {
    if (enabled != authenticationEnabled()) {
        m_authenticationEnabled = enabled;
        emit authenticationEnabledChanged();
    }
}

QString WebServer::username() const {
    return m_username;
}

void WebServer::setUsername(const QString &u) {
    if (u != username()) {
        m_username = u;
        m_auth = QByteArray(u.toUtf8() + ":" + password().toUtf8()).toBase64();
        emit usernameChanged();
    }
}

QString WebServer::password() const {
    return m_password;
}

void WebServer::setPassword(const QString &p) {
    if (p != password()) {
        m_password = p;
        m_auth = QByteArray(username().toUtf8() + ":" + p.toUtf8()).toBase64();
        emit passwordChanged();
    }
}

bool WebServer::isRunning() const {
    return m_status == Active;
}

void WebServer::setRunning(bool enabled) {
    if (enabled != isRunning()) {
        if (enabled) {
            start();
        }
        else {
            stop();
        }
    }
}

WebServer::Status WebServer::status() const {
    return m_status;
}

void WebServer::setStatus(WebServer::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool WebServer::start() {
    init();
    
    if (m_server->listen(port())) {
        setStatus(Active);
        return true;
    }
    
    setStatus(Error);
    return false;
}

void WebServer::stop() {
    if (m_server) {
        if (isRunning()) {
            m_server->close();
            setStatus(Idle);
        }
    }
}

void WebServer::init() {
    if (!m_server) {
        m_server = new QHttpServer(this);
        connect(m_server, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)),
                this, SLOT(onNewRequest(QHttpRequest*,QHttpResponse*)));
    }

    if (!m_imageServer) {
        m_imageServer = new ImageCacheServer(this);
    }

    if (!m_searchServer) {
        m_searchServer = new SearchServer(this);
    }
}

void WebServer::onNewRequest(QHttpRequest *request, QHttpResponse *response) {
    if (authenticationEnabled()) {
        const QByteArray auth = request->header("authorization").section(" ", -1).toUtf8();
        
        if (auth != m_auth) {
            response->setHeader("WWW-Authenticate", "Basic realm=\"cuteNews\"");
            response->setHeader("Content-Length", "0");
            response->writeHead(QHttpResponse::STATUS_UNAUTHORIZED);
            response->end();
            return;
        }
    }
    
    request->storeBody();
    
    if (request->successful()) {
        handleRequest(request, response);
    }
    else {
        m_requests.insert(request, response);
        connect(request, SIGNAL(end()), this, SLOT(onRequestEnd()));
    }    
}

void WebServer::onRequestEnd() {
    QHttpRequest *request = qobject_cast<QHttpRequest*>(sender());
    
    if (!request) {
        return;
    }
    
    QHttpResponse *response = m_requests.value(request);
    
    if (!response) {
        return;
    }
    
    m_requests.remove(request);
    handleRequest(request, response);
}

void WebServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    if (request->path().startsWith("/app")) {
        if (ApplicationServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/categories")) {
        if (CategoryServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/images")) {
        if (m_imageServer->handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/decaptchaplugins")) {
        if (DecaptchaPluginConfigServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/recaptchaplugins")) {
        if (RecaptchaPluginConfigServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/searchplugins")) {
        if (SearchPluginConfigServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/serviceplugins")) {
        if (ServicePluginConfigServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/downloadrequest")) {
        if (DownloadRequestServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/search")) {
        if (m_searchServer->handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/settings")) {
        if (SettingsServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/transfers")) {
        if (TransferServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/urlcheck")) {
        if (UrlCheckServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/urlretrieval")) {
        if (UrlRetrievalServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (FileServer::handleRequest(request, response)) {
        return;
    }
    
    response->setHeader("Content-Length", "0");
    response->writeHead(QHttpResponse::STATUS_NOT_FOUND);
    response->end();
}
