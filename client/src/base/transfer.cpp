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

#include "transfer.h"
#include "captchatype.h"
#include "definitions.h"
#include "imagecache.h"
#include "request.h"
#include "utils.h"
#include <QTimer>

Transfer::Transfer(QObject *parent) :
    TransferItem(parent),
    m_captchaType(CaptchaType::Unknown),
    m_captchaTimeout(0),
    m_priority(NormalPriority),
    m_bytesTransferred(0),
    m_size(0),
    m_speed(0),
    m_status(Paused),
    m_requestMethod("GET"),
    m_requestedSettingsTimeout(0),
    m_servicePluginIcon(DEFAULT_ICON),
    m_customCommandOverrideEnabled(false),
    m_usePlugins(true),
    m_waitTime(0)
{
}

QVariant Transfer::data(int role) const {
    switch (role) {
    case BytesTransferredRole:
        return bytesTransferred();
    case CaptchaDataRole:
        return captchaData();
    case CaptchaResponseRole:
        return captchaResponse();
    case CaptchaTimeoutRole:
        return captchaTimeout();
    case CaptchaTimeoutStringRole:
        return captchaTimeoutString();
    case CaptchaTypeRole:
        return captchaType();
    case CaptchaTypeStringRole:
        return captchaTypeString();
    case CustomCommandRole:
        return customCommand();
    case CustomCommandOverrideEnabledRole:
        return customCommandOverrideEnabled();
    case DownloadPathRole:
        return downloadPath();
    case ErrorStringRole:
        return errorString();
    case FileNameRole:
    case NameRole:
        return fileName();
    case FilePathRole:
        return filePath();
    case IdRole:
        return id();
    case PluginIconRole:
        return pluginIcon();
    case PluginIconPathRole:
        return pluginIconPath();
    case PluginIdRole:
        return pluginId();
    case PluginNameRole:
        return pluginName();
    case PostDataRole:
        return postData();
    case PriorityRole:
        return priority();
    case PriorityStringRole:
        return priorityString();
    case ProgressRole:
        return progress();
    case ProgressStringRole:
        return progressString();
    case RequestedSettingsRole:
        return requestedSettings();
    case RequestedSettingsTimeoutRole:
        return requestedSettingsTimeout();
    case RequestedSettingsTimeoutStringRole:
        return requestedSettingsTimeoutString();
    case RequestedSettingsTitleRole:
        return requestedSettingsTitle();
    case RequestHeadersRole:
        return requestHeaders();
    case RequestMethodRole:
        return requestMethod();
    case SizeRole:
        return size();
    case SpeedRole:
        return speed();
    case SpeedStringRole:
        return speedString();
    case StatusRole:
        return status();
    case StatusStringRole:
        return statusString();
    case SuffixRole:
        return fileSuffix();
    case UrlRole:
        return url();
    case UsePluginsRole:
        return usePlugins();
    case WaitTimeRole:
        return waitTime();
    case WaitTimeStringRole:
        return waitTimeString();
    default:
        return TransferItem::data(role);
    }
}

bool Transfer::setData(int role, const QVariant &value) {
    switch (role) {
    case CaptchaResponseRole:
        return submitCaptchaResponse(value.toString());
    case CustomCommandRole:
        setCustomCommand(value.toString());
        return true;
    case CustomCommandOverrideEnabledRole:
        setCustomCommandOverrideEnabled(value.toBool());
        return true;
    case DownloadPathRole:
        setDownloadPath(value.toString());
        return true;
    case FileNameRole:
    case NameRole:
        setFileName(value.toString());
        return true;
    case FilePathRole:
        setFilePath(value.toString());
        return true;
    case IdRole:
        setId(value.toString());
        return true;
    case PostDataRole:
        setPostData(value.toByteArray());
        return true;
    case PriorityRole:
        setPriority(TransferItem::Priority(value.toInt()));
        return true;
    case RequestedSettingsRole:
        return submitSettingsResponse(value.toMap());
    case RequestHeadersRole:
        setRequestHeaders(value.toMap());
        return true;
    case RequestMethodRole:
        setRequestMethod(value.toByteArray());
        return true;
    case StatusRole:
        switch (value.toInt()) {
        case Queued:
            return queue();
        case Downloading:
            return start();
        case Paused:
            return pause();
        case Canceled:
            return cancel();
        case CanceledAndDeleted:
            return cancel(true);
        default:
            return TransferItem::setData(role, value);
        }
    case UrlRole:
        setUrl(value.toString());
        return true;
    case UsePluginsRole:
        setUsePlugins(value.toBool());
        return true;
    default:
        return TransferItem::setData(role, value);
    }
}

QMap<int, QVariant> Transfer::itemData() const {
    QMap<int, QVariant> map = TransferItem::itemData();
    map[BytesTransferredRole] = bytesTransferred();
    map[CaptchaDataRole] = captchaData();
    map[CaptchaResponseRole] = captchaResponse();
    map[CaptchaTimeoutRole] = captchaTimeout();
    map[CaptchaTimeoutStringRole] = captchaTimeoutString();
    map[CaptchaTypeRole] = captchaType();
    map[CaptchaTypeStringRole] = captchaTypeString();
    map[CustomCommandRole] = customCommand();
    map[CustomCommandOverrideEnabledRole] = customCommandOverrideEnabled();
    map[DownloadPathRole] = downloadPath();
    map[ErrorStringRole] = errorString();
    map[FileNameRole] = fileName();
    map[FilePathRole] = filePath();
    map[IdRole] = id();
    map[PluginIconRole] = pluginIcon();
    map[PluginIconPathRole] = pluginIconPath();
    map[PluginIdRole] = pluginId();
    map[PluginNameRole] = pluginName();
    map[PostDataRole] = postData();
    map[PriorityRole] = priority();
    map[PriorityStringRole] = priorityString();
    map[ProgressRole] = progress();
    map[ProgressStringRole] = progressString();
    map[RequestedSettingsRole] = requestedSettings();
    map[RequestedSettingsTimeoutRole] = requestedSettingsTimeout();
    map[RequestedSettingsTimeoutStringRole] = requestedSettingsTimeoutString();
    map[RequestedSettingsTitleRole] = requestedSettingsTitle();
    map[RequestHeadersRole] = requestHeaders();
    map[RequestMethodRole] = requestMethod();
    map[SizeRole] = size();
    map[SpeedRole] = speed();
    map[SpeedStringRole] = speedString();
    map[StatusRole] = status();
    map[StatusStringRole] = statusString();
    map[SuffixRole] = fileSuffix();
    map[UrlRole] = url();
    map[UsePluginsRole] = usePlugins();
    map[WaitTimeRole] = waitTime();
    map[WaitTimeStringRole] = waitTimeString();
    return map;
}

QVariantMap Transfer::itemDataWithRoleNames() const {
    QVariantMap map = TransferItem::itemDataWithRoleNames();
    map[roleNames().value(BytesTransferredRole)] = bytesTransferred();
    map[roleNames().value(CaptchaDataRole)] = captchaData();
    map[roleNames().value(CaptchaResponseRole)] = captchaResponse();
    map[roleNames().value(CaptchaTimeoutRole)] = captchaTimeout();
    map[roleNames().value(CaptchaTimeoutStringRole)] = captchaTimeoutString();
    map[roleNames().value(CaptchaTypeRole)] = captchaType();
    map[roleNames().value(CaptchaTypeStringRole)] = captchaTypeString();
    map[roleNames().value(CustomCommandRole)] = customCommand();
    map[roleNames().value(CustomCommandOverrideEnabledRole)] = customCommandOverrideEnabled();
    map[roleNames().value(DownloadPathRole)] = downloadPath();
    map[roleNames().value(ErrorStringRole)] = errorString();
    map[roleNames().value(FileNameRole)] = fileName();
    map[roleNames().value(FilePathRole)] = filePath();
    map[roleNames().value(IdRole)] = id();
    map[roleNames().value(NameRole)] = fileName();
    map[roleNames().value(PluginIconRole)] = pluginIcon();
    map[roleNames().value(PluginIconPathRole)] = pluginIconPath();
    map[roleNames().value(PluginIdRole)] = pluginId();
    map[roleNames().value(PluginNameRole)] = pluginName();
    map[roleNames().value(PostDataRole)] = postData();
    map[roleNames().value(PriorityRole)] = priority();
    map[roleNames().value(PriorityStringRole)] = priorityString();
    map[roleNames().value(ProgressRole)] = progress();
    map[roleNames().value(ProgressStringRole)] = progressString();
    map[roleNames().value(RequestedSettingsRole)] = requestedSettings();
    map[roleNames().value(RequestedSettingsTimeoutRole)] = requestedSettingsTimeout();
    map[roleNames().value(RequestedSettingsTimeoutStringRole)] = requestedSettingsTimeoutString();
    map[roleNames().value(RequestedSettingsTitleRole)] = requestedSettingsTitle();
    map[roleNames().value(RequestHeadersRole)] = requestHeaders();
    map[roleNames().value(RequestMethodRole)] = requestMethod();
    map[roleNames().value(SizeRole)] = size();
    map[roleNames().value(SpeedRole)] = speed();
    map[roleNames().value(SpeedStringRole)] = speedString();
    map[roleNames().value(StatusRole)] = status();
    map[roleNames().value(StatusStringRole)] = statusString();
    map[roleNames().value(SuffixRole)] = fileSuffix();
    map[roleNames().value(UrlRole)] = url();
    map[roleNames().value(UsePluginsRole)] = usePlugins();
    map[roleNames().value(WaitTimeRole)] = waitTime();
    map[roleNames().value(WaitTimeStringRole)] = waitTimeString();
    return map;
}

TransferItem::ItemType Transfer::itemType() const {
    return TransferItem::TransferType;
}

bool Transfer::canStart() const {
    switch (status()) {
    case Null:
    case Paused:
    case Failed:
    case Queued:
        return true;
    default:
        return false;
    }
}

bool Transfer::canPause() const {
    switch (status()) {
    case Null:
    case Paused:
    case Canceling:
    case Canceled:
    case Failed:
    case Completed:
        return false;
    default:
        return true;
    }
}

bool Transfer::canCancel() const {
    switch (status()) {
    case Canceling:
    case Canceled:
    case CanceledAndDeleted:
        return false;
    default:
        return true;
    }
}

QString Transfer::customCommand() const {
    return m_customCommand;
}

void Transfer::setCustomCommand(const QString &c) {
    if (c != customCommand()) {
        m_customCommand = c;
        emit dataChanged(this, CustomCommandRole);
    }
}

bool Transfer::customCommandOverrideEnabled() const {
    return m_customCommandOverrideEnabled;
}

void Transfer::setCustomCommandOverrideEnabled(bool enabled) {
    if (enabled != customCommandOverrideEnabled()) {
        m_customCommandOverrideEnabled = enabled;
        emit dataChanged(this, CustomCommandOverrideEnabledRole);
    }
}

QString Transfer::downloadPath() const {
    return m_downloadPath;
}

void Transfer::setDownloadPath(const QString &p) {
    if (p != downloadPath()) {
        m_downloadPath = (p.endsWith("/") ? p : p + "/");
        emit dataChanged(this, DownloadPathRole);
        emit dataChanged(this, FilePathRole);
    }
}

QString Transfer::fileName() const {
    return m_fileName;
}

QString Transfer::filePath() const {
    return downloadPath() + fileName();
}

void Transfer::setFileName(const QString &f) {
    if (f != fileName()) {
        m_fileName = f;
        emit dataChanged(this, FileNameRole);
        emit dataChanged(this, FilePathRole);
    }
}

void Transfer::setFilePath(const QString &fp) {
    const int slash = fp.lastIndexOf("/");

    if (slash != -1) {
        setDownloadPath(fp.left(slash + 1));
        setFileName(fp.mid(slash + 1));
    }
}

QString Transfer::fileSuffix() const {
    const int dot = m_fileName.lastIndexOf(".");
    return dot == -1 ? QString() : m_fileName.mid(dot + 1);
}

int Transfer::captchaType() const {
    return m_captchaType;
}

void Transfer::setCaptchaType(int t) {
    if (t != captchaType()) {
        m_captchaType = t;
        emit dataChanged(this, CaptchaTypeRole);
    }
}

QString Transfer::captchaTypeString() const {
    switch (captchaType()) {
    case CaptchaType::Image:
        return tr("Image");
    case CaptchaType::NoCaptcha:
        return tr("NoCaptcha");
    default: 
        return tr("Unknown");
    }
}

QByteArray Transfer::captchaData() const {
    return m_captchaData;
}

void Transfer::setCaptchaData(const QByteArray &d) {
    if (d != captchaData()) {
        m_captchaData = d;
        emit dataChanged(this, CaptchaDataRole);
    }
}

QString Transfer::captchaResponse() const {
    return QString();
}

int Transfer::captchaTimeout() const {
    return m_captchaTimeout;
}

void Transfer::setCaptchaTimeout(int t) {
    if (t != captchaTimeout()) {
        m_captchaTimeout = t;
        emit dataChanged(this, CaptchaTimeoutRole);
    }
}

QString Transfer::captchaTimeoutString() const {
    return Utils::formatMSecs(captchaTimeout());
}

QString Transfer::id() const {
    return m_id;
}

void Transfer::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit dataChanged(this, IdRole);
    }
}

QIcon Transfer::pluginIcon() const {
    const QPixmap icon = ImageCache::instance()->image(pluginIconPath());

    if (icon.isNull()) {
        connect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(imageReady(QString)));
        return QPixmap(DEFAULT_ICON);
    }

    return icon;
}

QString Transfer::pluginIconPath() const {
    return m_servicePluginIcon;
}

void Transfer::setPluginIconPath(const QString &p) {
    if (p != pluginIconPath()) {
        m_servicePluginIcon = p;
        emit dataChanged(this, PluginIconPathRole);
        emit dataChanged(this, PluginIconRole);
    }
}

QString Transfer::pluginId() const {
    return m_servicePluginId;
}

void Transfer::setPluginId(const QString &i) {
    if (i != pluginId()) {
        m_servicePluginId = i;
        emit dataChanged(this, PluginIdRole);
    }
}

QString Transfer::pluginName() const {
    return m_servicePluginName;
}

void Transfer::setPluginName(const QString &n) {
    if (n != pluginName()) {
        m_servicePluginName = n;
        emit dataChanged(this, PluginNameRole);
    }
}

bool Transfer::usePlugins() const {
    return m_usePlugins;
}

void Transfer::setUsePlugins(bool enabled) {
    if (enabled != usePlugins()) {
        m_usePlugins = enabled;
        emit dataChanged(this, UsePluginsRole);
    }
}

QString Transfer::postData() const {
    return m_postData;
}

void Transfer::setPostData(const QString &data) {
    if (data != postData()) {
        m_postData = data;
        emit dataChanged(this, PostDataRole);
    }
}

TransferItem::Priority Transfer::priority() const {
    return m_priority;
}

void Transfer::setPriority(TransferItem::Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit dataChanged(this, PriorityRole);
    }
}

QString Transfer::priorityString() const {
    return TransferItem::priorityString(priority());
}

int Transfer::progress() const {
    return size() > 0 ? qMin(100, int(bytesTransferred() * 100 / size())) : 0;
}

QString Transfer::progressString() const {
    if (size() > 0) {
        return tr("%1 of %2 (%3%)").arg(Utils::formatBytes(bytesTransferred())).arg(Utils::formatBytes(size()))
                                        .arg(progress());
    }

    return tr("%1 of Unknown").arg(Utils::formatBytes(bytesTransferred()));
}

qint64 Transfer::bytesTransferred() const {
    return m_bytesTransferred;
}

void Transfer::setBytesTransferred(qint64 b) {
    if (b != bytesTransferred()) {
        m_bytesTransferred = b;
        emit dataChanged(this, BytesTransferredRole);
    }
}

qint64 Transfer::size() const {
    return m_size;
}

void Transfer::setSize(qint64 s) {
    if (s != size()) {
        m_size = s;
        emit dataChanged(this, SizeRole);
    }
}

int Transfer::speed() const {
    return status() == Downloading ? m_speed : 0;
}

void Transfer::setSpeed(int s) {
    if (s != speed()) {
        m_speed = s;
        emit dataChanged(this, SpeedRole);
    }
}

QString Transfer::speedString() const {
    return QString("%1/s").arg(Utils::formatBytes(speed()));
}

QVariantList Transfer::requestedSettings() const {
    return m_requestedSettings;
}

void Transfer::setRequestedSettings(const QVariantList &s) {
    m_requestedSettings = s;
    emit dataChanged(this, RequestedSettingsRole);
}

int Transfer::requestedSettingsTimeout() const {
    return m_requestedSettingsTimeout;
}

void Transfer::setRequestedSettingsTimeout(int t) {
    if (t != requestedSettingsTimeout()) {
        m_requestedSettingsTimeout = t;
        emit dataChanged(this, RequestedSettingsTimeoutRole);
    }
}

QString Transfer::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString Transfer::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void Transfer::setRequestedSettingsTitle(const QString &t) {
    if (t != requestedSettingsTitle()) {
        m_requestedSettingsTitle = t;
        emit dataChanged(this, RequestedSettingsTitleRole);
    }
}

QVariantMap Transfer::requestHeaders() const {
    return m_requestHeaders;
}

void Transfer::setRequestHeaders(const QVariantMap &headers) {
    m_requestHeaders = headers;
    emit dataChanged(this, RequestHeadersRole);
}

QString Transfer::requestMethod() const {
    return m_requestMethod;
}

void Transfer::setRequestMethod(const QString &method) {
    if (method != requestMethod()) {
        if (method.isEmpty()) {
            m_requestMethod = QString("GET");
        }
        else {
            m_requestMethod = method.toUpper();
        }
        
        emit dataChanged(this, RequestMethodRole);
    }
}

TransferItem::Status Transfer::status() const {
    return m_status;
}

void Transfer::setStatus(TransferItem::Status s) {
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

QString Transfer::statusString() const {
    switch (status()) {
    case Failed:
        return QString("%1: %2").arg(TransferItem::statusString(Failed)).arg(errorString());
    case WaitingInactive:
        return QString("%1: %2").arg(TransferItem::statusString(WaitingInactive)).arg(waitTimeString());
    case WaitingActive:
        return QString("%1: %2").arg(TransferItem::statusString(WaitingActive)).arg(waitTimeString());
    case AwaitingCaptchaResponse:
        return QString("%1: %2").arg(TransferItem::statusString(AwaitingCaptchaResponse)).arg(captchaTimeoutString());
    case AwaitingSettingsResponse:
        return QString("%1: %2").arg(TransferItem::statusString(status())).arg(requestedSettingsTimeoutString());
    default:
        return TransferItem::statusString(status());
    }
}

QString Transfer::errorString() const {
    return m_errorString;
}

void Transfer::setErrorString(const QString &e) {
    if (e != errorString()) {
        m_errorString = e;
        emit dataChanged(this, ErrorStringRole);
    }
}

QString Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit dataChanged(this, UrlRole);
    }
}

int Transfer::waitTime() const {
    return m_waitTime;
}

void Transfer::setWaitTime(int t) {
    if (t != waitTime()) {
        m_waitTime = t;
        emit dataChanged(this, WaitTimeRole);
    }
}

QString Transfer::waitTimeString() const {
    return Utils::formatMSecs(waitTime());
}

bool Transfer::queue() {
    if (canStart()) {
        QVariantMap params;
        params["id"] = id();
        Request *request = new Request(this);
        request->get("/transfers/startTransfer", params);
        connect(request, SIGNAL(finished(Request*)), this, SLOT(queueRequestFinished(Request*)));
        return true;
    }

    return false;
}

bool Transfer::start() {
    if (canStart()) {
        QVariantMap params;
        params["id"] = id();
        Request *request = new Request(this);
        request->get("/transfers/startTransfer", params);
        connect(request, SIGNAL(finished(Request*)), this, SLOT(startRequestFinished(Request*)));
        return true;
    }

    return false;
}

bool Transfer::pause() {
    switch (status()) {
    case Null:
    case Paused:
    case Canceling:
    case Canceled:
    case Failed:
    case Completed:
        return false;
    default:
        break;
    }

    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/pauseTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(pauseRequestFinished(Request*)));
    return true;
}

bool Transfer::cancel(bool deleteFiles) {
    switch (status()) {
    case Canceling:
    case Canceled:
    case CanceledAndDeleted:
        return false;
    default:
        break;
    }

    QVariantMap params;
    params["id"] = id();
    params["deleteFiles"] = deleteFiles;
    Request *request = new Request(this);
    request->get("/transfers/removeTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(cancelRequestFinished(Request*)));
    return true;
}

bool Transfer::reload() {
    QVariantMap params;
    params["id"] = id();
    Request *request = new Request(this);
    request->get("/transfers/getTransfer", params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(reloadRequestFinished(Request*)));
    return true;
}

void Transfer::restore(const QVariantMap &data) {
    setCustomCommand(data.value("customCommand").toString());
    setCustomCommandOverrideEnabled(data.value("customCommandOverrideEnabled").toBool());
    setDownloadPath(data.value("downloadPath").toString());
    setFileName(data.value("fileName").toString());
    setCaptchaType(data.value("captchaType").toInt());
    setCaptchaData(data.value("captchaData").toByteArray());
    setCaptchaTimeout(data.value("captchaTimeout").toInt());
    setId(data.value("id").toString());
    setPluginIconPath(data.value("pluginIconPath").toString());
    setPluginId(data.value("pluginId").toString());
    setPluginName(data.value("pluginName").toString());
    setUsePlugins(data.value("usePlugins", true).toBool());
    setPostData(data.value("postData").toByteArray());
    setPriority(TransferItem::Priority(data.value("priority").toInt()));
    setRequestHeaders(data.value("requestHeaders").toMap());
    setRequestMethod(data.value("requestMethod").toByteArray());
    setRequestedSettings(data.value("requestedSettings").toList());
    setRequestedSettingsTimeout(data.value("requestedSettingsTimeout").toInt());
    setRequestedSettingsTitle(data.value("requestedSettingsTitle").toString());
    setBytesTransferred(data.value("bytesTransferred").toInt());
    setSize(data.value("size").toLongLong());
    setSpeed(data.value("speed").toInt());
    setUrl(data.value("url").toString());
    setWaitTime(data.value("waitTime").toInt());
    setErrorString(data.value("errorString").toString());
    setStatus(Status(data.value("status").toInt()));
}

void Transfer::save() {
    QVariantMap params;
    params["id"] = id();
    QVariantMap data;
    data["customCommand"] = customCommand();
    data["customCommandOverrideEnabled"] = customCommandOverrideEnabled();
    data["priority"] = TransferItem::Priority(priority());
    Request *request = new Request(this);
    request->put("/transfers/setTransferProperties", params, data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(saveRequestFinished(Request*)));
}

bool Transfer::submitCaptchaResponse(const QString &response) {
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    QVariantMap params;
    params["id"] = id();
    QVariantMap data;
    data["captchaResponse"] = response;
    Request *request = new Request(this);
    request->put("/transfers/setTransferProperties", params, data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(saveRequestFinished(Request*)));
    return true;
}

bool Transfer::submitSettingsResponse(const QVariantMap &settings) {
    if (status() != AwaitingSettingsResponse) {
        return false;
    }

    QVariantMap params;
    params["id"] = id();
    QVariantMap data;
    data["requestedSettings"] = settings;
    Request *request = new Request(this);
    request->put("/transfers/setTransferProperties", params, data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(saveRequestFinished(Request*)));
    return true;
}

void Transfer::imageReady(const QString &path) {
    if (path == pluginIconPath()) {
        emit dataChanged(this, PluginIconRole);
        disconnect(ImageCache::instance(), SIGNAL(imageReady(QString)), this, SLOT(imageReady(QString)));
    }
}

void Transfer::queueRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Transfer::startRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Transfer::pauseRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Transfer::cancelRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        setStatus(Canceled);
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}

void Transfer::reloadRequestFinished(Request *request) {
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

void Transfer::saveRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        restore(request->result().toMap());
    }
    else if (request->status() == Request::Error) {
        emit error(this, request->errorString());
    }

    request->deleteLater();
}
