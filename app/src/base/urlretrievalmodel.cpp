/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "urlretrievalmodel.h"
#include "logger.h"
#include "urlretriever.h"
#include <QIcon>

UrlRetrievalModel* UrlRetrievalModel::self = 0;

UrlRetrievalModel::UrlRetrievalModel() :
    QAbstractListModel(),
    m_retriever(new UrlRetriever(this)),
    m_status(Idle),
    m_index(-1)
{
    m_roles[UrlRole] = "url";
    m_roles[PluginIdRole] = "pluginId";
    m_roles[IsDoneRole] = "done";
    m_roles[CountRole] = "count";
    m_roles[ResultsRole] = "results";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
    connect(m_retriever, SIGNAL(finished(UrlRetriever*)), this, SLOT(onRetrieverFinished(UrlRetriever*)));
}

UrlRetrievalModel::~UrlRetrievalModel() {
    self = 0;
}

UrlRetrievalModel* UrlRetrievalModel::instance() {
    return self ? self : self = new UrlRetrievalModel;
}

int UrlRetrievalModel::progress() const {
    return (!m_items.isEmpty()) && (m_index > 0) ? m_index * 100 / m_items.size() : 0;
}

UrlRetrievalModel::Status UrlRetrievalModel::status() const {
    return m_status;
}

void UrlRetrievalModel::setStatus(UrlRetrievalModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);

        if (s != Active) {
            m_index = -1;
        }
    }
}

QString UrlRetrievalModel::statusString() const {
    switch (status()) {
    case Active:
        return tr("Retrieving URLs");
    case Completed:
        return tr("Completed: %1 results found").arg(results().size());
    case Canceled:
        return tr("Canceled");
    default:
        return QString();
    }
}

QStringList UrlRetrievalModel::results() const {
    QStringList urls;

    for (int i = 0; i < m_items.size(); i++) {
        urls << m_items.at(i).results;
    }

    return urls;
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> UrlRetrievalModel::roleNames() const {
    return m_roles;
}
#endif

int UrlRetrievalModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int UrlRetrievalModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant UrlRetrievalModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("URL");
    case 1:
        return tr("Ok?");
    default:
        return QVariant();
    }
}

QVariant UrlRetrievalModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_items.at(index.row()).url;
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case 1:
            if (m_items.at(index.row()).done) {
                if (!m_items.at(index.row()).results.isEmpty()) {
                    return QIcon::fromTheme("go-next");
                }
                
                return QIcon::fromTheme("dialog-error");
            }

            return QIcon::fromTheme("dialog-question");
        default:
            return QVariant();
        }
    case UrlRole:
        return m_items.at(index.row()).url;
    case PluginIdRole:
        return m_items.at(index.row()).pluginId;
    case IsDoneRole:
        return m_items.at(index.row()).done;
    case CountRole:
        return m_items.at(index.row()).results.size();
    case ResultsRole:
        return m_items.at(index.row()).results;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> UrlRetrievalModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant UrlRetrievalModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap UrlRetrievalModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList UrlRetrievalModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                      Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int UrlRetrievalModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void UrlRetrievalModel::append(const QString &url, const QString &pluginId) {
    Logger::log(QString("UrlRetrievalModel::append(): URL: %1, pluginId: %2").arg(url).arg(pluginId),
                Logger::LowVerbosity);
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << UrlRetrieval(url, pluginId);
    endInsertRows();
    emit countChanged(rowCount());
    emit progressChanged(progress());

    if (status() != Active) {
        next();
    }
}

void UrlRetrievalModel::append(const QStringList &urls, const QString &pluginId) {
    foreach (const QString &url, urls) {
        append(url, pluginId);
    }
}

bool UrlRetrievalModel::remove(int row) {
    if ((row > m_index) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
        emit countChanged(rowCount());
        emit progressChanged(progress());
        return true;
    }
    
    return false;
}

void UrlRetrievalModel::cancel() {
    m_retriever->cancel();
    setStatus(Canceled);
}

void UrlRetrievalModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        setStatus(Idle);
        m_index = -1;
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void UrlRetrievalModel::next() {
    m_index++;
    emit progressChanged(progress());

    if (m_index >= m_items.size()) {
        setStatus(Completed);
        return;
    }

    setStatus(Active);
    const QModelIndex idx = index(m_index, 0);
    m_retriever->start(data(idx, UrlRole).toString(), data(idx, PluginIdRole).toString());
}

void UrlRetrievalModel::onRetrieverFinished(UrlRetriever *retriever) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlRetrievalModel::onRetrieverFinished(): %1. %2 URLs found")
                       .arg(retriever->url()).arg(retriever->results().size()), Logger::LowVerbosity);
    m_items[m_index].done = true;
    m_items[m_index].results = retriever->results();
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    next();
}
