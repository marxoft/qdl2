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

#include "dailymotionsearchplugin.h"
#include <qdailymotion/resourcesrequest.h>
#include <QDateTime>
#include <QUrl>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString DailymotionSearchPlugin::HTML = QObject::tr("<a href='%1'><img width='320' height='180' src='%2' /></a><p>Date: %3</p><p>Duration: %4</p><p>%5</p>");

const QString DailymotionSearchPlugin::PLAYLIST_FIELDS("id,created_time,description,name,thumbnail_180_url");
const QString DailymotionSearchPlugin::VIDEO_FIELDS("id,created_time,description,duration,thumbnail_180_url,title");

DailymotionSearchPlugin::DailymotionSearchPlugin(QObject *parent) :
    SearchPlugin(parent),
    m_request(0)
{
}

bool DailymotionSearchPlugin::cancelCurrentOperation() {
    if (m_request) {
        m_request->cancel();
    }
    
    m_filters.clear();
    return true;
}

void DailymotionSearchPlugin::fetchMore(const QVariantMap &params) {
    request()->list(params.value("path").toString(), params.value("filters").toMap());
}

void DailymotionSearchPlugin::search(const QVariantMap &settings) {
    m_filters.clear();
    
    if (!settings.value("useDefaultSearchOptions", false).toBool()) {
        QVariantMap searchQuery;
        searchQuery["type"] = "text";
        searchQuery["label"] = tr("Search query");
        searchQuery["key"] = "searchQuery";
        QVariantMap searchType;
        QVariantMap videos;
        videos["label"] = tr("Videos");
        videos["value"] = "/videos";
        QVariantMap playlists;
        playlists["label"] = tr("Playlists");
        playlists["value"] = "/playlists";
        searchType["type"] = "list";
        searchType["label"] = tr("Search type");
        searchType["key"] = "searchType";
        searchType["value"] = "/videos";
        searchType["options"] = QVariantList() << videos << playlists;
        QVariantMap searchOrder;
        QVariantMap date;
        date["label"] = tr("Date");
        date["value"] = "recent";
        QVariantMap relevance;
        relevance["label"] = tr("Relevance");
        relevance["value"] = "relevance";
        searchOrder["type"] = "list";
        searchOrder["label"] = tr("Search order");
        searchOrder["key"] = "searchOrder";
        searchOrder["value"] = "relevance";
        searchOrder["options"] = QVariantList() << date << relevance;
        QVariantMap familyFilter;
        familyFilter["type"] = "boolean";
        familyFilter["label"] = tr("Enable family filter");
        familyFilter["key"] = "familyFilterEnabled";
        familyFilter["value"] = false;
        emit settingsRequest(tr("Choose search options"), QVariantList() << searchQuery << searchType << searchOrder
                             << familyFilter, "submitSettings");
        return;
    }
    
    const QString type = settings.value("searchType", "/videos").toString();
    m_filters["search"] = settings.value("searchQuery").toString();
    m_filters["sort"] = settings.value("searchOrder", "relevance").toString();
    m_filters["family_filter"] = settings.value("familyFilterEnabled", false).toBool();
    m_filters["limit"] = 20;
    m_filters["fields"] = (type == "/playlists" ? PLAYLIST_FIELDS : VIDEO_FIELDS);
    request()->list(type, m_filters);
}

void DailymotionSearchPlugin::submitSettings(const QVariantMap &settings) {
    const QString type = settings.value("searchType", "/videos").toString();
    m_filters["search"] = settings.value("searchQuery").toString();
    m_filters["sort"] = settings.value("searchOrder", "relevance").toString();
    m_filters["family_filter"] = settings.value("familyFilterEnabled", false).toBool();
    m_filters["limit"] = 20;
    m_filters["fields"] = (type == "/playlists" ? PLAYLIST_FIELDS : VIDEO_FIELDS);
    request()->list(type, m_filters);
}

QDailymotion::ResourcesRequest* DailymotionSearchPlugin::request() {
    if (!m_request) {
        m_request = new QDailymotion::ResourcesRequest(this);
        connect(m_request, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }
    
    return m_request;
}

void DailymotionSearchPlugin::onRequestFinished() {
    if (m_request->status() == QDailymotion::ResourcesRequest::Ready) {
        SearchResultList results;
        const QVariantMap result = m_request->result().toMap();
        const QVariantList list = result.value("list").toList();
        const QString path = m_request->url().path();
        const QString baseUrl = (path.startsWith("/playlists") ? QString("https://www.dailymotion.com/playlist/") :
                                 QString("https://www.dailymotion.com/video/"));
        
        foreach (const QVariant &v, list) {
            const QVariantMap item = v.toMap();
            const QString title = (item.contains("title") ? item.value("title").toString()
                                   : item.value("name").toString());
            const QString url = baseUrl + item.value("id").toString();
            const QString thumbnailUrl = item.value("thumbnail_180_url").toString();
            const QString date = QDateTime::fromTime_t(item.value("created_time").toUInt()).toString("dd MMM yyyy");
            const int secs = item.value("duration", 0).toInt();
            const QString duration = (secs > 0 ? QString("%1:%2").arg(secs / 60, 2, 10, QChar('0'))
                                      .arg(secs % 60, 2, 10, QChar('0')) : QString("--:--"));
            const QString description = item.value("description").toString();
            const QString html = HTML.arg(url).arg(thumbnailUrl).arg(date).arg(duration).arg(description);
                
            results << SearchResult(title, html, url);
        }
                
        if (result.value("has_more", false).toBool()) {
            m_filters["page"] = qMax(2, m_filters.value("page", 1).toInt() + 1);
            QVariantMap params;
            params["path"] = path;
            params["filters"] = m_filters;
            emit searchCompleted(results, params);
        }
        else {
            emit searchCompleted(results);
        }
    }
    else if (m_request->status() == QDailymotion::ResourcesRequest::Failed) {
        emit error(m_request->errorString());
    }
}

SearchPlugin* DailymotionSearchPluginFactory::createPlugin(QObject *parent) {
    return new DailymotionSearchPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-dailymotionsearch, DailymotionSearchPluginFactory)
#endif
