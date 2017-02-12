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

#include "vimeoplugin.h"
#include <qvimeo/resourcesrequest.h>
#include <qvimeo/streamsrequest.h>
#include <QNetworkRequest>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString VimeoPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-vimeo");
#else
const QString VimeoPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-vimeo");
#endif
const QString VimeoPlugin::BASE_URL("https://vimeo.com");

const QString VimeoPlugin::CLIENT_ID("0bf284bf5a0e46630f5097a590a76ef976a94322");
const QString VimeoPlugin::CLIENT_SECRET("7nnZ1OPS13hjKAVhzuXx/4AIdKxmgDNasHkj5QraWWkrNsd6mxYWZG73AKaFUdLzoNWhGA75jSffs+JyAFfi0MiFi1OXnzHsxaL0HCIFpxk0GpZlXcScWmJTHvGGtVv1");
const QString VimeoPlugin::CLIENT_TOKEN("fb5c66ebe6938e858921963f850355a0");

const QStringList VimeoPlugin::VIDEO_FORMATS = QStringList() << "1080p" << "720p" << "540p" << "360p";

VimeoPlugin::VimeoPlugin(QObject *parent) :
    ServicePlugin(parent),
    m_resourcesRequest(0),
    m_streamsRequest(0)
{
}

ServicePlugin* VimeoPlugin::createPlugin(QObject *parent) {
    return new VimeoPlugin(parent);
}

void VimeoPlugin::checkUrl(const QString &url) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QVimeo::ResourcesRequest(this);
        m_resourcesRequest->setClientId(CLIENT_ID);
        m_resourcesRequest->setClientSecret(CLIENT_SECRET);
        m_resourcesRequest->setAccessToken(CLIENT_TOKEN);
        connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onResourcesRequestFinished()));
    }

    m_results.clear();
    const QString id = url.mid(url.lastIndexOf("/") + 1);
    m_resourcesRequest->get("/videos/" + id);
}

void VimeoPlugin::getDownloadRequest(const QString &url) {    
    if (!m_streamsRequest) {
        m_streamsRequest = new QVimeo::StreamsRequest(this);
        connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(onStreamsRequestFinished()));
    }
    
    const QString id = url.mid(url.lastIndexOf("/") + 1);
    m_streamsRequest->list(id);
}

void VimeoPlugin::submitFormat(const QVariantMap &format) {
    const QUrl url(format.value("videoFormat").toString());

    if (url.isEmpty()) {
        emit error(tr("Invalid video format chosen"));
    }
    else {
        emit downloadRequest(QNetworkRequest(url));
    }
}

bool VimeoPlugin::cancelCurrentOperation() {
    if (m_resourcesRequest) {
        m_resourcesRequest->cancel();
    }
    
    if (m_streamsRequest) {
        m_streamsRequest->cancel();
    }

    m_results.clear();
    return true;
}

void VimeoPlugin::onResourcesRequestFinished() {
    if (m_resourcesRequest->status() == QVimeo::ResourcesRequest::Ready) {
        const QVariantMap result = m_resourcesRequest->result().toMap();
        const QString title = result.value("name").toString();
        const QString url = BASE_URL + result.value("uri").toString();
        emit urlChecked(UrlResult(url, QString(title + ".mp4")));
    }
    else if (m_resourcesRequest->status() == QVimeo::ResourcesRequest::Failed) {
        emit error(m_resourcesRequest->errorString());
    }
}

void VimeoPlugin::onStreamsRequestFinished() {
    if (m_streamsRequest->status() == QVimeo::StreamsRequest::Ready) {
        const QVariantList streams = m_streamsRequest->result().toList();
        
        if (streams.isEmpty()) {
            emit error(tr("No streams found"));
            return;
        }

        const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
        
        if (settings.value("useDefaultVideoFormat", true).toBool()) {
            const QString format = settings.value("videoFormat", "1080p").toString();

            for (int i = 0; i < VIDEO_FORMATS.indexOf(format); i++) {
                for (int j = 0; j < streams.size(); j++) {
                    const QVariantMap stream = streams.at(j).toMap();

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
            list["value"] = streams.first().toMap().value("url");
            
            for (int i = 0; i < streams.size(); i++) {
                const QVariantMap stream = streams.at(i).toMap();
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
    else if (m_streamsRequest->status() == QVimeo::StreamsRequest::Failed) {
        emit error(m_streamsRequest->errorString());
    }   
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-vimeo, VimeoPlugin)
#endif
