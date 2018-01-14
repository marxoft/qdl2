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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QHash>

class QHttpServer;
class QHttpRequest;
class QHttpResponse;

class WebServer : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(bool authenticationEnabled READ authenticationEnabled WRITE setAuthenticationEnabled
               NOTIFY authenticationEnabledChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Error
    };
    
    ~WebServer();
    
    static WebServer* instance();
        
    int port() const;
    
    bool authenticationEnabled() const;
    QString username() const;
    QString password() const;
    
    bool isRunning() const;
    
    Status status() const;

public Q_SLOTS:
    void setPort(int p);
    
    void setAuthenticationEnabled(bool enabled);
    void setUsername(const QString &u);
    void setPassword(const QString &p);
    
    void setRunning(bool enabled);
    bool start();
    void stop();

private Q_SLOTS:
    void onNewRequest(QHttpRequest *request, QHttpResponse *response);
    void onRequestEnd();

Q_SIGNALS:
    void authenticationEnabledChanged();
    void passwordChanged();
    void portChanged();
    void statusChanged(WebServer::Status status);
    void usernameChanged();

private:
    WebServer();
    
    void init();
    
    void setStatus(Status s);
        
    void handleRequest(QHttpRequest *request, QHttpResponse *response);
        
    static WebServer *self;
    
    QHttpServer *m_server;
    
    int m_port;
    
    QString m_username;
    QString m_password;
    QByteArray m_auth;
    bool m_authenticationEnabled;
    
    Status m_status;
        
    QHash<QHttpRequest*, QHttpResponse*> m_requests;    
};

#endif // WEBSERVER_H
