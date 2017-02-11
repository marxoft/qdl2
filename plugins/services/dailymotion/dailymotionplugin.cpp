/*
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

#include "dailymotionplugin.h"
#include <qdailymotion/resourcesrequest.h>
#include <qdailymotion/streamsrequest.h>
#include <QNetworkRequest>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString DailymotionPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-dailymotion");
#else
const QString DailymotionPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-dailymotion");
#endif
const QString DailymotionPlugin::VIDEO_FIELDS("id,title,url");

const QStringList DailymotionPlugin::VIDEO_FORMATS = QStringList() << "2160" << "1080" << "720" << "480" << "380"
                                                                   << "240" << "18" << "144";

DailymotionPlugin::DailymotionPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_resourcesRequest(0),
    m_streamsRequest(0)
{
}

ServicePlugin* DailymotionPlugin::createPlugin(QObject *parent) {
    return new DailymotionPlugin(parent);
}

void DailymotionPlugin::checkUrl(const QString &url) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QDailymotion::ResourcesRequest(this);
        connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onResourcesRequestFinished()));
    }

    m_results.clear();
    m_filters.clear();
    const QString id = url.section('/', -1).section('_', 0, 0);
    
    if (url.contains("/playlist/")) {
        m_filters["limit"] = 50;
        m_filters["fields"] = VIDEO_FIELDS;
        m_resourcesRequest->list(QString("/playlist/%1/videos").arg(id), m_filters);
    }
    else {
        m_filters["fields"] = VIDEO_FIELDS;
        m_resourcesRequest->get("/video/" + id, m_filters);
    }
}

void DailymotionPlugin::getDownloadRequest(const QString &url) {    
    if (!m_streamsRequest) {
        m_streamsRequest = new QDailymotion::StreamsRequest(this);
        connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(onStreamsRequestFinished()));
    }
    
    const QString id = url.section('/', -1).section('_', 0, 0);
    m_streamsRequest->list(id);
}

void DailymotionPlugin::submitFormat(const QVariantMap &format) {
    const QUrl url(format.value("videoFormat").toString());

    if (url.isEmpty()) {
        emit error(tr("Invalid video format chosen"));
    }
    else {
        emit downloadRequest(QNetworkRequest(url));
    }
}

bool DailymotionPlugin::cancelCurrentOperation() {
    if (m_resourcesRequest) {
        m_resourcesRequest->cancel();
    }
    
    if (m_streamsRequest) {
        m_streamsRequest->cancel();
    }

    m_results.clear();
    m_filters.clear();
    m_streams.clear();
    return true;
}

void DailymotionPlugin::onResourcesRequestFinished() {
    if (m_resourcesRequest->status() == QDailymotion::ResourcesRequest::Ready) {
        const QVariantMap result = m_resourcesRequest->result().toMap();
        const QString path = m_resourcesRequest->url().path();
        
        if (path.startsWith("/playlist/")) {
            QVariantList list = result.value("list").toList();
            
            if (list.isEmpty()) {
                emit error(tr("No videos found"));
                return;
            }
            
            while (!list.isEmpty()) {
                const QVariantMap item = list.takeFirst().toMap();
                const QString title = item.value("title").toString();
                const QString url = item.value("url").toString();
                m_results << UrlResult(url, QString(title + ".mp4"));
            }
            
            if (!result.value("has_more", false).toBool()) {
                emit urlChecked(m_results, m_results.first().fileName.section(".", 0, -2));
            }
            else {
                m_filters["page"] = m_filters.value("page", 1).toInt() + 1;
                m_resourcesRequest->list(path, m_filters);
            }
        }
        else {
            const QString title = result.value("title").toString();
            const QString url = result.value("url").toString();
            emit urlChecked(UrlResult(url, QString(title + ".mp4")));
        }
    }
    else if (m_resourcesRequest->status() == QDailymotion::ResourcesRequest::Failed) {
        emit error(m_resourcesRequest->errorString());
    }
}

void DailymotionPlugin::onStreamsRequestFinished() {
    if (m_streamsRequest->status() == QDailymotion::StreamsRequest::Ready) {
        m_streams = m_streamsRequest->result().toList();
        
        if (m_streams.isEmpty()) {
            emit error(tr("No streams found"));
            return;
        }

        const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
        
        if (settings.value("useDefaultVideoFormat", true).toBool()) {
            const QString format = settings.value("videoFormat", "2160").toString();

            for (int i = 0; i < VIDEO_FORMATS.indexOf(format); i++) {
                for (int j = 0; j < m_streams.size(); j++) {
                    const QVariantMap stream = m_streams.at(j).toMap();

                    if (stream.value("id") == format) {
                        emit downloadRequest(QNetworkRequest(stream.value("url").toString()));
                        return;
                    }
                }
            }

            emit error(tr("No stream found for the chosen video format"));
        }
        else {
            QVariantList settingsList;
            QVariantList options;
            QVariantMap list;
            list["type"] = "list";
            list["label"] = tr("Video format");
            list["key"] = "videoFormat";
            list["value"] = m_streams.first().toMap().value("id");
            
            for (int i = 0; i < m_streams.size(); i++) {
                const QVariantMap stream = m_streams.at(i).toMap();
                QVariantMap option;
                option["label"] = QString("%1P").arg(stream.value("height").toString());
                option["value"] = stream.value("url");
                options << option;
            }

            list["options"] = options;
            settingsList << list;
            emit settingsRequest(tr("Choose video format"), settingsList, "submitFormat");
        }
    }
    else if (m_streamsRequest->status() == QDailymotion::StreamsRequest::Failed) {
        emit error(m_streamsRequest->errorString());
    }   
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-dailymotion, DailymotionPlugin)
#endif
