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

#include "downloadrequestmodel.h"
#include "captchatype.h"
#include "definitions.h"
#include "request.h"
#include <QIcon>
#include <QTimer>

DownloadRequestModel* DownloadRequestModel::self = 0;

DownloadRequestModel::DownloadRequestModel() :
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
    m_roles[IsCheckedRole] = "checked";
    m_roles[IsOkRole] = "ok";
    m_roles[ResultsStringRole] = "resultsString";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

DownloadRequestModel::~DownloadRequestModel() {
    self = 0;
}

DownloadRequestModel* DownloadRequestModel::instance() {
    return self ? self : self = new DownloadRequestModel;
}

int DownloadRequestModel::captchaType() const {
    return m_captchaType;
}

void DownloadRequestModel::setCaptchaType(int type) {
    m_captchaType = type;
}

QString DownloadRequestModel::captchaTypeString() const {
    return m_captchaTypeString;
}

void DownloadRequestModel::setCaptchaTypeString(const QString &type) {
    m_captchaTypeString = type;
}

QByteArray DownloadRequestModel::captchaData() const {
    return m_captchaData;
}

void DownloadRequestModel::setCaptchaData(const QByteArray &data) {
    m_captchaData = data;
}

int DownloadRequestModel::captchaTimeout() const {
    return m_captchaTimeout;
}

void DownloadRequestModel::setCaptchaTimeout(int timeout) {
    if (timeout != captchaTimeout()) {
        m_captchaTimeout = timeout;
        emit captchaTimeoutChanged(timeout);
    }
}

QString DownloadRequestModel::captchaTimeoutString() const {
    return m_captchaTimeoutString;
}

void DownloadRequestModel::setCaptchaTimeoutString(const QString &timeout) {
    m_captchaTimeoutString = timeout;
}

int DownloadRequestModel::progress() const {
    return m_progress;
}

void DownloadRequestModel::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

QVariantList DownloadRequestModel::requestedSettings() const {
    return m_requestedSettings;
}

void DownloadRequestModel::setRequestedSettings(const QVariantList &settings) {
    m_requestedSettings = settings;
}

int DownloadRequestModel::requestedSettingsTimeout() const {
    return m_requestedSettingsTimeout;
}

void DownloadRequestModel::setRequestedSettingsTimeout(int timeout) {
    if (timeout != requestedSettingsTimeout()) {
        m_requestedSettingsTimeout = timeout;
        emit requestedSettingsTimeoutChanged(timeout);
    }
}

QString DownloadRequestModel::requestedSettingsTimeoutString() const {
    return m_requestedSettingsTimeoutString;
}

void DownloadRequestModel::setRequestedSettingsTimeoutString(const QString &timeout) {
    m_requestedSettingsTimeoutString = timeout;
}

QString DownloadRequestModel::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void DownloadRequestModel::setRequestedSettingsTitle(const QString &title) {
    m_requestedSettingsTitle = title;
}

QString DownloadRequestModel::resultsString() const {
    QString s;

    for (int i = 0; i < m_items.size(); i++) {
        s.append(m_items.at(i).resultsString);
    }

    return s;
}

DownloadRequestModel::Status DownloadRequestModel::status() const {
    return m_status;
}

void DownloadRequestModel::setStatus(DownloadRequestModel::Status s) {
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
        default:
            break;
        }
    }
}

QString DownloadRequestModel::statusString() const {
    return m_statusString;
}

void DownloadRequestModel::setStatusString(const QString &s) {
    m_statusString = s;
}

int DownloadRequestModel::waitTime() const {
    return m_waitTime;
}

void DownloadRequestModel::setWaitTime(int time) {
    if (time != waitTime()) {
        m_waitTime = time;
        emit waitTimeChanged(time);
    }
}

QString DownloadRequestModel::waitTimeString() const {
    return m_waitTimeString;
}

void DownloadRequestModel::setWaitTimeString(const QString &time) {
    m_waitTimeString = time;
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> DownloadRequestModel::roleNames() const {
    return m_roles;
}
#endif

int DownloadRequestModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int DownloadRequestModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant DownloadRequestModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QVariant DownloadRequestModel::data(const QModelIndex &index, int role) const {
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
    case IsCheckedRole:
        return m_items.at(index.row()).checked;
    case IsOkRole:
        return m_items.at(index.row()).ok;
    case ResultsStringRole:
        return m_items.at(index.row()).resultsString;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> DownloadRequestModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant DownloadRequestModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap DownloadRequestModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList DownloadRequestModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
        Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int DownloadRequestModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void DownloadRequestModel::append(const QString &url) {
    append(QStringList() << url);
}

void DownloadRequestModel::append(const QStringList &urls) {
    Request *request = new Request(this);
    request->post("/downloadrequests/addRequests", urls);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onAppendRequestFinished(Request*)));
}

void DownloadRequestModel::cancel() {
    Request *request = new Request(this);
    request->get("/downloadrequests/clearRequests");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onClearRequestFinished(Request*)));
}

void DownloadRequestModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void DownloadRequestModel::reload() {
    Request *requestsRequest = new Request(this);
    requestsRequest->get("/downloadrequests/getRequests");
    connect(requestsRequest, SIGNAL(finished(Request*)), this, SLOT(onRequestsRequestFinished(Request*)));

    Request *statusRequest = new Request(this);
    statusRequest->get("/downloadrequests/getStatus");
    connect(statusRequest, SIGNAL(finished(Request*)), this, SLOT(onStatusRequestFinished(Request*)));
}

bool DownloadRequestModel::submitCaptchaResponse(const QString &response) {
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    Request *request = new Request(this);
    request->post("/downloadrequests/submitCaptchaResponse", response);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onCaptchaRequestFinished(Request*)));
    return true;
}

bool DownloadRequestModel::submitSettingsResponse(const QVariantMap &settings) {
    if (status() != AwaitingSettingsResponse) {
        return false;
    }

    Request *request = new Request(this);
    request->post("/downloadrequests/submitSettingsResponse", settings);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onSettingsRequestFinished(Request*)));
    return true;
}

void DownloadRequestModel::onAppendRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void DownloadRequestModel::onCaptchaRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void DownloadRequestModel::onClearRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void DownloadRequestModel::onRequestsRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        beginResetModel();
        m_items.clear();

        foreach (const QVariant &v, request->result().toList()) {
            const QVariantMap req = v.toMap();
            m_items << DownloadRequest(req.value("url").toString(), req.value("checked", false).toBool(),
                    req.value("ok", false).toBool(), req.value("resultsString").toString());
        }

        endResetModel();
        emit countChanged(rowCount());
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void DownloadRequestModel::onSettingsRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        reload();
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}

void DownloadRequestModel::onStatusRequestFinished(Request *request) {
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
