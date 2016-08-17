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
#include "decaptchaplugin.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "logger.h"
#include "recaptchaplugin.h"
#include "recaptchapluginmanager.h"
#include "serviceplugin.h"
#include "servicepluginconfig.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "utils.h"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

static QRegExp CONTENT_DISPOSITION_REGEXP("(filename=|filename\\*=UTF-8''|filename\\*= UTF-8'')([^;]+)");

Transfer::Transfer(QObject *parent) :
    TransferItem(parent),
    m_decaptchaPlugin(0),
    m_recaptchaPlugin(0),
    m_servicePlugin(0),
    m_nam(0),
    m_reply(0),
    m_file(0),
    m_timer(0),
    m_priority(NormalPriority),
    m_lastBytesTransferred(0),
    m_size(0),
    m_speed(0),
    m_status(Paused),
    m_requestMethod("GET"),
    m_servicePluginIcon(DEFAULT_ICON),
    m_customCommandOverrideEnabled(false),
    m_usePlugins(true),
    m_reportCaptchaError(false),
    m_metadataSet(false),
    m_redirects(0),
    m_timeRemaining(0)
{
}

QVariant Transfer::data(int role) const {
    switch (role) {
    case BytesTransferredRole:
        return bytesTransferred();
    case CaptchaImageRole:
        return captchaImage();
    case CaptchaTimeoutRole:
        return captchaTimeout();
    case CaptchaTimeoutStringRole:
        return captchaTimeoutString();
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
    case CaptchaImageRole:
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
    map[CaptchaImageRole] = captchaImage();
    map[CaptchaTimeoutRole] = captchaTimeout();
    map[CaptchaTimeoutStringRole] = captchaTimeoutString();
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
    map[roleNames().value(CaptchaImageRole)] = captchaImage();
    map[roleNames().value(CaptchaTimeoutRole)] = captchaTimeout();
    map[roleNames().value(CaptchaTimeoutStringRole)] = captchaTimeoutString();
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

QByteArray Transfer::captchaImage() const {
    return m_captchaImageData;
}

void Transfer::setCaptchaImage(const QImage &image) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QBuffer::WriteOnly);
    image.save(&buffer, "JPEG");
    m_captchaImageData = ba.toBase64();
    emit dataChanged(this, CaptchaImageRole);
}

void Transfer::clearCaptchaImage() {
    if (!m_captchaImageData.isEmpty()) {
        m_captchaImageData.clear();
        emit dataChanged(this, CaptchaImageRole);
    }
}

int Transfer::captchaTimeout() const {
    return status() == AwaitingCaptchaResponse ? m_timeRemaining : 0;
}

QString Transfer::captchaTimeoutString() const {
    return Utils::formatMSecs(captchaTimeout());
}

void Transfer::updateCaptchaTimeout() {
    switch (status()) {
    case AwaitingCaptchaResponse:
        m_timeRemaining -= m_timer->interval();
        emit dataChanged(this, CaptchaTimeoutRole);
        
        if (m_timeRemaining <= 0) {
            stopWaitTimer();
            setErrorString(tr("No captcha response"));
            setStatus(Failed);
        }
        
        break;
    default:
        stopWaitTimer();
        break;
    }
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

void Transfer::updateSpeed() {
    if (status() == Downloading) {
        emit dataChanged(this, SpeedRole);
    }
    else {
        stopSpeedTimer();
    }
}

QVariantList Transfer::requestedSettings() const {
    return m_requestedSettings;
}

void Transfer::setRequestedSettings(const QString &title, const QVariantList &settings) {
    m_requestedSettingsTitle = title;
    m_requestedSettings = settings;
    emit dataChanged(this, RequestedSettingsRole);
    emit dataChanged(this, RequestedSettingsTitleRole);
}

void Transfer::clearRequestedSettings() {
    if (!m_requestedSettings.isEmpty()) {
        m_requestedSettingsTitle.clear();
        m_requestedSettings.clear();
        emit dataChanged(this, RequestedSettingsRole);
        emit dataChanged(this, RequestedSettingsTitleRole);
    }
}

int Transfer::requestedSettingsTimeout() const {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return m_timeRemaining;
    default:
        return 0;
    }
}

QString Transfer::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString Transfer::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void Transfer::updateRequestedSettingsTimeout() {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        m_timeRemaining -= m_timer->interval();
        emit dataChanged(this, RequestedSettingsTimeoutRole);
        
        if (m_timeRemaining <= 0) {
            stopWaitTimer();
            setErrorString(tr("No settings response"));
            setStatus(Failed);
        }
        
        break;
    default:
        stopWaitTimer();
        break;
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
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
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
        
        if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(u)) {
            setPluginId(config->id());
            setPluginName(config->displayName());
            
            if (!config->iconFilePath().isEmpty()) {
                setPluginIconPath(config->iconFilePath());
            }
            else {
                setPluginIconPath(DEFAULT_ICON);
            }
        }
        else {
            setPluginId(QString());
            setPluginIconPath(DEFAULT_ICON);
        }
    }
}

int Transfer::waitTime() const {
    return m_timeRemaining;
}

QString Transfer::waitTimeString() const {
    return Utils::formatMSecs(waitTime());
}

void Transfer::updateWaitTime() {
    switch (status()) {
    case WaitingActive:
    case WaitingInactive:
        m_timeRemaining -= m_timer->interval();
        emit dataChanged(this, WaitTimeRole);
        
        if (m_timeRemaining <= 0) {
            stopWaitTimer();
            
            if (status() == WaitingInactive) {
                setStatus(Queued);
            }
        }
        
        break;
    default:
        stopWaitTimer();
        break;
    }
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
            return true;
        }
        
        if (initServicePlugin()) {
            setStatus(Connecting);
            m_servicePlugin->getDownloadRequest(url());
            return true;
        }

        setErrorString(tr("No service plugin found"));
        setStatus(Failed);
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
        stopWaitTimer();
        break;
    case WaitingActive:
        if ((m_servicePlugin) && (!m_servicePlugin->cancelCurrentOperation())) {
            return false;
        }

        stopWaitTimer();
        break;
    case RetrievingCaptchaChallenge:
    case SubmittingRecaptchaSettingsResponse:
        if ((m_recaptchaPlugin) && (!m_recaptchaPlugin->cancelCurrentOperation())) {
            return false;
        }

        break;
    case RetrievingCaptchaResponse:
    case ReportingCaptchaResponse:
    case SubmittingDecaptchaSettingsResponse:
        if ((m_decaptchaPlugin) && (!m_decaptchaPlugin->cancelCurrentOperation())) {
            return false;
        }

        break;
    case SubmittingCaptchaResponse:
    case SubmittingServiceSettingsResponse:
        if ((m_servicePlugin) && (!m_servicePlugin->cancelCurrentOperation())) {
            return false;
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
        stopWaitTimer();
        break;
    case WaitingActive:
        if ((m_servicePlugin) && (!m_servicePlugin->cancelCurrentOperation())) {
            return false;
        }

        stopWaitTimer();
        break;
    case RetrievingCaptchaChallenge:
    case SubmittingRecaptchaSettingsResponse:
        if ((m_recaptchaPlugin) && (!m_recaptchaPlugin->cancelCurrentOperation())) {
            return false;
        }

        break;
    case RetrievingCaptchaResponse:
    case ReportingCaptchaResponse:
    case SubmittingDecaptchaSettingsResponse:
        if ((m_decaptchaPlugin) && (!m_decaptchaPlugin->cancelCurrentOperation())) {
            return false;
        }

        break;
    case SubmittingCaptchaResponse:
    case SubmittingServiceSettingsResponse:
        if ((m_servicePlugin) && (!m_servicePlugin->cancelCurrentOperation())) {
            return false;
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
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    clearCaptchaImage();

    if (response.isEmpty()) {
        setErrorString(tr("No captcha response"));
        setStatus(Failed);
        return false;
    }
    
    if (!initServicePlugin()) {
        setErrorString(tr("No service plugin found"));
        setStatus(Failed);
        return false;
    }
    
    setStatus(SubmittingCaptchaResponse);
    m_captchaResponse = response;
    
    if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QString, m_captchaChallenge),
                                   Q_ARG(QString, response))) {
        setErrorString(tr("Invalid captcha callback method"));
        setStatus(Failed);
        return false;
    }
    
    return true;
}

bool Transfer::submitSettingsResponse(const QVariantMap &settings) {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
        clearRequestedSettings();
        
        if (settings.isEmpty()) {
            setErrorString(tr("No settings response"));
            setStatus(Failed);
            return false;
        }
        
        if (!initDecaptchaPlugin(m_decaptchaPluginId)) {
            setErrorString(tr("No decaptcha plugin found"));
            setStatus(Failed);
            return false;
        }

        setStatus(SubmittingDecaptchaSettingsResponse);

        if (!QMetaObject::invokeMethod(m_decaptchaPlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            setErrorString(tr("Invalid settings callback method"));
            setStatus(Failed);
        }
        
        return true;
    case AwaitingRecaptchaSettingsResponse:
        clearRequestedSettings();
        
        if (settings.isEmpty()) {
            setErrorString(tr("No settings response"));
            setStatus(Failed);
            return false;
        }
        
        if (!initRecaptchaPlugin(m_recaptchaPluginId)) {
            setErrorString(tr("No recaptcha plugin found"));
            setStatus(Failed);
            return false;
        }

        setStatus(SubmittingRecaptchaSettingsResponse);

        if (!QMetaObject::invokeMethod(m_recaptchaPlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            setErrorString(tr("Invalid settings callback method"));
            setStatus(Failed);
        }

        return true;
    case AwaitingServiceSettingsResponse:
        clearRequestedSettings();
        
        if (settings.isEmpty()) {
            setErrorString(tr("No settings response"));
            setStatus(Failed);
            return false;
        }
        
        if (!initServicePlugin()) {
            setErrorString(tr("No service plugin found"));
            setStatus(Failed);
            return false;
        }

        setStatus(SubmittingServiceSettingsResponse);

        if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            setErrorString(tr("Invalid settings callback method"));
            setStatus(Failed);
        }

        return true;
    default:
        return false;
    }
}

void Transfer::cleanup() {
    if (m_decaptchaPlugin) {
        m_decaptchaPlugin->deleteLater();
        m_decaptchaPlugin = 0;
    }

    if (m_recaptchaPlugin) {
        m_recaptchaPlugin->deleteLater();
        m_recaptchaPlugin = 0;
    }

    if (m_servicePlugin) {
        m_servicePlugin->deleteLater();
        m_servicePlugin = 0;
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

bool Transfer::initDecaptchaPlugin(const QString &pluginId) {
    if ((pluginId == m_decaptchaPluginId) && (m_decaptchaPlugin)) {
        return true;
    }

    m_decaptchaPluginId = pluginId;

    if (m_decaptchaPlugin) {
        m_decaptchaPlugin->deleteLater();
        m_decaptchaPlugin = 0;
    }

    m_decaptchaPlugin = DecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (!m_decaptchaPlugin) {
        Logger::log("Transfer::initDecaptchaPlugin(): No plugin found for " + pluginId);
        return false;
    }

    initNetworkAccessManager();
    m_decaptchaPlugin->setNetworkAccessManager(m_nam);
    connect(m_decaptchaPlugin, SIGNAL(captchaResponse(QString, QString)), this, SLOT(onCaptchaResponse(QString, QString)));
    connect(m_decaptchaPlugin, SIGNAL(captchaResponseReported(QString)), this, SLOT(onCaptchaResponseReported(QString)));
    connect(m_decaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onDecaptchaError(QString)));
    connect(m_decaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onDecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    return true;
}

bool Transfer::initRecaptchaPlugin(const QString &pluginId) {
    if ((pluginId == m_recaptchaPluginId) && (m_recaptchaPlugin)) {
        return true;
    }

    m_recaptchaPluginId = pluginId;

    if (m_recaptchaPlugin) {
        m_recaptchaPlugin->deleteLater();
        m_recaptchaPlugin = 0;
    }

    m_recaptchaPlugin = RecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (!m_recaptchaPlugin) {
        Logger::log("Transfer::initRecaptchaPlugin(): No plugin found for " + pluginId);
        return false;
    }

    initNetworkAccessManager();
    m_recaptchaPlugin->setNetworkAccessManager(m_nam);
    connect(m_recaptchaPlugin, SIGNAL(captcha(QString, QImage)), this, SLOT(onCaptchaReady(QString, QImage)));
    connect(m_recaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onRecaptchaError(QString)));
    connect(m_recaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onRecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    return true;
}

bool Transfer::initServicePlugin() {
    if (m_servicePlugin) {
        return true;
    }
    
    m_servicePlugin = ServicePluginManager::instance()->createPluginById(pluginId(), this);

    if (!m_servicePlugin) {
        Logger::log("Transfer::initServicePlugin(): No plugin found for " + url());
        return false;
    }

    initNetworkAccessManager();
    m_servicePlugin->setNetworkAccessManager(m_nam);
    connect(m_servicePlugin, SIGNAL(captchaRequest(QString, QString, QByteArray)),
            this, SLOT(onCaptchaRequest(QString, QString, QByteArray)));
    connect(m_servicePlugin, SIGNAL(downloadRequest(QNetworkRequest, QByteArray, QByteArray)),
            this, SLOT(onDownloadRequest(QNetworkRequest, QByteArray, QByteArray)));
    connect(m_servicePlugin, SIGNAL(error(QString)), this, SLOT(onServiceError(QString)));
    connect(m_servicePlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onServiceSettingsRequest(QString, QVariantList, QByteArray)));
    connect(m_servicePlugin, SIGNAL(waitRequest(int, bool)), this, SLOT(onWaitRequest(int, bool)));
    return true;
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

void Transfer::startSpeedTimer() {
    m_speedTime.start();
}

void Transfer::startWaitTimer(int msecs, const char* slot) {
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
    }
    
    m_timeRemaining = msecs;
    disconnect(m_timer, 0, this, 0);
    connect(m_timer, SIGNAL(timeout()), this, slot);
    m_timer->start();
}

void Transfer::stopSpeedTimer() {
    if (m_timer) {
        m_timer->stop();
    }
}

void Transfer::stopWaitTimer() {
    if (m_timer) {
        m_timer->stop();
    }
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
        startSpeedTimer();
        m_reply = m_nam->sendCustomRequest(request, requestMethod().toUtf8());
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
    else {
        setStatus(Downloading);
        startSpeedTimer();
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

void Transfer::onCaptchaReady(const QString &challenge, const QImage &image) {    
    if (image.isNull()) {
        setErrorString(tr("Invalid captcha image"));
        setStatus(Failed);
        return;
    }

    m_captchaChallenge = challenge;
    const QString pluginId = Settings::decaptchaPlugin();
    
    if (!pluginId.isEmpty()) {
        if (initDecaptchaPlugin(pluginId)) {
            setStatus(RetrievingCaptchaResponse);
            m_reportCaptchaError = true;
            m_decaptchaPlugin->getCaptchaResponse(image);
            return;
        }
    }

    m_reportCaptchaError = false;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateCaptchaTimeout()));
    setCaptchaImage(image);
    setStatus(AwaitingCaptchaResponse);
}

void Transfer::onCaptchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey,
                                const QByteArray &callback) {
    if (!initRecaptchaPlugin(recaptchaPluginId)) {
        setErrorString(tr("No recaptcha plugin found"));
        setStatus(Failed);
        return;
    }

    setStatus(RetrievingCaptchaChallenge);
    m_callback = callback;
    m_recaptchaPlugin->getCaptcha(recaptchaKey);
}

void Transfer::onCaptchaResponse(const QString &captchaId, const QString &response) {
    if (!initServicePlugin()) {
        setErrorString(tr("No service plugin found"));
        setStatus(Failed);
        return;
    }

    m_decaptchaId = captchaId;
    m_captchaResponse = response;

    if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QString, m_captchaChallenge),
                                   Q_ARG(QString, response))) {
        setErrorString(tr("Invalid captcha callback method"));
        setStatus(Failed);
    }
}

void Transfer::onCaptchaResponseReported(const QString &) {
    if (!initRecaptchaPlugin(m_recaptchaPluginId)) {
        setErrorString(tr("No recaptcha plugin found"));
        setStatus(Failed);
        return;
    }

    m_recaptchaPlugin->getCaptcha(m_recaptchaKey);
}

void Transfer::onDownloadRequest(QNetworkRequest request, const QByteArray &method, const QByteArray &data) {
    Logger::log(QString("Transfer::onDownloadRequest(). URL: %1, Method: %2, Data: %3")
                       .arg(request.url().toString())
                       .arg(QString::fromUtf8(method)).arg(QString::fromUtf8(data)), Logger::LowVerbosity);
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
        startSpeedTimer();
        m_reply = m_nam->sendCustomRequest(request, method);
        connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
    else {
        setStatus(Downloading);
        startSpeedTimer();
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

void Transfer::onWaitRequest(int msecs, bool isLongDelay) {
    startWaitTimer(msecs, SLOT(updateWaitTime()));

    if (isLongDelay) {
        setStatus(WaitingInactive);
    }
    else {
        setStatus(WaitingActive);
    }    
}

void Transfer::onDecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
                                          const QByteArray &callback) {
    m_callback = callback;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setRequestedSettings(title, settings);
    setStatus(AwaitingDecaptchaSettingsResponse);
}

void Transfer::onRecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
                                          const QByteArray &callback) {
    m_callback = callback;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setRequestedSettings(title, settings);
    setStatus(AwaitingRecaptchaSettingsResponse);
}

void Transfer::onServiceSettingsRequest(const QString &title, const QVariantList &settings,
                                        const QByteArray &callback) {
    m_callback = callback;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setRequestedSettings(title, settings);
    setStatus(AwaitingServiceSettingsResponse);
}

void Transfer::onDecaptchaError(const QString &errorString) {
    setErrorString(errorString.isEmpty() ? tr("Unknown decaptcha plugin error")
                                         : tr("Decaptcha error - %1").arg(errorString));
    setStatus(Failed);
}

void Transfer::onRecaptchaError(const QString &errorString) {
    setErrorString(errorString.isEmpty() ? tr("Unknown recaptcha plugin error")
                                         : tr("Recaptcha error - %1").arg(errorString));
    setStatus(Failed);
}

void Transfer::onServiceError(const QString &errorString) {
    setErrorString(errorString.isEmpty() ? tr("Unknown service plugin error")
                                         : tr("Service error - %1").arg(errorString));
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
