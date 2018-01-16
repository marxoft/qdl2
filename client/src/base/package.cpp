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

#include "package.h"
#include "definitions.h"
#include "request.h"
#include "transfer.h"
#include <QTimer>

Package::Package(QObject *parent) :
    TransferItem(parent),
    m_createSubfolder(false),
    m_priority(NormalPriority),
    m_status(Null)
{
}

QVariant Package::data(int role) const {
    switch (role) {
    case CategoryRole:
        return category();
    case CreateSubfolderRole:
        return createSubfolder();
    case ErrorStringRole:
        return errorString();
    case IdRole:
        return id();
    case NameRole:
        return name();
    case PriorityRole:
        return priority();
    case PriorityStringRole:
        return priorityString();
    case ProgressRole:
        return progress();
    case ProgressStringRole:
        return progressString();
    case StatusRole:
        return status();
    case StatusStringRole:
        return statusString();
    case SuffixRole:
        return suffix();
    default:
        return TransferItem::data(role);
    }
}

bool Package::setData(int role, const QVariant &value) {
    switch (role) {
    case CategoryRole:
        setCategory(value.toString());
        return true;
    case CreateSubfolderRole:
        setCreateSubfolder(value.toBool());
        return true;
    case IdRole:
        setId(value.toString());
        return true;
    case NameRole:
        setName(value.toString());
        return true;
    case PriorityRole:
        setPriority(TransferItem::Priority(value.toInt()));
        return true;
    case StatusRole:
        switch (value.toInt()) {
        case Queued:
            queue();
            return true;
        case Downloading:
            start();
            return true;
        case Paused:
            pause();
            return true;
        case Canceled:
            cancel();
            return true;
        case CanceledAndDeleted:
            cancel(true);
            return true;
        default:
            return TransferItem::setData(role, value);
        }
    case SuffixRole:
        setSuffix(value.toString());
        return true;
    default:
        return TransferItem::setData(role, value);
    }
}

QMap<int, QVariant> Package::itemData() const {
    QMap<int, QVariant> map = TransferItem::itemData();
    map[CategoryRole] = category();
    map[CreateSubfolderRole] = createSubfolder();
    map[ErrorStringRole] = errorString();
    map[IdRole] = id();
    map[NameRole] = name();
    map[PriorityRole] = priority();
    map[PriorityStringRole] = priorityString();
    map[ProgressRole] = progress();
    map[ProgressStringRole] = progressString();
    map[StatusRole] = status();
    map[StatusStringRole] = statusString();
    map[SuffixRole] = suffix();
    return map;
}

QVariantMap Package::itemDataWithRoleNames() const {
    QVariantMap map = TransferItem::itemDataWithRoleNames();
    map[roleNames().value(CategoryRole)] = category();
    map[roleNames().value(CreateSubfolderRole)] = createSubfolder();
    map[roleNames().value(ErrorStringRole)] = errorString();
    map[roleNames().value(IdRole)] = id();
    map[roleNames().value(NameRole)] = name();
    map[roleNames().value(PriorityRole)] = priority();
    map[roleNames().value(PriorityStringRole)] = priorityString();
    map[roleNames().value(ProgressRole)] = progress();
    map[roleNames().value(ProgressStringRole)] = progressString();
    map[roleNames().value(StatusRole)] = status();
    map[roleNames().value(StatusStringRole)] = statusString();
    map[roleNames().value(SuffixRole)] = suffix();
    return map;
}

TransferItem::ItemType Package::itemType() const {
    return TransferItem::PackageType;
}

bool Package::canStart() const {
    return (rowCount() > 0) && (status() == Null);
}

bool Package::canPause() const {
    return (rowCount() > 0) && (status() == Null);
}

bool Package::canCancel() const {
    return (rowCount() == 0) || (status() == Null) || (status() == Failed);
}

QString Package::category() const {
    return m_category;
}

void Package::setCategory(const QString &c) {
    if (c != category()) {
        m_category = c;
        emit dataChanged(this, CategoryRole);
    }
}

bool Package::createSubfolder() const {
    return m_createSubfolder;
}

void Package::setCreateSubfolder(bool enabled) {
    if (enabled != createSubfolder()) {
        m_createSubfolder = enabled;
        emit dataChanged(this, CreateSubfolderRole);
    }
}

QString Package::id() const {
    return m_id;
}

void Package::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit dataChanged(this, IdRole);
    }
}

QString Package::name() const {
    return m_name;
}

void Package::setName(const QString &n) {
    if (n != name()) {
        m_name = n;
        emit dataChanged(this, NameRole);
    }
}

QString Package::suffix() const {
    return m_suffix;
}

void Package::setSuffix(const QString &s) {
    if (s != suffix()) {
        m_suffix = s;
        emit dataChanged(this, SuffixRole);
    }
}

TransferItem::Priority Package::priority() const {
    return m_priority;
}

void Package::setPriority(TransferItem::Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit dataChanged(this, PriorityRole);
    }

    foreach (TransferItem *child, m_childItems) {
        child->setData(PriorityRole, p);
    }
}

QString Package::priorityString() const {
    return TransferItem::priorityString(priority());
}

int Package::progress() const {
    if (rowCount() == 0) {
        return 0;
    }
    
    int completed = 0;

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            ++completed;
        }
    }

    if (completed > 0) {
        return completed * 100 / rowCount();
    }

    return 0;
}

QString Package::progressString() const {
    if (rowCount() == 0) {
        return QString();
    }

    int completed = 0;

    foreach (const TransferItem *child, m_childItems) {
        if (child->data(StatusRole) == Completed) {
            ++completed;
        }
    }

    return tr("%1 of %2 (%3%)").arg(completed).arg(rowCount()).arg(completed > 0 ? completed * 100 / rowCount() : 0);
}

TransferItem::Status Package::status() const {
    return m_status;
}

void Package::setStatus(TransferItem::Status s) {
    if (s != status()) {
        m_status = s;

        switch (s) {
        case Canceled:
        case CanceledAndDeleted:
        case Failed:
        case Completed:
            emit finished(this);
            break;
        default:
            break;
        }
        
        emit statusChanged(this, s);
        emit dataChanged(this, StatusRole);
    }
}

QString Package::statusString() const {
    switch (status()) {
    case Failed:
        return QString("%1: %2").arg(TransferItem::statusString(Failed)).arg(errorString());
    default:
        return TransferItem::statusString(status());
    }
}

QString Package::errorString() const {
    return m_errorString;
}

void Package::setErrorString(const QString &e) {
    m_errorString = e;
}

bool Package::queue() {
    if (!canStart()) {
        return false;
    }

    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/startTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(queueRequestFinished(Request*)));
    return true;
}

bool Package::start() {
    if (!canStart()) {
        return false;
    }

    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/startTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(startRequestFinished(Request*)));
    return true;
}

bool Package::pause() {
    if (!canPause()) {
        return false;
    }

    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/pauseTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(pauseRequestFinished(Request*)));
    return true;
}

bool Package::cancel(bool deleteFiles) {
    if (!canCancel()) {
        return false;
    }

    if (rowCount() == 0) {
        setStatus(deleteFiles ? CanceledAndDeleted : Canceled);
        return true;
    }

    setStatus(Canceling);
    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/cancelTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(cancelRequestFinished(Request*)));
    return true;
}

bool Package::reload() {
    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/getTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(reloadRequestFinished(Request*)));
    return true;
}

void Package::restore(const QVariantMap &data) {
    setCategory(data.value("category").toString());
    setCreateSubfolder(data.value("createSubfolder").toBool());
    setId(data.value("id").toString());
    setName(data.value("name").toString());
    setSuffix(data.value("suffix").toString());
    setPriority(Priority(data.value("priority").toInt()));
    setErrorString(data.value("errorString").toString());
    setStatus(Status(data.value("status").toInt()));

    if (data.contains("children")) {
        const QVariantList children = data.value("children").toList();
        const int diff = children.size() - rowCount();

        if (diff > 0) {
            for (int i = 0; i < diff; i++) {
                appendRow(new Transfer(this));
            }
        }
        else if (diff < 0) {
            for (int i = rowCount() - 1; i >= children.size(); i--) {
                removeRow(i);
            }
        }

        for (int i = 0; i < children.size(); i++) {
            if (TransferItem *item = childItem(i)) {
                item->restore(children.at(i).toMap());
            }
        }
    }
}

void Package::save() {
    QVariantMap params;
    params["id"] = id();
    QVariantMap data;
    data["category"] = category();
    data["createSubfolder"] = createSubfolder();
    data["priority"] = priority();
    Request *request = new Request(this);
    request->put("/transfers/setTransferProperties", params, data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(saveRequestFinished(Request*)));
}

void Package::childItemFinished(TransferItem *item) {
    const TransferItem::Status status = TransferItem::Status(item->data(StatusRole).toInt());
    
    if (status == Completed) {
        emit dataChanged(this, ProgressRole);
        
        foreach (const TransferItem *child, m_childItems) {
            if (child->data(StatusRole) != Completed) {
                return;
            }
        }

        setStatus(Completed);
    }
    else if ((status == Canceled) || (status == CanceledAndDeleted)) {

        foreach (const TransferItem *child, m_childItems) {
            switch (child->data(StatusRole).toInt()) {
            case Canceled:
            case CanceledAndDeleted:
                break;
            default:
                return;
            }
        }
        
        setStatus(status);
    }
}

void Package::queueRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Package::startRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Package::pauseRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Package::cancelRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        setStatus(Canceled);
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Package::reloadRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
        emit loaded(this);

        if (autoReloadEnabled()) {
            QTimer::singleShot(RELOAD_INTERVAL, this, SLOT(reload()));
        }
    }
    else if (request->status() == Request::Error) {
        if (request->statusCode() == 404) {
            setStatus(Completed);
        }
        else {
            emit error(this, request->errorString());
        }
    }

    request->deleteLater();
}

void Package::saveRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}
