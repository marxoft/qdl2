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

#include "dailymotionsearchplugin.h"
#include <qdailymotion/resourcesrequest.h>
#include <QSettings>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#include <QtPlugin>
#endif

#if QT_VERSION >= 0x050000
const QString DailymotionSearchPlugin::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-dailymotionsearch");
#else
const QString DailymotionSearchPlugin::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                         + "/.config/qdl2/plugins/qdl2-dailymotionsearch");
#endif

const QString DailymotionSearchPlugin::FIELDS("id,description,thumbnail_360_url,title,url");

DailymotionSearchPlugin::DailymotionSearchPlugin(QObject *parent) :
    SearchPlugin(parent),
    m_request(0)
{
}

SearchPlugin* DailymotionSearchPlugin::createPlugin(QObject *parent) {
    return new DailymotionSearchPlugin(parent);
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

void DailymotionSearchPlugin::search(const QString &query) {
    m_filters.clear();
    m_filters["search"] = query;
    const QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    
    if (!settings.value("useDefaultSearchOptions", false).toBool()) {
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
        emit settingsRequest(tr("Choose search options"), QVariantList() << searchType << searchOrder << familyFilter,
                             "submitSettings");
        return;
    }
    
    m_filters["sort"] = settings.value("searchOrder", "relevance").toString();
    m_filters["family_filter"] = settings.value("familyFilterEnabled", false).toBool();
    m_filters["limit"] = 20;
    m_filters["fields"] = FIELDS;
    request()->list(settings.value("searchType", "/videos").toString(), m_filters);
}

void DailymotionSearchPlugin::submitSettings(const QVariantMap &settings) {
    m_filters["sort"] = settings.value("searchOrder", "relevance").toString();
    m_filters["family_filter"] = settings.value("familyFilterEnabled", false).toBool();
    m_filters["limit"] = 20;
    m_filters["fields"] = FIELDS;
    request()->list(settings.value("searchType", "/videos").toString(), m_filters);
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
        
        foreach (const QVariant &v, list) {
            const QVariantMap item = v.toMap();
            const QString title = item.value("title").toString();
            const QString url = item.value("url").toString();
            const QString description =
                QString("<a href=\"%1\"><img src=\"%2\" width=\"480\" height=\"270\" /></a><p>%3")
                .arg(url).arg(item.value("thumbnail_360_url").toString())
                .arg(item.value("description").toString());
                
            results << SearchResult(title, description, url);
        }
                
        if (result.value("has_more", false).toBool()) {
            m_filters["page"] = m_filters.value("page", 1).toInt() + 1;
            QVariantMap params;
            params["path"] = m_request->url().path();
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

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-dailymotionsearch, DailymotionSearchPlugin)
#endif
