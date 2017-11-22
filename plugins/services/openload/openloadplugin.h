/**
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
 * along with plugin.program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENLOADPLUGIN_H
#define OPENLOADPLUGIN_H

#include "serviceplugin.h"
#include <QPointer>

class QNetworkReply;
class QWebPage;

class OpenloadPlugin : public ServicePlugin
{
    Q_OBJECT

public:
    explicit OpenloadPlugin(QObject *parent = 0);

    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();

    virtual void checkUrl(const QString &url, const QVariantMap &settings);

    virtual void getDownloadRequest(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void checkUrlIsValid();
    void checkDownloadRequest(bool ok);

Q_SIGNALS:
    void currentOperationCanceled();

private:
    static QString getRedirect(const QNetworkReply *reply);

    QNetworkAccessManager* networkAccessManager();

    QWebPage* webPage();

    void followRedirect(const QString &url, const char *slot);

    static const QString EMBED_URL;
    static const QString STREAM_URL;

    static const int MAX_REDIRECTS;

    QWebPage *m_page;
    QPointer<QNetworkAccessManager> m_nam;

    int m_redirects;

    bool m_ownManager;
};

class OpenloadPluginFactory : public QObject, public ServicePluginFactory
{
    Q_OBJECT
    Q_INTERFACES(ServicePluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.OpenloadPluginFactory")
#endif

public:
    virtual ServicePlugin* createPlugin(QObject *parent = 0);
};

#endif // OPENLOADPLUGIN_H
