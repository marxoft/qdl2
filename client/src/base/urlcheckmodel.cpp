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

#include "urlcheckmodel.h"
#include "captchatype.h"
#include "definitions.h"
#include "request.h"
#include "transfermodel.h"
#include <QIcon>
#include <QTimer>

UrlCheckModel* UrlCheckModel::self = 0;

UrlCheckModel::UrlCheckModel() :
    QAbstractListModel(),
    m_captchaType(CaptchaType::Unknown),
    m_captchaTypeString(tr("Unknown")),
    m_captchaTimeout(0),
    m_captchaTimeoutString("--:--"),
    m_progress(0),
    m_requestedSettingsTimeout(0),
    m_requestedSettingsTimeoutString("--:--"),
    m_status(Idle),
    m_waitTime(0),
    m_waitTimeString("--:--")
{
    m_roles[UrlRole] = "url";
    m_roles[CategoryRole] = "category";
    m_roles[CreateSubfolderRole] = "createSubfolder";
    m_roles[PriorityRole] = "priority";
    m_roles[CustomCommandRole] = "customCommand";
    m_roles[CustomCommandOverrideEnabledRole] = "customCommandOverrideEnabled";
    m_roles[StartAutomaticallyRole] = "startAutomatically";
    m_roles[IsCheckedRole] = "checked";
    m_roles[IsOkRole] = "ok";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

UrlCheckModel::~UrlCheckModel() {
    self = 0;
}

UrlCheckModel* UrlCheckModel::instance() {
    return self ? self : self = new UrlCheckModel;
}

int UrlCheckModel::captchaType() const {
    return m_captchaType;
}

void UrlCheckModel::setCaptchaType(int type) {
    m_captchaType = type;
}

QString UrlCheckModel::captchaTypeString() const {
    return m_captchaTypeString;
}

void UrlCheckModel::setCaptchaTypeString(const QString &type) {
    m_captchaTypeString = type;
}

QByteArray UrlCheckModel::captchaData() const {
    return m_captchaData;
}

void UrlCheckModel::setCaptchaData(const QByteArray &data) {
    m_captchaData = data;
}

int UrlCheckModel::captchaTimeout() const {
    return m_captchaTimeout;
}

void UrlCheckModel::setCaptchaTimeout(int timeout) {
    if (timeout != captchaTimeout()) {
        m_captchaTimeout = timeout;
        emit captchaTimeoutChanged(timeout);
    }
}

QString UrlCheckModel::captchaTimeoutString() const {
    return m_captchaTimeoutString;
}

void UrlCheckModel::setCaptchaTimeoutString(const QString &timeout) {
    m_captchaTimeoutString = timeout;
}

int UrlCheckModel::progress() const {
    return m_progress;
}

void UrlCheckModel::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

QVariantList UrlCheckModel::requestedSettings() const {
    return m_requestedSettings;
}

void UrlCheckModel::setRequestedSettings(const QVariantList &settings) {
    m_requestedSettings = settings;
}

int UrlCheckModel::requestedSettingsTimeout() const {
    return m_requestedSettingsTimeout;
}

void UrlCheckModel::setRequestedSettingsTimeout(int timeout) {
    if (timeout != requestedSettingsTimeout()) {
        m_requestedSettingsTimeout = timeout;
        emit requestedSettingsTimeoutChanged(timeout);
    }
}

QString UrlCheckModel::requestedSettingsTimeoutString() const {
    return m_requestedSettingsTimeoutString;
}

void UrlCheckModel::setRequestedSettingsTimeoutString(const QString &timeout) {
    m_requestedSettingsTimeoutString = timeout;
}

QString UrlCheckModel::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void UrlCheckModel::setRequestedSettingsTitle(const QString &title) {
    m_requestedSettingsTitle = title;
}

UrlCheckModel::Status UrlCheckModel::status() const {
    return m_status;
}

void UrlCheckModel::setStatus(UrlCheckModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);

        switch (s) {
        case AwaitingCaptchaResponse:
            emit captchaRequest(captchaType(), captchaData());
            break;
        case AwaitingSettingsResponse:
            emit settingsRequest(requestedSettingsTitle(), requestedSettings());
            break;
        case Completed:
        case Canceled:
            TransferModel::instance()->reload();
            break; 
        default:
            break;
        }
    }
}

QString UrlCheckModel::statusString() const {
    return m_statusString;
}

void UrlCheckModel::setStatusString(const QString &s) {
    m_statusString = s;
}

int UrlCheckModel::waitTime() const {
    return m_waitTime;
}

void UrlCheckModel::setWaitTime(int time) {
    if (time != waitTime()) {
        m_waitTime = time;
        emit waitTimeChanged(time);
    }
}

QString UrlCheckModel::waitTimeString() const {
    return m_waitTimeString;
}

void UrlCheckModel::setWaitTimeString(const QString &time) {
    m_waitTimeString = time;
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> UrlCheckModel::roleNames() const {
    return m_roles;
}
#endif

int UrlCheckModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int UrlCheckModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant UrlCheckModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QVariant UrlCheckModel::data(const QModelIndex &index, int role) const {
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
            if (m_items.at(index.row()).checked) {
                if (m_items.at(index.row()).ok) {
                    return QIcon::fromTheme(OK_ICON);
                }
                
                return QIcon::fromTheme(ERROR_ICON);
            }

            return QVariant();
        default:
            return QVariant();
        }
    case UrlRole:
        return m_items.at(index.row()).url;
    case CategoryRole:
        return m_items.at(index.row()).category;
    case CreateSubfolderRole:
        return m_items.at(index.row()).createSubfolder;
    case PriorityRole:
        return m_items.at(index.row()).priority;
    case CustomCommandRole:
        return m_items.at(index.row()).customCommand;
    case CustomCommandOverrideEnabledRole:
        return m_items.at(index.row()).customCommandOverrideEnabled;
    case StartAutomaticallyRole:
        return m_items.at(index.row()).startAutomatically;
    case IsCheckedRole:
        return m_items.at(index.row()).checked;
    case IsOkRole:
        return m_items.at(index.row()).ok;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> UrlCheckModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant UrlCheckModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap UrlCheckModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList UrlCheckModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
        Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int UrlCheckModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void UrlCheckModel::append(const QString &url, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    append(QStringList() << url, category, createSubfolder, priority, customCommand, overrideGlobalCommand,
            startAutomatically);
}

void UrlCheckModel::append(const QStringList &urls, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    QVariantMap data;
    data["urls"] = urls;
    data["category"] = category;
    data["createSubfolder"] = createSubfolder;
    data["priority"] = priority;
    data["customCommand"] = customCommand;
    data["customCommandOverrideEnabled"] = overrideGlobalCommand;
    data["startAutomatically"] = startAutomatically;
    Request *request = new Request(this);
    request->post("/urlchecks/addChecks", data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onAppendRequestFinished(Request*)));
}

void UrlCheckModel::cancel() {
    Request *request = new Request(this);
    request->get("/urlchecks/clearChecks");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onClearRequestFinished(Request*)));
}

void UrlCheckModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void UrlCheckModel::reload() {
    Request *checksRequest = new Request(this);
    checksRequest->get("/urlchecks/getChecks");
    connect(checksRequest, SIGNAL(finished(Request*)), this, SLOT(onChecksRequestFinished(Request*)));

    Request *statusRequest = new Request(this);
    statusRequest->get("/urlchecks/getStatus");
    connect(statusRequest, SIGNAL(finished(Request*)), this, SLOT(onStatusRequestFinished(Request*)));
}

bool UrlCheckModel::submitCaptchaResponse(const QString &response) {
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    Request *request = new Request(this);
    request->post("/urlchecks/submitCaptchaResponse", response);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onCaptchaRequestFinished(Request*)));
    return true;
}

bool UrlCheckModel::submitSettingsResponse(const QVariantMap &settings) {
    if (status() != AwaitingSettingsResponse) {
        return false;
    }

    Request *request = new Request(this);
    request->post("/urlchecks/submitSettingsResponse", settings);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onSettingsRequestFinished(Request*)));
    return true;
}

void UrlCheckModel::onAppendRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlCheckModel::onCaptchaRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlCheckModel::onChecksRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        beginResetModel();
        m_items.clear();

        foreach (const QVariant &v, request->result().toList()) {
            const QVariantMap check = v.toMap();
            m_items << UrlCheck(check.value("url").toString(), check.value("category").toString(),
                    check.value("createSubfolder", false).toBool(),
                    check.value("priority", TransferItem::NormalPriority).toInt(),
                    check.value("customCommand").toString(),
                    check.value("customCommandOverrideEnabled", false).toBool(),
                    check.value("startAutomatically", false).toBool(),
                    check.value("checked", false).toBool(), check.value("ok", false).toBool());
        }

        endResetModel();
        emit countChanged(rowCount());
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlCheckModel::onClearRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlCheckModel::onSettingsRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void UrlCheckModel::onStatusRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        const QVariantMap result = request->result().toMap();
        setCaptchaType(result.value("captchaType", CaptchaType::Unknown).toInt());
        setCaptchaData(result.value("captchaData").toByteArray());
        setCaptchaTimeoutString(result.value("captchaTimeoutString").toString());
        setCaptchaTimeout(result.value("captchaTimeout", 0).toInt());
        setProgress(result.value("progress", 0).toInt());
        setRequestedSettings(result.value("requestedSettings").toList());
        setRequestedSettingsTimeoutString(result.value("requestedSettingsTimeoutString").toString());
        setRequestedSettingsTimeout(result.value("requestedSettingsTimeout", 0).toInt());
        setRequestedSettingsTitle(result.value("requestedSettingsTitle").toString());
        setWaitTimeString(result.value("waitTimeString").toString());
        setWaitTime(result.value("waitTime", 0).toInt());
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
