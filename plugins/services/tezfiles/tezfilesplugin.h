/**
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef TEZFILESPLUGIN_H
#define TEZFILESPLUGIN_H

#include "serviceplugin.h"
#include <QPointer>
#include <QRegExp>
#include <QUrl>

class QNetworkReply;
class QTimer;

class TezfilesPlugin : public ServicePlugin
{
    Q_OBJECT
    
public:
    explicit TezfilesPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();

    virtual void checkUrl(const QString &url, const QVariantMap &settings);

    virtual void getDownloadRequest(const QString &url, const QVariantMap &settings);
    
    void submitCaptchaResponse(const QString &, const QString &response);
    void submitLogin(const QVariantMap &credentials);

private Q_SLOTS:
    void checkLogin();
    void checkUrlIsValid();
    void checkDownloadRequest();
    void checkWaitTime();
    void checkCaptcha();
    void getDownloadLink();
    void checkDownloadLink();

Q_SIGNALS:
    void currentOperationCanceled();

private:
    static QString getRedirect(const QNetworkReply *reply);
    
    QNetworkAccessManager* networkAccessManager();

    void fetchDownloadRequest(const QUrl &url);

    void followRedirect(const QUrl &url, const char* slot);

    void getWaitTime();
        
    void login(const QString &username, const QString &password);
    
    void startWaitTimer(int msecs, const char* slot);
    void stopWaitTimer();

    static const QRegExp ERROR_REGEXP;
    static const QRegExp FILE_REGEXP;
    static const QRegExp FILENAME_REGEXP;
    static const QRegExp WAITTIME_REGEXP;
    static const QString LOGIN_URL;
    static const QString RECAPTCHA_PLUGIN_ID;
    
    static const int MAX_REDIRECTS;

    QPointer<QNetworkAccessManager> m_nam;    
    QTimer *m_waitTimer;
    
    QUrl m_url;
    QString m_fileId;
    
    int m_redirects;

    bool m_ownManager;
};

class TezfilesPluginFactory : public QObject, public ServicePluginFactory
{
    Q_OBJECT
    Q_INTERFACES(ServicePluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.TezfilesPluginFactory")
#endif

public:
    virtual ServicePlugin* createPlugin(QObject *parent = 0);
};

#endif // TEZFILESPLUGIN_H
