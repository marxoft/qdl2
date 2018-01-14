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

#include "transfer.h"
#include "captchatype.h"
#include "definitions.h"
#include "logger.h"
#include "servicepluginconfig.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "utils.h"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

const QRegExp Transfer::CONTENT_DISPOSITION_REGEXP("(filename=|filename\\*=UTF-8''|filename\\*= UTF-8'')([^;]+)");

Transfer::Transfer(QObject *parent) :
    TransferItem(parent),
    m_requester(0),
    m_nam(0),
    m_reply(0),
    m_file(0),
    m_priority(NormalPriority),
    m_lastBytesTransferred(0),
    m_size(0),
    m_speed(0),
    m_status(Paused),
    m_requestMethod("GET"),
    m_servicePluginIcon(DEFAULT_ICON),
    m_customCommandOverrideEnabled(false),
    m_usePlugins(true),
    m_metadataSet(false),
    m_redirects(0)
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

        if (!fileName().isEmpty()) {
            if (!m_file) {
                m_file = new QFile(this);
            }

            if (!m_file->isOpen()) {
                m_file->setFileName(filePath());
                emit dataChanged(this, BytesTransferredRole);
            }
        }
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

        if (!downloadPath().isEmpty()) {
            if (!m_file) {
                m_file = new QFile(this);
            }

            if (!m_file->isOpen()) {
                m_file->setFileName(filePath());
                emit dataChanged(this, BytesTransferredRole);
            }
        }
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
    return m_requester ? m_requester->captchaType() : CaptchaType::Unknown;
}

QString Transfer::captchaTypeString() const {
    return m_requester ? m_requester->captchaTypeString() : tr("Unknown");
}

QByteArray Transfer::captchaData() const {
    return m_requester ? m_requester->captchaData() : QByteArray();
}

QString Transfer::captchaResponse() const {
    return QString();
}

int Transfer::captchaTimeout() const {
    return m_requester ? m_requester->captchaTimeout() : 0;
}

QString Transfer::captchaTimeoutString() const {
    return m_requester ? m_requester->captchaTimeoutString() : QString();
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

QString Transfer::pluginIconPath() const {
    return m_servicePluginIcon;
}

void Transfer::setPluginIconPath(const QString &p) {
    if (p != pluginIconPath()) {
        m_servicePluginIcon = p;
        emit dataChanged(this, PluginIconPathRole);
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

void Transfer::updatePluginInfo() {
    if (usePlugins()) {
        if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(url())) {
            const QString path = config->iconFilePath();
            
            if (!path.isEmpty()) {
                setPluginIconPath(path);
            }
            else {
                setPluginIconPath(DEFAULT_ICON);
            }
            
            setPluginId(config->id());
            setPluginName(config->displayName());
            return;
        }
    }
    
    setPluginIconPath(DEFAULT_ICON);
    setPluginId(QString());
    setPluginName(QString());
}

bool Transfer::usePlugins() const {
    return m_usePlugins;
}

void Transfer::setUsePlugins(bool enabled) {
    if (enabled != usePlugins()) {
        m_usePlugins = enabled;
        emit dataChanged(this, UsePluginsRole);
        updatePluginInfo();
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
    return m_file ? m_file->size() : 0;
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
    return m_requester ? m_requester->requestedSettings() : QVariantList();
}

int Transfer::requestedSettingsTimeout() const {
    return m_requester ? m_requester->requestedSettingsTimeout() : 0;
}

QString Transfer::requestedSettingsTimeoutString() const {
    return m_requester ? m_requester->requestedSettingsTimeoutString() : QString();
}

QString Transfer::requestedSettingsTitle() const {
    return m_requester ? m_requester->requestedSettingsTitle() : QString();
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
        Logger::log(QString("Transfer::setStatus(): ID: %1, Status: %2").arg(id()).arg(statusString()));

        switch (s) {
        case Canceled:
        case CanceledAndDeleted:
        case Failed:
            emit finished(this);
            break;
        case Completed:
            cleanup();
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
    m_errorString = e;
}

QString Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit dataChanged(this, UrlRole);
        updatePluginInfo();
    }
}

int Transfer::waitTime() const {
    return m_requester ? m_requester->waitTime() : 0;
}

QString Transfer::waitTimeString() const {
    return m_requester ? m_requester->waitTimeString() : QString();
}

bool Transfer::queue() {
    if (canStart()) {
        setStatus(Queued);
        return true;
    }

    return false;
}

bool Transfer::start() {
    if (canStart()) {
        if (!usePlugins()) {
            startDownload();
        }
        else {
            initRequester();
            setStatus(Connecting);
            m_requester->getDownloadRequest(url());
        }

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
    case WaitingInactive:
    case Connecting:
    case WaitingActive:
    case RetrievingCaptchaChallenge:
    case RetrievingCaptchaResponse:
    case ReportingCaptchaResponse:
    case SubmittingCaptchaResponse:
    case SubmittingSettingsResponse:
        if (m_requester) {
            m_requester->cancel();
        }

        break;
    case Downloading:
        if (m_reply) {
            if (m_reply->isRunning()) {
                m_reply->abort();
                return true;
            }
        }

        break;
    default:
        break;
    }

    setStatus(Paused);
    return true;
}

bool Transfer::cancel(bool deleteFiles) {
    m_deleteFiles = deleteFiles;
    
    switch (status()) {
    case Canceling:
    case Canceled:
    case CanceledAndDeleted:
        return false;
    case WaitingInactive:
    case WaitingActive:
    case RetrievingCaptchaChallenge:
    case RetrievingCaptchaResponse:
    case ReportingCaptchaResponse:
    case SubmittingCaptchaResponse:
    case SubmittingSettingsResponse:
        if (m_requester) {
            m_requester->cancel();
        }

        break;
    case Downloading:
        if (m_reply) {
            if (m_reply->isRunning()) {
                setStatus(Canceling);
                m_reply->abort();
                return true;
            }
        }

        break;
    default:
        break;
    }

    if (deleteFiles) {
        deleteFile();
        setStatus(CanceledAndDeleted);
    }
    else {
        setStatus(Canceled);
    }

    return true;
}

void Transfer::restore(const QSettings &settings) {
    setCustomCommand(settings.value("customCommand").toString());
    setCustomCommandOverrideEnabled(settings.value("customCommandOverrideEnabled", false).toBool());
    setDownloadPath(settings.value("downloadPath").toString());
    setErrorString(settings.value("errorString").toString());
    setFileName(settings.value("fileName").toString());
    setId(settings.value("id").toString());
    setPostData(settings.value("postData").toByteArray());
    setPriority(TransferItem::Priority(settings.value("priority", NormalPriority).toInt()));
    setRequestHeaders(settings.value("requestHeaders").toMap());
    setRequestMethod(settings.value("requestMethod").toByteArray());
    setSize(qMax(qlonglong(0), settings.value("size").toLongLong()));
    setUrl(settings.value("url").toString());
    setUsePlugins(settings.value("usePlugins", true).toBool());

    const TransferItem::Status status = TransferItem::Status(settings.value("status", Paused).toInt());

    switch (status) {
    case Paused:
    case Failed:
    case Completed:
        setStatus(status);
        break;
    default:
        setStatus(Paused);
        break;
    }
}

void Transfer::save(QSettings &settings) {
    settings.setValue("customCommand", customCommand());
    settings.setValue("customCommandOverrideEnabled", customCommandOverrideEnabled());
    settings.setValue("downloadPath", downloadPath());
    settings.setValue("errorString", errorString());
    settings.setValue("fileName", fileName());
    settings.setValue("id", id());
    settings.setValue("postData", postData());
    settings.setValue("priority", TransferItem::Priority(priority()));
    settings.setValue("requestHeaders", requestHeaders());
    settings.setValue("requestMethod", requestMethod());
    settings.setValue("size", size());
    settings.setValue("url", url());
    settings.setValue("usePlugins", usePlugins());

    switch (status()) {
    case Paused:
    case Failed:
    case Completed:
        settings.setValue("status", TransferItem::Status(status()));
        break;
    default:
        settings.setValue("status", TransferItem::Status(Paused));
        break;
    }
}

bool Transfer::submitCaptchaResponse(const QString &response) {
    return m_requester ? m_requester->submitCaptchaResponse(response) : false;
}

bool Transfer::submitSettingsResponse(const QVariantMap &settings) {
    return m_requester ? m_requester->submitSettingsResponse(settings) : false;
}

void Transfer::cleanup() {
    if (m_requester) {
        m_requester->deleteLater();
        m_requester = 0;
    }

    if (m_nam) {
        m_nam->deleteLater();
        m_nam = 0;
    }
}

void Transfer::deleteFile() {
    if (m_file) {
        m_file->close();
    }

    if (QFile::exists(filePath())) {
        QFile::remove(filePath());
    }
}

void Transfer::initRequester() {
    if (!m_requester) {
        m_requester = new DownloadRequester(this);
        connect(m_requester, SIGNAL(captchaTimeoutChanged(int)), this, SLOT(onDownloadRequestCaptchaTimeoutChanged()));
        connect(m_requester, SIGNAL(downloadRequest(QNetworkRequest, QByteArray, QByteArray)),
                this, SLOT(onDownloadRequest(QNetworkRequest, QByteArray, QByteArray)));
        connect(m_requester, SIGNAL(error(QString)), this, SLOT(onDownloadRequestError(QString)));
        connect(m_requester, SIGNAL(requestedSettingsTimeoutChanged(int)),
                this, SLOT(onDownloadRequestRequestedSettingsTimeoutChanged()));
        connect(m_requester, SIGNAL(statusChanged(DownloadRequester::Status)),
                this, SLOT(onDownloadRequestStatusChanged(DownloadRequester::Status)));
        connect(m_requester, SIGNAL(waitTimeChanged(int)), this, SLOT(onDownloadRequestWaitTimeChanged()));
    }
}

void Transfer::initNetworkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }
}

bool Transfer::openFile() {
    if (!m_file) {
        m_file = new QFile(this);
    }

    if (m_file->isOpen()) {
        return true;
    }

    if (!QDir().mkpath(downloadPath())) {
        return false;
    }
    
    m_file->setFileName(filePath());
    return m_file->open(QFile::WriteOnly | QFile::Append);
}

void Transfer::startDownload() {
    Logger::log(QString("Transfer::startDownload(). URL: %1, Method: %2").arg(url()).arg(requestMethod()),
                Logger::LowVerbosity);
    m_redirects = 0;
    m_metadataSet = false;
    initNetworkAccessManager();
    
    QNetworkRequest request(url());
    QMapIterator<QString, QVariant> iterator(requestHeaders());
    
    while (iterator.hasNext()) {
        iterator.next();
        request.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
    }

    if (bytesTransferred() > 0) {
        Logger::log("Transfer::startDownload(). Setting 'Range' header to " + QString::number(bytesTransferred()),
                    Logger::MediumVerbosity);
        request.setRawHeader("Range", "bytes=" + QByteArray::number(bytesTransferred()) + "-");
    }

    if (postData().isEmpty()) {
        setStatus(Downloading);
        m_speedTime.start();
        m_reply = m_nam->sendCustomRequest(request, requestMethod().toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
    else {
        setStatus(Downloading);
        m_speedTime.start();
        QBuffer *buffer = new QBuffer;
        buffer->setData(postData().toUtf8());
        buffer->open(QBuffer::ReadOnly);
        m_reply = m_nam->sendCustomRequest(request, requestMethod().toUtf8(), buffer);
        buffer->setParent(m_reply);
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
}

void Transfer::followRedirect(const QUrl &url) {
    Logger::log("Transfer::followRedirect(): " + url.toString(), Logger::LowVerbosity);
    m_redirects++;
    initNetworkAccessManager();
    QNetworkRequest request(url);

    if (bytesTransferred() > 0) {
        Logger::log("Transfer::followRedirect(). Setting 'Range' header to " + QString::number(bytesTransferred()),
                    Logger::MediumVerbosity);
        request.setRawHeader("Range", "bytes=" + QByteArray::number(bytesTransferred()) + "-");
    }
    
    m_reply = m_nam->get(request);
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
}

void Transfer::onDownloadRequest(QNetworkRequest request, const QByteArray &method, const QByteArray &data) {
    Logger::log(QString("Transfer::onDownloadRequest(). URL: %1, Method: %2, Data: %3")
            .arg(request.url().toString()).arg(QString::fromUtf8(method)).arg(QString::fromUtf8(data)),
            Logger::LowVerbosity);
    m_redirects = 0;
    m_metadataSet = false;
    initNetworkAccessManager();

    if (bytesTransferred() > 0) {
        Logger::log("Transfer::startDownload(). Setting 'Range' header to " + QString::number(bytesTransferred()),
                    Logger::MediumVerbosity);
        request.setRawHeader("Range", "bytes=" + QByteArray::number(bytesTransferred()) + "-");
    }

    if (data.isEmpty()) {
        setStatus(Downloading);
        m_speedTime.start();
        m_reply = m_nam->sendCustomRequest(request, method);
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
    else {
        setStatus(Downloading);
        m_speedTime.start();
        QBuffer *buffer = new QBuffer;
        buffer->setData(data);
        buffer->open(QBuffer::ReadOnly);
        m_reply = m_nam->sendCustomRequest(request, method, buffer);
        buffer->setParent(m_reply);
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
}

void Transfer::onDownloadRequestCaptchaTimeoutChanged() {
    emit dataChanged(this, CaptchaTimeoutRole);
}

void Transfer::onDownloadRequestRequestedSettingsTimeoutChanged() {
    emit dataChanged(this, RequestedSettingsTimeoutRole);
}

void Transfer::onDownloadRequestWaitTimeChanged() {
    emit dataChanged(this, WaitTimeRole);
}

void Transfer::onDownloadRequestStatusChanged(DownloadRequester::Status s) {
    switch (s) {
    case DownloadRequester::Idle:
        setStatus(Queued);
        break;
    case DownloadRequester::RetrievingCaptchaChallenge:
        setStatus(RetrievingCaptchaChallenge);
        break;
    case DownloadRequester::AwaitingCaptchaResponse:
        setStatus(AwaitingCaptchaResponse);
        break;
    case DownloadRequester::RetrievingCaptchaResponse:
        setStatus(RetrievingCaptchaResponse);
        break;
    case DownloadRequester::SubmittingCaptchaResponse:
        setStatus(SubmittingCaptchaResponse);
        break;
    case DownloadRequester::ReportingCaptchaResponse:
        setStatus(ReportingCaptchaResponse);
        break;
    case DownloadRequester::AwaitingDecaptchaSettingsResponse:
    case DownloadRequester::AwaitingRecaptchaSettingsResponse:
    case DownloadRequester::AwaitingServiceSettingsResponse:
        setStatus(AwaitingSettingsResponse);
        break;
    case DownloadRequester::SubmittingDecaptchaSettingsResponse:
    case DownloadRequester::SubmittingRecaptchaSettingsResponse:
    case DownloadRequester::SubmittingServiceSettingsResponse:
        setStatus(SubmittingSettingsResponse);
        break;
    case DownloadRequester::WaitingActive:
        setStatus(WaitingActive);
        break;
    case DownloadRequester::WaitingInactive:
        setStatus(WaitingInactive);
        break;
    default:
        break;
    }
}

void Transfer::onDownloadRequestError(const QString &errorString) {
    setErrorString(errorString);
    setStatus(Failed);
}

void Transfer::onReplyMetaDataChanged() {
    if ((m_metadataSet) || (m_reply->error() != QNetworkReply::NoError)
        || (!m_reply->rawHeader("Location").isEmpty())) {
        return;
    }

    qint64 bytes = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    
    if (bytes <= 0) {
        bytes = m_reply->rawHeader("Content-Length").toLongLong();
    }

    Logger::log("Transfer::onReplyMetadataChanged(): Content-Length: " + QString::number(bytes),
                Logger::MediumVerbosity);
    
    if (bytes > 0) {
        setSize(bytes + bytesTransferred());
    }

    if (bytesTransferred() == 0) {
        // Only set the filename if no data has been written
        const QString contentDisposition =
        QString::fromUtf8(QByteArray::fromPercentEncoding(m_reply->rawHeader("Content-Disposition"))).remove('"');
        Logger::log("Transfer::onReplyMetadataChanged(): Content-Disposition: " + contentDisposition);
        
        if ((!contentDisposition.isEmpty()) && (CONTENT_DISPOSITION_REGEXP.indexIn(contentDisposition) != -1)) {
            const QString fileName = CONTENT_DISPOSITION_REGEXP.cap(2);
            
            if (!fileName.isEmpty()) {
                Logger::log("Transfer::onReplyMetadataChanged(): Found filename: " + fileName, Logger::MediumVerbosity);
                setFileName(Utils::getSanitizedFileName(fileName));
            }
        }
    }
    
    m_metadataSet = true;
}

void Transfer::onReplyReadyRead() {
    if (!m_metadataSet) {
        return;
    }

    const qint64 bytes = m_reply->bytesAvailable();

    if (bytes < DOWNLOAD_BUFFER_SIZE) {
        return;
    }

    if ((!openFile()) || (m_file->write(m_reply->read(bytes)) == -1)) {
        m_reply->deleteLater();
        m_reply = 0;
        setErrorString(tr("Cannot write to file - %1").arg(m_file->errorString()));
        setStatus(Failed);
        return;
    }
    
    setSpeed(int(bytes) * 1000 / qMax(1, m_speedTime.restart()));
    emit dataChanged(this, BytesTransferredRole);
}

void Transfer::onReplyFinished() {
    setSpeed(0);
    const QString redirect = QString::fromUtf8(m_reply->rawHeader("Location"));

    if (!redirect.isEmpty()) {
        m_file->close();
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect);
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
        }
        
        return;
    }

    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorString = m_reply->errorString();

    if ((m_reply->isOpen()) && (error == QNetworkReply::NoError) && (openFile())) {
        const qint64 bytes = m_reply->bytesAvailable();
        
        if ((bytes > 0) && (m_metadataSet)) {
            m_file->write(m_reply->read(bytes));
            emit dataChanged(this, BytesTransferredRole);
        }
    }

    m_file->close();
    m_reply->deleteLater();
    m_reply = 0;

    switch (error) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        if (status() == Canceling) {
            if (m_deleteFiles) {
                deleteFile();
                setStatus(CanceledAndDeleted);
            }
            else {
                setStatus(Canceled);
            }
        }
        else {
            setStatus(Paused);
        }

        return;
    default:
        setErrorString(errorString);
        setStatus(Failed);
        return;
    }

    setStatus(Completed);
}
