/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "searchmodel.h"
#include "definitions.h"
#include "logger.h"
#include "pluginsettings.h"
#include "searchpluginmanager.h"
#include "servicepluginmanager.h"
#include "transfermodel.h"
#include "utils.h"
#include <QIcon>

SearchModel::SearchModel(QObject *parent) :
    QAbstractListModel(parent),
    m_plugin(0),
    m_status(Idle)
{
    m_roles[NameRole] = "name";
    m_roles[IconFilePathRole] = "iconFilePath";
    m_roles[DescriptionRole] = "description";
    m_roles[UrlRole] = "url";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

SearchModel::~SearchModel() {
    cancel();
}

QString SearchModel::errorString() const {
    return m_errorString;
}

void SearchModel::setErrorString(const QString &e) {
    m_errorString = e;
    
    if (!e.isEmpty()) {
        Logger::log("SearchModel::error(). " + e);
    }
}

SearchModel::Status SearchModel::status() const {
    return m_status;
}

void SearchModel::setStatus(SearchModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString SearchModel::statusString() const {
    switch (status()) {
    case Active:
    case AwaitingSettingsResponse:
        return tr("Searching");
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    case Error:
        return tr("Error");
    default:
        return QString();
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> SearchModel::roleNames() const {
    return m_roles;
}
#endif

int SearchModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

bool SearchModel::canFetchMore(const QModelIndex &) const {
    return ((status() == Completed) && (!m_next.isEmpty()));
}

void SearchModel::fetchMore(const QModelIndex &) {
    if (!canFetchMore()) {
        return;
    }
    
    if (SearchPlugin *p = plugin()) {
        setStatus(Active);
        p->fetchMore(m_next);
    }
}

QVariant SearchModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DecorationRole:
        return QIcon(m_items.at(index.row()).iconFilePath);
    case NameRole:
        return m_items.at(index.row()).name;
    case IconFilePathRole:
        return m_items.at(index.row()).iconFilePath;
    case DescriptionRole:
        return m_items.at(index.row()).description;
    case UrlRole:
        return m_items.at(index.row()).url;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> SearchModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant SearchModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap SearchModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList SearchModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                      Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int SearchModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void SearchModel::search(const QString &pluginId) {
    if (status() == Active) {
        return;
    }
    
    if ((pluginId != m_pluginId) && (m_plugin)) {
        delete m_plugin;
    }
    
    m_pluginId = pluginId;
    
    if (SearchPlugin *p = plugin()) {
        setStatus(Active);
        p->search(PluginSettings(pluginId).values());
    }
    else {
        Logger::log("SearchModel::search(). No plugin acquired.");
    }
}

void SearchModel::cancel() {
    if (m_plugin) {
        if (m_plugin->cancelCurrentOperation()) {
            setStatus(Canceled);
        }
    }
}

void SearchModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        setStatus(Idle);
        beginResetModel();
        m_items.clear();
        m_next.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void SearchModel::reload() {
    clear();
    
    if (SearchPlugin *p = plugin()) {
        setStatus(Active);
        p->search(PluginSettings(m_pluginId).values());
    }
    else {
        Logger::log("SearchModel::reload(). No plugin acquired.");
    }
}

bool SearchModel::submitSettingsResponse(const QVariantMap &settings) {
    if (status() == AwaitingSettingsResponse) {        
        if (SearchPlugin *p = plugin()) {            
            if (QMetaObject::invokeMethod(p, m_callback, Q_ARG(QVariantMap, settings))) {
                Logger::log("SearchModel::submitSettingsResponse(): Callback successful: "
                            + m_callback, Logger::MediumVerbosity);
                setStatus(Active);
                return true;
            }
            
            Logger::log("SearchModel::submitSettingsResponse(): Error calling callback: "
                        + m_callback);
        }
        else {
            Logger::log("SearchModel::submitSettingsResponse(): No plugin acquired");
        }
    }
    else {
        Logger::log("SearchModel::submitSettingsResponse(): Not awaiting settings response", Logger::MediumVerbosity);
    }
    
    return false;
}

SearchPlugin* SearchModel::plugin() {
    if (!m_plugin) {
        m_plugin = SearchPluginManager::instance()->createPluginById(m_pluginId, this);
        
        if (m_plugin) {
            connect(m_plugin, SIGNAL(error(QString)), this, SLOT(onSearchError(QString)));
            connect(m_plugin, SIGNAL(searchCompleted(SearchResultList)),
                    this, SLOT(onSearchCompleted(SearchResultList)));
            connect(m_plugin, SIGNAL(searchCompleted(SearchResultList, QVariantMap)),
                    this, SLOT(onSearchCompleted(SearchResultList, QVariantMap)));
            connect(m_plugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
                    this, SLOT(onSearchSettingsRequest(QString, QVariantList, QByteArray)));
        }
        else {
            Logger::log("SearchModel::plugin(). No plugin acquired.");
        }
    }
    
    return m_plugin;
}

static void setSearchResultIconFilePath(const SearchResult &result) {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(result.url)) {
        result.iconFilePath = config->iconFilePath();
    }
    else {
        result.iconFilePath = DEFAULT_ICON;
    }
}

void SearchModel::onSearchCompleted(const SearchResultList &results, const QVariantMap &nextParams) {
    if (!results.isEmpty()) {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size() + results.size() - 1);
        
        for (int i = 0; i < results.size(); i++) {
            const SearchResult &result = results.at(i);
            setSearchResultIconFilePath(result);
            m_items << result;
        }
        
        endInsertRows();
        emit countChanged(rowCount());
    }
    
    m_next = nextParams;
    setStatus(Completed);
}

void SearchModel::onSearchError(const QString &errorString) {
    setErrorString(errorString);
    setStatus(Error);
}

void SearchModel::onSearchSettingsRequest(const QString &title, const QVariantList &settings,
                                          const QByteArray &callback) {
    Logger::log("SearchModel::onSearchSettingsRequest()", Logger::MediumVerbosity);
    m_callback = callback;
    setStatus(AwaitingSettingsResponse);
    emit settingsRequest(title, settings);
}
