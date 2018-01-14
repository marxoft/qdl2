/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "definitions.h"
#include "request.h"
#include <QTimer>

UrlRetrievalModel* UrlRetrievalModel::self = 0;

UrlRetrievalModel::UrlRetrievalModel() :
    QAbstractListModel(),
    m_progress(0),
    m_status(Idle)
{
    m_roles[UrlRole] = "url";
    m_roles[PluginIdRole] = "pluginId";
    m_roles[IsDoneRole] = "done";
    m_roles[CountRole] = "count";
    m_roles[ResultsRole] = "results";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

UrlRetrievalModel::~UrlRetrievalModel() {
    self = 0;
}

UrlRetrievalModel* UrlRetrievalModel::instance() {
    return self ? self : self = new UrlRetrievalModel;
}

int UrlRetrievalModel::progress() const {
    return m_progress;
}

void UrlRetrievalModel::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

UrlRetrievalModel::Status UrlRetrievalModel::status() const {
    return m_status;
}

void UrlRetrievalModel::setStatus(UrlRetrievalModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString UrlRetrievalModel::statusString() const {
    return m_statusString;
}

void UrlRetrievalModel::setStatusString(const QString &s) {
    m_statusString = s;
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
        case 1:
            if (m_items.at(index.row()).done) {
                return m_items.at(index.row()).results.size();
            }

            return QVariant();
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
    append(QStringList() << url, pluginId);
}

void UrlRetrievalModel::append(const QStringList &urls, const QString &pluginId) {
    QVariantMap data;
    data["urls"] = urls;
    data["pluginId"] = pluginId;
    Request *request = new Request(this);
    request->post("/urlretrievals/addRetrievals", data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onAppendRequestFinished(Request*)));
}

void UrlRetrievalModel::cancel() {
    Request *request = new Request(this);
    request->get("/urlretrievals/clearRetrievals");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onClearRequestFinished(Request*)));
}

void UrlRetrievalModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        setStatus(Idle);
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void UrlRetrievalModel::reload() {
    Request *retrievalsRequest = new Request(this);
    retrievalsRequest->get("/urlretrievals/getRetrievals");
    connect(retrievalsRequest, SIGNAL(finished(Request*)), this, SLOT(onRetrievalsRequestFinished(Request*)));

    Request *statusRequest = new Request(this);
    statusRequest->get("/urlretrievals/getStatus");
    connect(statusRequest, SIGNAL(finished(Request*)), this, SLOT(onStatusRequestFinished(Request*)));
}

void UrlRetrievalModel::onAppendRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlRetrievalModel::onClearRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlRetrievalModel::onRetrievalsRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        beginResetModel();
        m_items.clear();

        foreach (const QVariant &v, request->result().toList()) {
            const QVariantMap retrieval = v.toMap();
            m_items << UrlRetrieval(retrieval.value("url").toString(), retrieval.value("pluginId").toString(),
                    retrieval.value("results").toStringList(), retrieval.value("done").toBool());
        }

        endResetModel();
        emit countChanged(rowCount());
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlRetrievalModel::onStatusRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        const QVariantMap result = request->result().toMap();
        setProgress(result.value("progress", 0).toInt());
        setStatusString(result.value("statusString").toString());
        setStatus(Status(result.value("status", Idle).toInt()));

        switch (status()) {
        case Idle:
        case Completed:
        case Canceled:
            break;
        default:
            QTimer::singleShot(RELOAD_INTERVAL, this, SLOT(reload()));
            break;
        }
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}
