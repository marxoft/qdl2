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

#include "vimeosearchplugin.h"
#include <qvimeo/resourcesrequest.h>
#include <QDateTime>
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

const QString VimeoSearchPlugin::BASE_URL("https://vimeo.com");

const QString VimeoSearchPlugin::HTML = QObject::tr("<a href='%1'><img width='320' height='180' src='https://i.vimeocdn.com/video/%2_320x180.jpg' /></a><p>Date: %3</p><p>Duration: %4</p><p>%5</p>");

const QString VimeoSearchPlugin::CLIENT_ID("0bf284bf5a0e46630f5097a590a76ef976a94322");
const QString VimeoSearchPlugin::CLIENT_SECRET("7nnZ1OPS13hjKAVhzuXx/4AIdKxmgDNasHkj5QraWWkrNsd6mxYWZG73AKaFUdLzoNWhGA75jSffs+JyAFfi0MiFi1OXnzHsxaL0HCIFpxk0GpZlXcScWmJTHvGGtVv1");
const QString VimeoSearchPlugin::CLIENT_TOKEN("fb5c66ebe6938e858921963f850355a0");

VimeoSearchPlugin::VimeoSearchPlugin(QObject *parent) :
    SearchPlugin(parent),
    m_request(0)
{
}

bool VimeoSearchPlugin::cancelCurrentOperation() {
    if (m_request) {
        m_request->cancel();
    }
    
    m_filters.clear();
    return true;
}

void VimeoSearchPlugin::fetchMore(const QVariantMap &params) {
    request()->list("/videos", params);
}

void VimeoSearchPlugin::search(const QVariantMap &settings) {
    m_filters.clear();
    
    if (!settings.value("useDefaultSearchOptions", false).toBool()) {
        QVariantMap searchQuery;
        searchQuery["type"] = "text";
        searchQuery["label"] = tr("Search query");
        searchQuery["key"] = "searchQuery";
        QVariantMap searchOrder;
        QVariantMap date;
        date["label"] = tr("Date");
        date["value"] = "date";
        QVariantMap duration;
        duration["label"] = tr("Duration");
        duration["value"] = "duration";
        QVariantMap likes;
        likes["label"] = tr("Likes");
        likes["value"] = "likes";
        QVariantMap relevance;
        relevance["label"] = tr("Relevance");
        relevance["value"] = "relevant";
        QVariantMap title;
        title["label"] = tr("Title");
        title["value"] = "alphabetical";
        QVariantMap views;
        views["label"] = tr("Views");
        views["value"] = "plays";
        searchOrder["type"] = "list";
        searchOrder["label"] = tr("Search order");
        searchOrder["key"] = "searchOrder";
        searchOrder["value"] = "relevant";
        searchOrder["options"] = QVariantList() << date << duration << likes << relevance << title << views;
        emit settingsRequest(tr("Choose search options"), QVariantList() << searchQuery << searchOrder, "submitSettings");
        return;
    }
    
    m_filters["query"] = settings.value("searchQuery").toString();
    m_filters["sort"] = settings.value("searchOrder", "relevant").toString();
    m_filters["per_page"] = 20;
    request()->list(settings.value("searchType", "/videos").toString(), m_filters);
}

void VimeoSearchPlugin::submitSettings(const QVariantMap &settings) {
    m_filters["query"] = settings.value("searchQuery").toString();
    m_filters["sort"] = settings.value("searchOrder", "relevant").toString();
    m_filters["per_page"] = 20;
    request()->list("/videos", m_filters);
}

QVimeo::ResourcesRequest* VimeoSearchPlugin::request() {
    if (!m_request) {
        m_request = new QVimeo::ResourcesRequest(this);
        m_request->setClientId(CLIENT_ID);
        m_request->setClientSecret(CLIENT_SECRET);
        m_request->setAccessToken(CLIENT_TOKEN);
        connect(m_request, SIGNAL(finished()), this, SLOT(onRequestFinished()));
    }
    
    return m_request;
}

void VimeoSearchPlugin::onRequestFinished() {
    if (m_request->status() == QVimeo::ResourcesRequest::Ready) {
        SearchResultList results;
        const QVariantMap result = m_request->result().toMap();
        const QVariantList list = result.value("data").toList();
        
        foreach (const QVariant &v, list) {
            const QVariantMap item = v.toMap();
            const QString title = item.value("name").toString();
            const QString url = BASE_URL + item.value("uri").toString();
            const QString thumbnailId = item.value("pictures").toMap().value("uri").toString().section("/", -1);
            const QString date = QDateTime::fromString(item.value("created_time").toString(), Qt::ISODate)
                .toString("dd MMM yyyy");
            const int secs = item.value("duration", 0).toInt();
            const QString duration = (secs > 0 ? QString("%1:%2").arg(secs / 60, 2, 10, QChar('0'))
                                      .arg(secs % 60, 2, 10, QChar('0')) : QString("--:--"));
            const QString description = item.value("description").toString();
            const QString html = HTML.arg(url).arg(thumbnailId).arg(date).arg(duration).arg(description);
                
            results << SearchResult(title, html, url);
        }
        
        if (!result.value("paging").toMap().value("next").isNull()) {
            m_filters["page"] = qMax(2, m_filters.value("page", 1).toInt() + 1);
            emit searchCompleted(results, m_filters);
        }
        else {
            emit searchCompleted(results);
        }
    }
    else if (m_request->status() == QVimeo::ResourcesRequest::Failed) {
        emit error(m_request->errorString());
    }
}

SearchPlugin* VimeoSearchPluginFactory::createPlugin(QObject *parent) {
    return new VimeoSearchPlugin(parent);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qdl2-vimeosearch, VimeoSearchPluginFactory)
#endif
