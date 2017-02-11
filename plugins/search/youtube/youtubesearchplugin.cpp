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

#include "youtubesearchplugin.h"
#include <qyoutube/resourcesrequest.h>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString YouTubeSearchPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-youtubesearch");
#else
const QString YouTubeSearchPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-youtubesearch");
#endif
const QString YouTubeSearchPlugin::API_KEY("AIzaSyDhIlkLzHJKDCNr6thsjlQpZrkY3lO_Uu4");
const QString YouTubeSearchPlugin::CLIENT_ID("957843447749-ur7hg6de229ug0svjakaiovok76s6ecr.apps.googleusercontent.com");
const QString YouTubeSearchPlugin::CLIENT_SECRET("dDs2_WwgS16LZVuzqA9rIg-I");

YouTubeSearchPlugin::YouTubeSearchPlugin(QObject *parent) :
    SearchPlugin(parent),
    m_request(0)
{
}

SearchPlugin* YouTubeSearchPlugin::createPlugin(QObject *parent) {
    return new YouTubeSearchPlugin(parent);
}

bool YouTubeSearchPlugin::cancelCurrentOperation() {
    if (m_request) {
        m_request->cancel();
    }
    
    m_params.clear();
    return true;
}

void YouTubeSearchPlugin::fetchMore(const QVariantMap &params) {
    request()->list("/search", QStringList() << "snippet", params);
}

void YouTubeSearchPlugin::search(const QString &query) {
    m_params["q"] = query;
    const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    
    if (!settings.value("useDefaultSearchOptions", false).toBool()) {
        QVariantMap searchType;
        QVariantMap videos;
        videos["label"] = tr("Videos");
        videos["value"] = "video";
        QVariantMap playlists;
        playlists["label"] = tr("Playlists");
        playlists["value"] = "playlist";
        searchType["type"] = "list";
        searchType["label"] = tr("Search type");
        searchType["key"] = "searchType";
        searchType["value"] = "video";
        searchType["options"] = QVariantList() << videos << playlists;
        QVariantMap searchOrder;
        QVariantMap date;
        date["label"] = tr("Date");
        date["value"] = "date";
        QVariantMap rating;
        rating["label"] = tr("Rating");
        rating["value"] = "rating";
        QVariantMap relevance;
        relevance["label"] = tr("Relevance");
        relevance["value"] = "relevance";
        QVariantMap title;
        title["label"] = tr("Title");
        title["value"] = "title";
        QVariantMap views;
        views["label"] = tr("Views");
        views["value"] = "viewCount";
        searchOrder["type"] = "list";
        searchOrder["label"] = tr("Search order");
        searchOrder["key"] = "searchOrder";
        searchOrder["value"] = "relevance";
        searchOrder["options"] = QVariantList() << date << rating << relevance << title << views;
        QVariantMap safeSearch;
        QVariantMap none;
        none["label"] = tr("None");
        none["value"] = "none";
        QVariantMap moderate;
        moderate["label"] = tr("Moderate");
        moderate["value"] = "moderate";
        QVariantMap strict;
        strict["label"] = tr("Strict");
        strict["value"] = "strict";
        safeSearch["type"] = "list";
        safeSearch["label"] = tr("Safe search");
        safeSearch["key"] = "safeSearch";
        safeSearch["value"] = "none";
        safeSearch["options"] = QVariantList() << none << moderate << strict;
        emit settingsRequest(tr("Choose search options"), QVariantList() << searchType << searchOrder << safeSearch,
                             "submitSettings");
        return;
    }
    
    m_params["type"] = settings.value("searchType", "video").toString();
    m_params["order"] = settings.value("searchOrder", "relevance").toString();
    m_params["safeSearch"] = settings.value("safeSearch", "none").toString();
    m_params["maxResults"] = 20;
    request()->list("/search", QStringList() << "snippet", QVariantMap(), m_params);
}

void YouTubeSearchPlugin::submitSettings(const QVariantMap &settings) {
    m_params["type"] = settings.value("searchType", "video").toString();
    m_params["order"] = settings.value("searchOrder", "relevance").toString();
    m_params["safeSearch"] = settings.value("safeSearch", "none").toString();
    m_params["maxResults"] = 20;
    request()->list("/search", QStringList() << "snippet", QVariantMap(), m_params);
}

QYouTube::ResourcesRequest* YouTubeSearchPlugin::request() {
    if (!m_request) {
        m_request = new QYouTube::ResourcesRequest(this);
        m_request->setApiKey(API_KEY);
        m_request->setClientId(CLIENT_ID);
        m_request->setClientSecret(CLIENT_SECRET);
        connect(m_request, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }
    
    return m_request;
}

void YouTubeSearchPlugin::onRequestFinished() {
    if (m_request->status() == QYouTube::ResourcesRequest::Ready) {
        SearchResultList results;
        const QVariantMap result = m_request->result().toMap();
        const QVariantList list = result.value("items").toList();
        
        foreach (const QVariant &v, list) {
            const QVariantMap item = v.toMap();
            const QVariantMap snippet = item.value("snippet").toMap();
            const QString title = snippet.value("title").toString();
            const QVariantMap id = item.value("id").toMap();
            QString url;
            
            if (id.value("kind") == "playlist") {
                url = QString("https://www.youtube.com/playlist?list=" + id.value("playlistId").toString());
            }
            else {
                url = QString("https://www.youtube.com/watch?v=" + id.value("videoId").toString());
            }
            
            const QString description =
                QString("<a href=\"%1\"><img src=\"%2\" width=\"480\" height=\"360\" /></a><p>%3")
                .arg(url).arg(snippet.value("thumbnails").toMap().value("high").toMap().value("url").toString())
                .arg(snippet.value("description").toString());
                
            results << SearchResult(title, description, url);
        }
        
        const QString token = result.value("nextPageToken").toString();
        
        if (!token.isEmpty()) {
            m_params["pageToken"] = token;
            emit searchCompleted(results, m_params);
        }
        else {
            emit searchCompleted(results);
        }
    }
    else if (m_request->status() == QYouTube::ResourcesRequest::Failed) {
        emit error(m_request->errorString());
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-youtubesearch, YouTubeSearchPlugin)
#endif
