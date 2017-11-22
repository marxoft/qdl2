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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PORNHUBSEARCHPLUGIN_H
#define PORNHUBSEARCHPLUGIN_H

#include "searchplugin.h"
#include <QNetworkCookie>

class QNetworkAccessManager;
class QNetworkReply;

class PornhubSearchPlugin : public SearchPlugin
{
    Q_OBJECT

public:
    explicit PornhubSearchPlugin(QObject *parent = 0);
     
public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void fetchMore(const QVariantMap &params);
    virtual void search(const QVariantMap &settings);
    void getVideos(const QVariantMap &settings);

private Q_SLOTS:
    void checkVideos();

Q_SIGNALS:
    void finished();

private:
    static QString getRedirect(const QNetworkReply *reply);
    
    static QUrl incrementPageNumber(QUrl url);
    
    void getVideos(const QString &url);
    
    void followRedirect(const QString &url, const char *slot);
    
    QNetworkAccessManager* networkAccessManager();
    
    static const QString BASE_URL;
    static const QString HTML;
    
    static const QList<QNetworkCookie> MOBILE_COOKIES;
    static const QList<QNetworkCookie> TABLET_COOKIES;
    
    static const int MAX_REDIRECTS;    
    
    QNetworkAccessManager *m_nam;
    
    int m_redirects;
};

class PornhubSearchPluginFactory : public QObject, public SearchPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SearchPluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.PornhubSearchPluginFactory")
#endif

public:
    virtual SearchPlugin* createPlugin(QObject *parent = 0);
};

#endif // PORNHUBSEARCHPLUGIN_H
