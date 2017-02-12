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

#include "soundcloudsearchplugin.h"
#include <qsoundcloud/resourcesrequest.h>
#include <QSettings>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString SoundCloudSearchPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-soundcloudsearch");
#else
const QString SoundCloudSearchPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-soundcloudsearch");
#endif

const QString SoundCloudSearchPlugin::CLIENT_ID("9b7cb759c6d41b14af05855f94bc743c");

SoundCloudSearchPlugin::SoundCloudSearchPlugin(QObject *parent) :
    SearchPlugin(parent),
    m_request(0)
{
}

SearchPlugin* SoundCloudSearchPlugin::createPlugin(QObject *parent) {
    return new SoundCloudSearchPlugin(parent);
}

bool SoundCloudSearchPlugin::cancelCurrentOperation() {
    if (m_request) {
        m_request->cancel();
    }
    
    return true;
}

void SoundCloudSearchPlugin::fetchMore(const QVariantMap &params) {
    request()->get(params.value("path").toString());
}

void SoundCloudSearchPlugin::search() {
    const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    
    if (!settings.value("useDefaultSearchOptions", false).toBool()) {
        QVariantMap searchQuery;
        searchQuery["type"] = "text";
        searchQuery["label"] = tr("Search query");
        searchQuery["key"] = "searchQuery";
        QVariantMap searchType;
        QVariantMap playlists;
        playlists["label"] = tr("Sets");
        playlists["value"] = "/playlists";
        QVariantMap tracks;
        tracks["label"] = tr("Tracks");
        tracks["value"] = "/tracks";
        searchType["type"] = "list";
        searchType["label"] = tr("Search type");
        searchType["key"] = "searchType";
        searchType["value"] = "/tracks";
        searchType["options"] = QVariantList() << playlists << tracks;
        emit settingsRequest(tr("Choose search options"), QVariantList() << searchQuery << searchType, "submitSettings");
        return;
    }
    
    QVariantMap filters;
    filters["q"] = settings.value("searchQuery").toString();
    filters["limit"] = 20;
    filters["representation"] = "compact";
    filters["linked_partitioning"] = true;
    request()->get(settings.value("searchType", "/tracks").toString(), filters);
}

void SoundCloudSearchPlugin::submitSettings(const QVariantMap &settings) {
    QVariantMap filters;
    filters["q"] = settings.value("searchQuery").toString();
    filters["limit"] = 20;
    filters["representation"] = "compact";
    filters["linked_partitioning"] = true;
    request()->get(settings.value("searchType", "/tracks").toString(), filters);
}

QSoundCloud::ResourcesRequest* SoundCloudSearchPlugin::request() {
    if (!m_request) {
        m_request = new QSoundCloud::ResourcesRequest(this);
        m_request->setClientId(CLIENT_ID);
        connect(m_request, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }
    
    return m_request;
}

void SoundCloudSearchPlugin::onRequestFinished() {
    if (m_request->status() == QSoundCloud::ResourcesRequest::Ready) {
        SearchResultList results;
        const QVariantMap result = m_request->result().toMap();
        const QVariantList list = result.value("collection").toList();
        
        foreach (const QVariant &v, list) {
            const QVariantMap item = v.toMap();
            const QString title = item.value("title").toString();
            const QString url = item.value("permalink_url").toString();
            const QString description =
                QString("<a href=\"%1\"><img src=\"%2-t300x300.jpg\" width=\"300\" height=\"300\" /></a><p>%3")
                .arg(url).arg(item.value("artwork_url").toString().section("-", 0, -2))
                .arg(item.value("description").toString());
                
            results << SearchResult(title, description, url);
        }
        
        const QString next = result.value("next_href").toString();
        
        if (!next.isEmpty()) {
            QVariantMap params;
            params["path"] = next.mid(next.lastIndexOf("/"));
            emit searchCompleted(results, params);
        }
        else {
            emit searchCompleted(results);
        }
    }
    else if (m_request->status() == QSoundCloud::ResourcesRequest::Failed) {
        emit error(m_request->errorString());
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-soundcloudsearch, SoundCloudSearchPlugin)
#endif
