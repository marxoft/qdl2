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

#include "urlcheckmodel.h"
#include "captchatype.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "logger.h"
#include "pluginsettings.h"
#include "recaptchapluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "utils.h"
#include <QIcon>
#include <QTimer>

UrlCheckModel* UrlCheckModel::self = 0;

UrlCheckModel::UrlCheckModel() :
    QAbstractListModel(),
    m_timer(0),
    m_servicePlugin(0),
    m_recaptchaPlugin(0),
    m_decaptchaPlugin(0),
    m_captchaType(CaptchaType::Unknown),
    m_status(Idle),
    m_reportCaptchaError(false),
    m_index(-1),
    m_timeRemaining(0)
{
    m_roles[UrlRole] = "url";
    m_roles[FileNameRole] = "fileName";
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

QString UrlCheckModel::captchaTypeString() const {
    switch (captchaType()) {
    case CaptchaType::Image:
        return tr("Image");
    case CaptchaType::NoCaptcha:
        return tr("NoCaptcha");
    default:
        return tr("Unknown");
    }
}

QByteArray UrlCheckModel::captchaData() const {
    return m_captchaData;
}

void UrlCheckModel::setCaptchaData(int captchaType, const QByteArray &captchaData) {
    m_captchaType = captchaType;

    if (captchaType == CaptchaType::NoCaptcha) {
        m_captchaChallenge.clear();
        m_captchaData = captchaData;
    }
    else {
        const int newline = captchaData.indexOf("\n");
        m_captchaChallenge = QString::fromUtf8(captchaData.left(newline));
        m_captchaData = captchaData.mid(newline + 1);
    }
}

void UrlCheckModel::clearCaptchaData() {
    m_captchaType = CaptchaType::Unknown;
    m_captchaChallenge.clear();
    m_captchaData.clear();
}

int UrlCheckModel::captchaTimeout() const {
    return status() == AwaitingCaptchaResponse ? m_timeRemaining : 0;
}

QString UrlCheckModel::captchaTimeoutString() const {
    return Utils::formatMSecs(captchaTimeout());
}

void UrlCheckModel::updateCaptchaTimeout() {
    switch (status()) {
    case AwaitingCaptchaResponse:
        m_timeRemaining -= m_timer->interval();
        emit captchaTimeoutChanged(m_timeRemaining);

        if (m_timeRemaining <= 0) {
            stopWaitTimer();
            onError(tr("No captcha response"));
        }

        break;
    default:
        stopWaitTimer();
        break;
    }
}

int UrlCheckModel::progress() const {
    return (!m_items.isEmpty()) && (m_index > 0) ? m_index * 100 / m_items.size() : 0;
}

QVariantList UrlCheckModel::requestedSettings() const {
    return m_requestedSettings;
}

void UrlCheckModel::setRequestedSettings(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    m_requestedSettingsTitle = title;
    m_requestedSettings = settings;
    m_callback = callback;
}

void UrlCheckModel::clearRequestedSettings() {
    m_requestedSettingsTitle.clear();
    m_requestedSettings.clear();
}

int UrlCheckModel::requestedSettingsTimeout() const {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return m_timeRemaining;
    default:
        return 0;
    }
}

QString UrlCheckModel::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString UrlCheckModel::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void UrlCheckModel::startWaitTimer(int msecs, const char *slot) {
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
    }
    else {
        disconnect(m_timer, 0, this, 0);
    }

    connect(m_timer, SIGNAL(timeout()), this, slot);
    m_timeRemaining = msecs;
    m_timer->start();
}

void UrlCheckModel::stopWaitTimer() {
    if (m_timer) {
        m_timer->stop();
    }
}

void UrlCheckModel::updateRequestedSettingsTimeout() {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        m_timeRemaining -= m_timer->interval();
        emit requestedSettingsTimeoutChanged(m_timeRemaining);
        
        if (m_timeRemaining <= 0) {
            stopWaitTimer();
            onError(tr("No settings response"));
        }
        
        break;
    default:
        stopWaitTimer();
        break;
    }
}

UrlCheckModel::Status UrlCheckModel::status() const {
    return m_status;
}

void UrlCheckModel::setStatus(UrlCheckModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
        
        switch (s) {
        case Idle:
        case Completed:
        case Canceled:
            m_index = -1;
            break;
        default:
            break;
        }
    }
}

QString UrlCheckModel::statusString() const {
    switch (status()) {
    case Active:
    case AwaitingCaptchaResponse:
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return tr("Checking URLs");
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    default:
        return QString();
    }
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
                    return QIcon::fromTheme("dialog-yes");
                }
                
                return QIcon::fromTheme("dialog-no");
            }

            return QIcon::fromTheme("dialog-question");
        default:
            return QVariant();
        }
    case UrlRole:
        return m_items.at(index.row()).url;
    case FileNameRole:
        return m_items.at(index.row()).fileName;
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

void UrlCheckModel::append(const QString &url) {
    Logger::log("UrlCheckModel::append(): " + url, Logger::LowVerbosity);
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << UrlCheck(url);
    endInsertRows();
    emit countChanged(rowCount());
    emit progressChanged(progress());

    if (status() != Active) {
        next();
    }
}

void UrlCheckModel::append(const QStringList &urls) {
    foreach (const QString &url, urls) {
        append(url);
    }
}

bool UrlCheckModel::remove(int row) {
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

void UrlCheckModel::cancel() {
    clearPlugins();
    setStatus(Canceled);
}

void UrlCheckModel::clear() {
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

bool UrlCheckModel::submitCaptchaResponse(const QString &response) {
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    setStatus(Active);
    stopWaitTimer();

    if (response.isEmpty()) {
        onError(tr("No captcha response"));
        return false;
    }

    if (!m_servicePlugin) {
        onError(tr("No plugin found"));
        return false;
    }

    if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QString, m_captchaChallenge),
                Q_ARG(QString, response))) {
        onError(tr("Invalid captcha callback method"));
        clearCaptchaData();
        return false;
    }

    clearCaptchaData();
    return true;
}

bool UrlCheckModel::submitSettingsResponse(const QVariantMap &settings) {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
        setStatus(Active);
        clearRequestedSettings();
        stopWaitTimer();

        if (settings.isEmpty()) {
            onError(tr("No settings response"));
            return false;
        }

        if (!m_decaptchaPlugin) {
            onError(tr("No decaptcha plugin found"));
            return false;
        }

        if (!QMetaObject::invokeMethod(m_decaptchaPlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            onError(tr("Invalid settings callback method"));
            return false;
        }

        return true;
    case AwaitingRecaptchaSettingsResponse:
        setStatus(Active);
        clearRequestedSettings();
        stopWaitTimer();

        if (settings.isEmpty()) {
            onError(tr("No settings response"));
            return false;
        }

        if (!m_recaptchaPlugin) {
            onError(tr("No recaptcha plugin found"));
            return false;
        }

        if (!QMetaObject::invokeMethod(m_recaptchaPlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            onError(tr("Invalid settings callback method"));
            return false;
        }

        return true;
    case AwaitingServiceSettingsResponse:
        setStatus(Active);
        clearRequestedSettings();
        stopWaitTimer();

        if (settings.isEmpty()) {
            onError(tr("No settings response"));
            return false;
        }

        if (!m_servicePlugin) {
            onError(tr("No service plugin found"));
            return false;
        }

        if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QVariantMap, settings))) {
            onError(tr("Invalid settings callback method"));
            return false;
        }

        return true;
    default:
        return false;
    }
}

bool UrlCheckModel::initDecaptchaPlugin(const QString &pluginId) {
    if (m_decaptchaPlugin) {
        return true;
    }

    m_decaptchaPluginId = pluginId;
    m_decaptchaPlugin = DecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (!m_decaptchaPlugin) {
        Logger::log("UrlCheckModel::initDecaptchaPlugin(): No plugin found for " + pluginId);
        return false;
    }

    connect(m_decaptchaPlugin, SIGNAL(captchaResponse(QString, QString)),
            this, SLOT(onCaptchaResponse(QString, QString)));
    connect(m_decaptchaPlugin, SIGNAL(captchaResponseReported(QString)),
            this, SLOT(onCaptchaResponseReported(QString)));
    connect(m_decaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_decaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onDecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    return true;
}

bool UrlCheckModel::initRecaptchaPlugin(const QString &pluginId) {
    if (m_recaptchaPlugin) {
        return true;
    }

    m_recaptchaPluginId = pluginId;
    m_recaptchaPlugin = RecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (!m_recaptchaPlugin) {
        Logger::log("UrlCheckModel::initRecaptchaPlugin(): No plugin found for " + pluginId);
        return false;
    }

    connect(m_recaptchaPlugin, SIGNAL(captcha(int, QByteArray)), this, SLOT(onCaptchaReady(int, QByteArray)));
    connect(m_recaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_recaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onRecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    return true;
}

bool UrlCheckModel::initServicePlugin(const QString &url) {
    if (m_servicePlugin) {
        return true;
    }
    
    const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(url);

    if (!config) {
        Logger::log("UrlCheckModel::initServicePlugin(): No config found for " + url);
        return false;
    }

    m_servicePlugin = ServicePluginManager::instance()->createPluginByUrl(url, this);

    if (!m_servicePlugin) {
        Logger::log("UrlCheckModel::initServicePlugin(): No plugin found for " + url);
        return false;
    }

    m_servicePluginId = config->id();
    connect(m_servicePlugin, SIGNAL(captchaRequest(QString, int, QString, QByteArray)),
            this, SLOT(onCaptchaRequest(QString, int, QString, QByteArray)));
    connect(m_servicePlugin, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_servicePlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
            this, SLOT(onServiceSettingsRequest(QString, QVariantList, QByteArray)));
    connect(m_servicePlugin, SIGNAL(urlChecked(UrlResult)), this, SLOT(onUrlChecked(UrlResult)));
    connect(m_servicePlugin, SIGNAL(urlChecked(UrlResultList, QString)),
            this, SLOT(onUrlChecked(UrlResultList, QString)));
    return true;
}

void UrlCheckModel::clearPlugins() {
    if (m_decaptchaPlugin) {
        m_decaptchaPlugin->cancelCurrentOperation();
        m_decaptchaPlugin->deleteLater();
        m_decaptchaPlugin = 0;
        m_decaptchaPluginId = QString();
    }

    if (m_recaptchaPlugin) {
        m_recaptchaPlugin->cancelCurrentOperation();
        m_recaptchaPlugin->deleteLater();
        m_recaptchaPlugin = 0;
        m_recaptchaPluginId = QString();
    }

    if (m_servicePlugin) {
        m_servicePlugin->cancelCurrentOperation();
        m_servicePlugin->deleteLater();
        m_servicePlugin = 0;
        m_servicePluginId = QString();
    }
}

bool UrlCheckModel::isRedirect(const QString &url) const {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(url)) {
        if (config->id() != m_servicePluginId) {
            return true;
        }
    }

    return false;
}

void UrlCheckModel::followRedirect(const QString &url) {
    clearPlugins();

    if (!initServicePlugin(url)) {
        onError(tr("No service plugin found"));
        return;
    }

    m_servicePlugin->checkUrl(url, PluginSettings(m_servicePluginId).values());
}

void UrlCheckModel::next() {
    m_index++;
    emit progressChanged(progress());
    clearPlugins();

    if (m_index >= m_items.size()) {
        setStatus(Completed);
        return;
    }

    setStatus(Active);
    const QModelIndex idx = index(m_index, 0);
    const QString url = data(idx, UrlRole).toString();

    if (!initServicePlugin(url)) {
        onError(tr("No service plugin found"));
        return;
    }

    m_servicePlugin->checkUrl(url, PluginSettings(m_servicePluginId).values());
}

void UrlCheckModel::onUrlChecked(const UrlResult &result) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    if (isRedirect(result.url)) {
        Logger::log(QString("UrlCheckModel::onUrlChecked(): %1. Following redirect: %2").arg(m_items[m_index].url)
                .arg(result.url), Logger::MediumVerbosity);
        followRedirect(result.url);
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1 1 URL found").arg(m_items[m_index].url),
            Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = true;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    TransferModel::instance()->append(result);
    next();
}

void UrlCheckModel::onUrlChecked(const UrlResultList &results, const QString &packageName) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1. %2 URLs found")
            .arg(m_items[m_index].url).arg(results.size()), Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = !results.isEmpty();
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);

    if (!results.isEmpty()) {
        TransferModel::instance()->append(results, packageName);
    }

    next();
}

void UrlCheckModel::onCaptchaReady(int type, const QByteArray &data) {
    setCaptchaData(type, data);
    const QString pluginId = Settings::decaptchaPlugin();
    
    if (!pluginId.isEmpty()) {
        if (initDecaptchaPlugin(pluginId)) {
            m_reportCaptchaError = true;
            m_decaptchaPlugin->getCaptchaResponse(captchaType(), captchaData(), PluginSettings(pluginId).values());
            return;
        }
    }

    m_reportCaptchaError = false;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateCaptchaTimeout()));
    setStatus(AwaitingCaptchaResponse);
    captchaRequest(captchaType(), captchaData());
}

void UrlCheckModel::onCaptchaRequest(const QString &recaptchaPluginId, int captchaType, const QString &captchaKey,
        const QByteArray &callback) {
    Logger::log("UrlCheckModel::onCaptchaRequest()", Logger::MediumVerbosity);

    if (!initRecaptchaPlugin(recaptchaPluginId)) {
        onError(tr("No recaptcha plugin found"));
        return;
    }

    setStatus(Active);
    m_captchaType = captchaType;
    m_captchaKey = captchaKey;
    m_callback = callback;
    m_recaptchaPlugin->getCaptcha(captchaType, captchaKey, PluginSettings(recaptchaPluginId).values());
}

void UrlCheckModel::onCaptchaResponse(const QString &captchaId, const QString &response) {
    if (!m_servicePlugin) {
        onError(tr("No service plugin found"));
        return;
    }

    m_decaptchaId = captchaId;
    m_captchaResponse = response;

    if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QString,
                    captchaType() == CaptchaType::NoCaptcha ? QString::fromUtf8(captchaData()) : m_captchaChallenge),
                Q_ARG(QString, response))) {
        onError(tr("Invalid captcha callback method"));
    }

    clearCaptchaData();
}

void UrlCheckModel::onCaptchaResponseReported(const QString &) {
    if (!m_recaptchaPlugin) {
        onError(tr("No recaptcha plugin found"));
        return;
    }

    m_recaptchaPlugin->getCaptcha(captchaType(), m_captchaKey, PluginSettings(m_recaptchaPluginId).values());
}

void UrlCheckModel::onDecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    Logger::log("UrlCheckModel::onDecaptchaSettingsRequest()", Logger::MediumVerbosity);
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setStatus(AwaitingDecaptchaSettingsResponse);
    emit settingsRequest(title, settings);
}

void UrlCheckModel::onRecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    Logger::log("UrlCheckModel::onRecaptchaSettingsRequest()", Logger::MediumVerbosity);
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setStatus(AwaitingRecaptchaSettingsResponse);
    emit settingsRequest(title, settings);
}

void UrlCheckModel::onServiceSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    Logger::log("UrlCheckModel::onServiceSettingsRequest()", Logger::MediumVerbosity);
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    setStatus(AwaitingServiceSettingsResponse);
    emit settingsRequest(title, settings);
}

void UrlCheckModel::onError(const QString &errorString) {
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onError(): %1. Error: %2").arg(m_items[m_index].url).arg(errorString));
    m_items[m_index].checked = true;
    m_items[m_index].ok = false;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    next();
}
