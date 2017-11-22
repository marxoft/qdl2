/**
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SOUNDCLOUDPLUGIN_H
#define SOUNDCLOUDPLUGIN_H

#include "serviceplugin.h"
#include <QStringList>
#include <QVariantList>

namespace QSoundCloud {
    class ResourcesRequest;
    class StreamsRequest;
}

class SoundCloudPlugin : public ServicePlugin
{
    Q_OBJECT
    
public:
    explicit SoundCloudPlugin(QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();

    virtual void checkUrl(const QString &url, const QVariantMap &settings);

    virtual void getDownloadRequest(const QString &url, const QVariantMap &settings);

    void submitFormat(const QVariantMap &format);

private Q_SLOTS:
    void onResourcesRequestFinished();
    void onStreamsRequestFinished();
    
private:
    static const QString CLIENT_ID;
    static const QStringList AUDIO_FORMATS;
    
    QSoundCloud::ResourcesRequest *m_resourcesRequest;
    QSoundCloud::StreamsRequest *m_streamsRequest;

    QVariantMap m_settings;
};

class SoundCloudPluginFactory : public QObject, public ServicePluginFactory
{
    Q_OBJECT
    Q_INTERFACES(ServicePluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.SoundCloudPluginFactory")
#endif

public:
    virtual ServicePlugin* createPlugin(QObject *parent = 0);
};

#endif // SOUNDCLOUDPLUGIN_H
