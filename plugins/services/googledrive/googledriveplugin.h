/*
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

#ifndef GOOGLEDRIVEPLUGIN_H
#define GOOGLEDRIVEPLUGIN_H

#include "serviceplugin.h"
#include <QRegExp>
#include <QStringList>
#include <QUrl>

class GoogleDrivePlugin : public ServicePlugin
{
    Q_OBJECT
    
    Q_INTERFACES(ServicePlugin)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.GoogleDrivePlugin")
#endif

public:
    explicit GoogleDrivePlugin(QObject *parent = 0);

    virtual ServicePlugin* createPlugin(QObject *parent = 0);

    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();

    virtual void checkUrl(const QString &url);

    virtual void getDownloadRequest(const QString &url);

private Q_SLOTS:
    void checkUrlIsValid();
    void checkDownloadRequest();
    void checkDownloadPage();

Q_SIGNALS:
    void currentOperationCanceled();
    
private:
    QNetworkAccessManager* networkAccessManager();

    void followRedirect(const QUrl &url, const char* slot);

    void getDownloadPage(const QUrl &url);
    
    static QMap<QString, QUrl> getYouTubeVideoUrlMap(const QString &page);
    
    static QString unescape(const QString &s);

    static QString CONFIG_FILE;

    static QStringList VIDEO_FORMATS;

    static int MAX_REDIRECTS;

    QNetworkAccessManager *m_nam;

    int m_redirects;

    bool m_ownManager;
};

#endif // GOOGLEDRIVEPLUGIN_H
