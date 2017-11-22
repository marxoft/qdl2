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

#include "soundcloudplugin.h"
#include <qsoundcloud/resourcesrequest.h>
#include <qsoundcloud/streamsrequest.h>
#include <QNetworkRequest>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString SoundCloudPlugin::CLIENT_ID("9b7cb759c6d41b14af05855f94bc743c");

const QStringList SoundCloudPlugin::AUDIO_FORMATS = QStringList() << "original" << "mp3";

SoundCloudPlugin::SoundCloudPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_resourcesRequest(0),
    m_streamsRequest(0)
{
}

void SoundCloudPlugin::checkUrl(const QString &url, const QVariantMap &) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QSoundCloud::ResourcesRequest(this);
        m_resourcesRequest->setClientId(CLIENT_ID);
        connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onResourcesRequestFinished()));
    }
    
    QVariantMap filters;
    filters["url"] = url;
    m_resourcesRequest->get("/resolve", filters);
}

void SoundCloudPlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    if (!m_streamsRequest) {
        m_streamsRequest = new QSoundCloud::StreamsRequest(this);
        m_streamsRequest->setClientId(CLIENT_ID);
        connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(onStreamsRequestFinished()));
    }
    
    m_settings = settings;
    m_streamsRequest->get(url);
}

void SoundCloudPlugin::submitFormat(const QVariantMap &format) {
    const QUrl url(format.value("audioFormat").toString());

    if (url.isEmpty()) {
        emit error(tr("Invalid audio format chosen"));
    }
    else {
        emit downloadRequest(QNetworkRequest(url));
    }
}

bool SoundCloudPlugin::cancelCurrentOperation() {
    if (m_resourcesRequest) {
        m_resourcesRequest->cancel();
    }
    
    if (m_streamsRequest) {
        m_streamsRequest->cancel();
    }

    return true;
}

void SoundCloudPlugin::onResourcesRequestFinished() {
    if (m_resourcesRequest->status() == QSoundCloud::ResourcesRequest::Ready) {
        const QVariantMap result = m_resourcesRequest->result().toMap();
        
        if (result.contains("tracks")) {
            QVariantList list = result.value("tracks").toList();
            
            if (list.isEmpty()) {
                emit error(tr("No tracks found"));
                return;
            }
            
            UrlResultList results;
            
            while (!list.isEmpty()) {
                const QVariantMap item = list.takeFirst().toMap();
                const QString title = item.value("title").toString();
                const QString url = item.value("permalink_url").toString();
                results << UrlResult(url, QString(title + ".mp3"));
            }
            
            emit urlChecked(results, result.value("title").toString());
        }
        else {
            const QString title = result.value("title").toString();
            const QString url = result.value("permalink_url").toString();
            emit urlChecked(UrlResult(url, QString(title + ".mp3")));
        }
    }
    else if (m_resourcesRequest->status() == QSoundCloud::ResourcesRequest::Failed) {
        emit error(m_resourcesRequest->errorString());
    }
}

void SoundCloudPlugin::onStreamsRequestFinished() {
    if (m_streamsRequest->status() == QSoundCloud::StreamsRequest::Ready) {
        const QVariantList streams = m_streamsRequest->result().toList();
        
        if (streams.isEmpty()) {
            emit error(tr("No streams found"));
            return;
        }

        if (m_settings.value("useDefaultAudioFormat", true).toBool()) {
            const QString format = m_settings.value("audioFormat", "original").toString();

            for (int i = 0; i < AUDIO_FORMATS.indexOf(format); i++) {
                for (int j = 0; j < streams.size(); j++) {
                    const QVariantMap stream = streams.at(j).toMap();

                    if (stream.value("id") == format) {
                        emit downloadRequest(QNetworkRequest(stream.value("url").toString()));
                        return;
                    }
                }
            }

            emit error(tr("No stream found for the chosen audio format"));
        }
        else {
            QVariantList settingsList;
            QVariantList options;
            QVariantMap list;
            list["type"] = "list";
            list["label"] = tr("Audio format");
            list["key"] = "audioFormat";
            list["value"] = streams.first().toMap().value("url");
            
            for (int i = 0; i < streams.size(); i++) {
                const QVariantMap stream = streams.at(i).toMap();
                QVariantMap option;
                option["label"] = stream.value("description");
                option["value"] = stream.value("url");
                options << option;
            }

            list["options"] = options;
            settingsList << list;
            emit settingsRequest(tr("Choose audio format"), settingsList, "submitFormat");
        }
    }
    else if (m_streamsRequest->status() == QSoundCloud::StreamsRequest::Failed) {
        emit error(m_streamsRequest->errorString());
    }   
}

ServicePlugin* SoundCloudPluginFactory::createPlugin(QObject *parent) {
    return new SoundCloudPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-soundcloud, SoundCloudPluginFactory)
#endif
