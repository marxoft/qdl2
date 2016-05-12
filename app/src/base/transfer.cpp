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
#include "captchadialog.h"
#include "decaptchaplugin.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "logger.h"
#include "pluginsettingsdialog.h"
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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

static QRegExp CONTENT_DISPOSITION_REGEXP("(=\")(.+)(\";)");

Transfer::Transfer(QObject *parent) :
    TransferItem(parent),
    m_decaptchaPlugin(0),
    m_recaptchaPlugin(0),
    m_servicePlugin(0),
    m_nam(0),
    m_reply(0),
    m_file(0),
    m_timer(0),
    m_captchaDialog(0),
    m_settingsDialog(0),
    m_priority(NormalPriority),
    m_lastBytesTransferred(0),
    m_size(0),
    m_speed(0),
    m_status(Paused),
    m_servicePluginIcon(DEFAULT_ICON),
    m_servicePluginName(tr("Unknown")),
    m_reportCaptchaError(false),
    m_metadataSet(false),
    m_canceled(false),
    m_redirects(0),
    m_waitTime(0)
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
    case DownloadPathRole:
        setDownloadPath(value.toString());
        return true;
    case FileNameRole:
    case NameRole:
        setFileName(value.toString());
        return true;
    case IdRole:
        setId(value.toString());
        return true;
    case PriorityRole:
        setPriority(TransferItem::Priority(value.toInt()));
        return true;
    case RequestedSettingsRole:
        return submitSettingsResponse(value.toMap());
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
    default:
        return TransferItem::setData(role, value);
    }
}

QMap<int, QVariant> Transfer::itemData() const {
    QMap<int, QVariant> map = TransferItem::itemData();
    map[BytesTransferredRole] = bytesTransferred();
    map[CaptchaImageRole] = captchaImage();
    map[CaptchaTimeoutRole] = captchaTimeout();
    map[DownloadPathRole] = downloadPath();
    map[ErrorStringRole] = errorString();
    map[FileNameRole] = fileName();
    map[FilePathRole] = filePath();
    map[IdRole] = id();
    map[PluginIconPathRole] = pluginIconPath();
    map[PluginIdRole] = pluginId();
    map[PluginNameRole] = pluginName();
    map[PriorityRole] = priority();
    map[PriorityStringRole] = priorityString();
    map[ProgressRole] = progress();
    map[ProgressStringRole] = progressString();
    map[RequestedSettingsRole] = requestedSettings();
    map[RequestedSettingsTimeoutRole] = requestedSettingsTimeout();
    map[SizeRole] = size();
    map[SpeedRole] = speed();
    map[SpeedStringRole] = speedString();
    map[StatusRole] = status();
    map[StatusStringRole] = statusString();
    map[SuffixRole] = fileSuffix();
    map[UrlRole] = url();
    map[WaitTimeRole] = waitTime();
    map[WaitTimeStringRole] = waitTimeString();
    return map;
}

QVariantMap Transfer::itemDataWithRoleNames() const {
    QVariantMap map = TransferItem::itemDataWithRoleNames();
    map[roleNames().value(BytesTransferredRole)] = bytesTransferred();
    map[roleNames().value(CaptchaImageRole)] = captchaImage();
    map[roleNames().value(CaptchaTimeoutRole)] = captchaTimeout();
    map[roleNames().value(DownloadPathRole)] = downloadPath();
    map[roleNames().value(ErrorStringRole)] = errorString();
    map[roleNames().value(FileNameRole)] = fileName();
    map[roleNames().value(FilePathRole)] = filePath();
    map[roleNames().value(IdRole)] = id();
    map[roleNames().value(NameRole)] = fileName();
    map[roleNames().value(PluginIconPathRole)] = pluginIconPath();
    map[roleNames().value(PluginIdRole)] = pluginId();
    map[roleNames().value(PluginNameRole)] = pluginName();
    map[roleNames().value(PriorityRole)] = priority();
    map[roleNames().value(PriorityStringRole)] = priorityString();
    map[roleNames().value(ProgressRole)] = progress();
    map[roleNames().value(ProgressStringRole)] = progressString();
    map[roleNames().value(RequestedSettingsRole)] = requestedSettings();
    map[roleNames().value(RequestedSettingsTimeoutRole)] = requestedSettingsTimeout();
    map[roleNames().value(SizeRole)] = size();
    map[roleNames().value(SpeedRole)] = speed();
    map[roleNames().value(SpeedStringRole)] = speedString();
    map[roleNames().value(StatusRole)] = status();
    map[roleNames().value(StatusStringRole)] = statusString();
    map[roleNames().value(SuffixRole)] = fileSuffix();
    map[roleNames().value(UrlRole)] = url();
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
    case Canceled:
    case CanceledAndDeleted:
        return false;
    default:
        return true;
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

    if (buffer.open(QBuffer::WriteOnly)) {
        image.save(&buffer);
        buffer.close();
    }

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
    switch (status()) {
    case Transfer::AwaitingCaptchaResponse:
        return m_captchaDialog.isNull() ? 0 : m_captchaDialog->timeRemaining();
    default:
        return 0;
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

void Transfer::setRequestedSettings(const QVariantList &settings) {
    m_requestedSettings = settings;
    emit dataChanged(this, RequestedSettingsRole);
}

void Transfer::clearRequestedSettings() {
    if (!m_requestedSettings.isEmpty()) {
        m_requestedSettings.clear();
        emit dataChanged(this, RequestedSettingsRole);
    }
}

int Transfer::requestedSettingsTimeout() const {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return m_settingsDialog.isNull() ? 0 : m_settingsDialog->timeRemaining();
    default:
        return 0;
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
    return m_waitTime;
}

QString Transfer::waitTimeString() const {
    return Utils::formatMSecs(waitTime());
}

void Transfer::updateWaitTime() {
    switch (status()) {
    case WaitingActive:
    case WaitingInactive:
        m_waitTime -= m_timer->interval();
        emit dataChanged(this, WaitTimeRole);
        
        if (m_waitTime <= 0) {
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
                m_canceled = false;
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
                m_canceled = true;
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
    setDownloadPath(settings.value("downloadPath").toString());
    setErrorString(settings.value("errorString").toString());
    setFileName(settings.value("fileName").toString());
    setId(settings.value("id").toString());
    setPriority(TransferItem::Priority(settings.value("priority").toInt()));
    setSize(qMax(qlonglong(0), settings.value("size").toLongLong()));
    setUrl(settings.value("url").toString());

    const TransferItem::Status status = TransferItem::Status(settings.value("status").toInt());

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
    settings.setValue("downloadPath", downloadPath());
    settings.setValue("errorString", errorString());
    settings.setValue("fileName", fileName());
    settings.setValue("id", id());
    settings.setValue("priority", TransferItem::Priority(priority()));
    settings.setValue("size", size());
    settings.setValue("url", url());

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

void Transfer::openCaptchaDialog(const QImage &image) {
    CaptchaDialog dialog;
    m_captchaDialog = &dialog;
    dialog.setImage(image);
    connect(this, SIGNAL(dataChanged(TransferItem*, int)), &dialog, SLOT(reject()));
    
    if (dialog.exec() == QDialog::Accepted) {
        submitCaptchaResponse(dialog.response());
    }
    else if (status() == AwaitingCaptchaResponse) {
        setErrorString(tr("No captcha response"));
        setStatus(Failed);
    }
}

void Transfer::startSpeedTimer() {
    m_speedTime.start();
}

void Transfer::startWaitTimer() {
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
    }

    disconnect(m_timer, 0, this, 0);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
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

void Transfer::followRedirect(const QUrl &url) {
    Logger::log("Transfer::followRedirect(): " + url.toString());
    m_redirects++;
    initNetworkAccessManager();
    QNetworkRequest request(url);

    if (bytesTransferred() > 0) {
        Logger::log("Transfer::followRedirect(). Setting 'Range' header to " + QString::number(bytesTransferred()));
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
    setStatus(AwaitingCaptchaResponse);
    setCaptchaImage(image);
    openCaptchaDialog(image);
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
                       .arg(QString::fromUtf8(method)).arg(QString::fromUtf8(data)));
    m_redirects = 0;
    initNetworkAccessManager();

    if (bytesTransferred() > 0) {
        Logger::log("Transfer::onDownloadRequest(). Setting 'Range' header to " + QString::number(bytesTransferred()));
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
    m_waitTime = msecs;
    startWaitTimer();

    if (isLongDelay) {
        setStatus(WaitingInactive);
    }
    else {
        setStatus(WaitingActive);
    }    
}

void Transfer::onDecaptchaSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback) {
    m_callback = callback;
    setRequestedSettings(settings);
    setStatus(AwaitingDecaptchaSettingsResponse);
    PluginSettingsDialog dialog(settings);
    m_settingsDialog = &dialog;
    dialog.setWindowTitle(title);
    connect(this, SIGNAL(dataChanged(TransferItem*, int)), &dialog, SLOT(reject()));
    
    if (dialog.exec() == QDialog::Accepted) {
        submitSettingsResponse(dialog.settings());
    }
    else if (status() == AwaitingDecaptchaSettingsResponse) {
        setErrorString(tr("No settings response"));
        setStatus(Failed);
    }
}

void Transfer::onRecaptchaSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback) {
    m_callback = callback;
    setRequestedSettings(settings);
    setStatus(AwaitingRecaptchaSettingsResponse);
    PluginSettingsDialog dialog(settings);
    m_settingsDialog = &dialog;
    dialog.setWindowTitle(title);
    connect(this, SIGNAL(dataChanged(TransferItem*, int)), &dialog, SLOT(reject()));
    
    if (dialog.exec() == QDialog::Accepted) {
        submitSettingsResponse(dialog.settings());
    }
    else if (status() == AwaitingRecaptchaSettingsResponse) {
        setErrorString(tr("No settings response"));
        setStatus(Failed);
    }
}

void Transfer::onServiceSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback) {
    m_callback = callback;
    setRequestedSettings(settings);
    setStatus(AwaitingServiceSettingsResponse);
    PluginSettingsDialog dialog(settings);
    m_settingsDialog = &dialog;
    dialog.setWindowTitle(title);
    connect(this, SIGNAL(dataChanged(TransferItem*, int)), &dialog, SLOT(reject()));
    
    if (dialog.exec() == QDialog::Accepted) {
        submitSettingsResponse(dialog.settings());
    }
    else if (status() == AwaitingServiceSettingsResponse) {
        setErrorString(tr("No settings response"));
        setStatus(Failed);
    }
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
    if (m_metadataSet) {
        return;
    }
    
    QVariant redirect = m_reply->header(QNetworkRequest::LocationHeader);

    if (!redirect.isNull()) {
        return;
    }

    qint64 bytes = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    
    if (bytes <= 0) {
        bytes = m_reply->rawHeader("Content-Length").toLongLong();
    }

    Logger::log("Transfer::onReplyMetadataChanged(): Content-Length: " + QString::number(bytes));
    
    if (bytes > 0) {
        setSize(bytes + bytesTransferred());
    }

    if (bytesTransferred() == 0) {
        // Only set the filename if no data has been written
        const QString contentDisposition =
        QString::fromUtf8(QByteArray::fromPercentEncoding(m_reply->rawHeader("Content-Disposition")));
        Logger::log("Transfer::onReplyMetadataChanged(): Content-Disposition: " + contentDisposition);
        
        if ((!contentDisposition.isEmpty()) && (CONTENT_DISPOSITION_REGEXP.indexIn(contentDisposition) != -1)) {
            const QString fileName = CONTENT_DISPOSITION_REGEXP.cap(2);
            
            if (!fileName.isEmpty()) {
                setFileName(Utils::getSanitizedFileName(fileName));
            }
        }
    }

    if (!openFile()) {
	m_reply->deleteLater();
	m_reply = 0;
	setErrorString(tr("Cannot open file: %1").arg(m_file->errorString()));
	setStatus(Failed);
	return;
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

    if (m_file->write(m_reply->read(bytes)) == -1) {
        m_reply->deleteLater();
	m_reply = 0;
        setErrorString(tr("Cannot write to file: %1").arg(m_file->errorString()));
        setStatus(Failed);
        return;
    }
    
    setSpeed(int(bytes) * 1000 / qMax(1, m_speedTime.restart()));
    emit dataChanged(this, BytesTransferredRole);
}

void Transfer::onReplyFinished() {
    setSpeed(0);
    const QVariant redirect = m_reply->header(QNetworkRequest::LocationHeader);

    if (!redirect.isNull()) {
	m_file->close();
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect.toUrl());
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
        }
        
        return;
    }

    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorString = m_reply->errorString();

    if (m_reply->isOpen()) {
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
        if (m_canceled) {
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
