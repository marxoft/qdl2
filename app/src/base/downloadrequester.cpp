/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "downloadrequester.h"
#include "captchatype.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "logger.h"
#include "pluginsettings.h"
#include "recaptchapluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "utils.h"
#include <QTimer>

DownloadRequester::DownloadRequester(QObject *parent) :
    QObject(parent),
    m_decaptchaPlugin(0),
    m_recaptchaPlugin(0),
    m_servicePlugin(0),
    m_timer(0),
    m_captchaType(CaptchaType::Unknown),
    m_reportCaptchaError(false),
    m_timeRemaining(CAPTCHA_TIMEOUT),
    m_status(Idle)
{
}

DownloadRequester::~DownloadRequester() {
    cancel();
}

int DownloadRequester::captchaType() const {
    return m_captchaType;
}

QString DownloadRequester::captchaTypeString() const {
    switch (captchaType()) {
    case CaptchaType::Image:
        return tr("Image");
    case CaptchaType::NoCaptcha:
        return tr("NoCaptcha");
    default:
        return tr("Unknown");
    }
}

QByteArray DownloadRequester::captchaData() const {
    return m_captchaData;
}

void DownloadRequester::setCaptchaData(int captchaType, const QByteArray &captchaData) {
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

void DownloadRequester::clearCaptchaData() {
    m_captchaType = CaptchaType::Unknown;
    m_captchaChallenge.clear();
    m_captchaData.clear();
}

int DownloadRequester::captchaTimeout() const {
    return status() == AwaitingCaptchaResponse ? m_timeRemaining : 0;
}

QString DownloadRequester::captchaTimeoutString() const {
    return Utils::formatMSecs(captchaTimeout());
}

void DownloadRequester::updateCaptchaTimeout() {
    m_timeRemaining -= m_timer->interval();
    emit captchaTimeoutChanged(m_timeRemaining);

    if (m_timeRemaining <= 0) {
        stopWaitTimer();
        onError(tr("No captcha response"));
    }
}

QVariantList DownloadRequester::requestedSettings() const {
    return m_requestedSettings;
}

void DownloadRequester::setRequestedSettings(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    m_requestedSettingsTitle = title;
    m_requestedSettings = settings;
    m_callback = callback;
}

void DownloadRequester::clearRequestedSettings() {
    m_requestedSettingsTitle.clear();
    m_requestedSettings.clear();
}

int DownloadRequester::requestedSettingsTimeout() const {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return m_timeRemaining;
    default:
        return 0;
    }
}

QString DownloadRequester::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString DownloadRequester::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

int DownloadRequester::waitTime() const {
    switch (status()) {
    case WaitingActive:
    case WaitingInactive:
        return m_timeRemaining;
    default:
        return 0;
    }
}

void DownloadRequester::updateRequestedSettingsTimeout() {
    m_timeRemaining -= m_timer->interval();
    emit requestedSettingsTimeoutChanged(m_timeRemaining);
    
    if (m_timeRemaining <= 0) {
        stopWaitTimer();
        onError(tr("No settings response"));
    }
}

DownloadRequester::Status DownloadRequester::status() const {
    return m_status;
}

void DownloadRequester::setStatus(DownloadRequester::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString DownloadRequester::statusString() const {
    switch (status()) {
    case Connecting:
        return tr("Connecting");
    case RetrievingCaptchaChallenge:
        return tr("Retrieving captcha challenge");
    case AwaitingCaptchaResponse:
        return tr("Awaiting captcha response: %1").arg(captchaTimeoutString());
    case RetrievingCaptchaResponse:
        return tr("Retrieving captcha response");
    case SubmittingCaptchaResponse:
        return tr("Submitting captcha response");
    case AwaitingDecaptchaSettingsResponse:
    case AwaitingRecaptchaSettingsResponse:
    case AwaitingServiceSettingsResponse:
        return tr("Awaiting settings response: %1").arg(requestedSettingsTimeoutString());
    case SubmittingDecaptchaSettingsResponse:
    case SubmittingRecaptchaSettingsResponse:
    case SubmittingServiceSettingsResponse:
        return tr("Submitting settings response");
    case WaitingActive:
        return tr("Waiting (active): %1").arg(waitTimeString());
    case WaitingInactive:
        return tr("Waiting (inactive): %2").arg(waitTimeString());
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    case Error:
        return tr("Error");
    default:
        return QString();
    }
}

QString DownloadRequester::waitTimeString() const {
    return Utils::formatMSecs(waitTime());
}

void DownloadRequester::updateWaitTime() {
    m_timeRemaining -= m_timer->interval();
    emit waitTimeChanged(m_timeRemaining);
    
    if (m_timeRemaining <= 0) {
        stopWaitTimer();
        
        if (status() == WaitingInactive) {
            getDownloadRequest();
        }
    }
}

void DownloadRequester::startWaitTimer(int msecs, const char *slot) {
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

void DownloadRequester::stopWaitTimer() {
    if (m_timer) {
        m_timer->stop();
    }
}

QString DownloadRequester::url() const {
    return m_url;
}

bool DownloadRequester::getDownloadRequest(const QString &url) {
    switch (status()) {
    case Idle:
    case Completed:
    case Canceled:
    case Error:
        break;
    default:
        return false;
    }

    if (ServicePlugin *plugin = servicePlugin(url)) {
        m_url = url;
        setStatus(Connecting);
        plugin->getDownloadRequest(url, PluginSettings(m_servicePluginId).values());
        return true;
    }

    onError(tr("No service plugin found"));
    return false;
}

void DownloadRequester::getDownloadRequest() {
    if (ServicePlugin *plugin = servicePlugin(url())) {
        plugin->getDownloadRequest(url(), PluginSettings(m_servicePluginId).values());
    }
    else {
        onError(tr("No service plugin found"));
    }
}

void DownloadRequester::cancel() {
    if (m_decaptchaPlugin) {
        m_decaptchaPlugin->cancelCurrentOperation();
    }

    if (m_recaptchaPlugin) {
        m_recaptchaPlugin->cancelCurrentOperation();
    }

    if (m_servicePlugin) {
        m_servicePlugin->cancelCurrentOperation();
    }

    setStatus(Canceled);
}

bool DownloadRequester::submitCaptchaResponse(const QString &response) {
    if (status() != AwaitingCaptchaResponse) {
        return false;
    }

    setStatus(SubmittingCaptchaResponse);
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

bool DownloadRequester::submitSettingsResponse(const QVariantMap &settings) {
    switch (status()) {
    case AwaitingDecaptchaSettingsResponse:
        setStatus(SubmittingDecaptchaSettingsResponse);
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
        setStatus(SubmittingRecaptchaSettingsResponse);
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
        setStatus(SubmittingServiceSettingsResponse);
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

DecaptchaPlugin* DownloadRequester::decaptchaPlugin(const QString &pluginId) {
    if ((pluginId == m_decaptchaPluginId) && (m_decaptchaPlugin)) {
        return m_decaptchaPlugin;
    }

    if (m_decaptchaPlugin) {
        delete m_decaptchaPlugin;
    }

    m_decaptchaPluginId = pluginId;
    m_decaptchaPlugin = DecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (m_decaptchaPlugin) {
        connect(m_decaptchaPlugin, SIGNAL(captchaResponse(QString, QString)),
                this, SLOT(onCaptchaResponse(QString, QString)));
        connect(m_decaptchaPlugin, SIGNAL(captchaResponseReported(QString)),
                this, SLOT(onCaptchaResponseReported(QString)));
        connect(m_decaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onDecaptchaError(QString)));
        connect(m_decaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
                this, SLOT(onDecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    }

    return m_decaptchaPlugin;
}

RecaptchaPlugin* DownloadRequester::recaptchaPlugin(const QString &pluginId) {
    if ((pluginId == m_recaptchaPluginId) && (m_recaptchaPlugin)) {
        return m_recaptchaPlugin;
    }

    if (m_recaptchaPlugin) {
        delete m_recaptchaPlugin;
    }

    m_recaptchaPluginId = pluginId;
    m_recaptchaPlugin = RecaptchaPluginManager::instance()->createPluginById(pluginId, this);

    if (m_recaptchaPlugin) {
        connect(m_recaptchaPlugin, SIGNAL(captcha(int, QByteArray)), this, SLOT(onCaptchaReady(int, QByteArray)));
        connect(m_recaptchaPlugin, SIGNAL(error(QString)), this, SLOT(onRecaptchaError(QString)));
        connect(m_recaptchaPlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
                this, SLOT(onRecaptchaSettingsRequest(QString, QVariantList, QByteArray)));
    }

    return m_recaptchaPlugin;
}

ServicePlugin* DownloadRequester::servicePlugin(const QString &url) {
    const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigByUrl(url);

    if (!config) {
        if (m_servicePlugin) {
            delete m_servicePlugin;
            m_servicePlugin = 0;
        }

        return m_servicePlugin;
    }

    m_servicePluginId = config->id();
    m_servicePlugin = ServicePluginManager::instance()->createPluginByUrl(url, this);

    if (m_servicePlugin) {
        connect(m_servicePlugin, SIGNAL(captchaRequest(QString, int, QString, QByteArray)),
                this, SLOT(onCaptchaRequest(QString, int, QString, QByteArray)));
        connect(m_servicePlugin, SIGNAL(downloadRequest(QNetworkRequest, QByteArray, QByteArray)),
                this, SLOT(onDownloadRequest(QNetworkRequest, QByteArray, QByteArray)));
        connect(m_servicePlugin, SIGNAL(error(QString)), this, SLOT(onServiceError(QString)));
        connect(m_servicePlugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
                this, SLOT(onServiceSettingsRequest(QString, QVariantList, QByteArray)));
        connect(m_servicePlugin, SIGNAL(waitRequest(int, bool)), this, SLOT(onWaitRequest(int, bool)));
    }

    return m_servicePlugin;
}

void DownloadRequester::onDownloadRequest(const QNetworkRequest &request, const QByteArray &method,
        const QByteArray &data) {
    setStatus(Completed);
    emit downloadRequest(request, method, data);
}

void DownloadRequester::onCaptchaReady(int type, const QByteArray &data) {
    setCaptchaData(type, data);
    const QString pluginId = Settings::decaptchaPlugin();
    
    if (!pluginId.isEmpty()) {
        if (DecaptchaPlugin *plugin = decaptchaPlugin(pluginId)) {
            m_reportCaptchaError = true;
            setStatus(RetrievingCaptchaResponse);
            plugin->getCaptchaResponse(captchaType(), captchaData(), PluginSettings(pluginId).values());
            return;
        }
    }

    m_reportCaptchaError = false;
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateCaptchaTimeout()));
    emit captchaTimeoutChanged(CAPTCHA_TIMEOUT);
    setStatus(AwaitingCaptchaResponse);
    emit captchaRequest(captchaType(), captchaData());
}

void DownloadRequester::onCaptchaRequest(const QString &recaptchaPluginId, int captchaType, const QString &captchaKey,
        const QByteArray &callback) {
    if (RecaptchaPlugin *plugin = recaptchaPlugin(recaptchaPluginId)) {
        m_captchaType = captchaType;
        m_captchaKey = captchaKey;
        m_callback = callback;
        setStatus(RetrievingCaptchaChallenge);
        plugin->getCaptcha(captchaType, captchaKey, PluginSettings(recaptchaPluginId).values());
    }
    else {
        onError(tr("No recaptcha plugin found"));
    }
}

void DownloadRequester::onCaptchaResponse(const QString &captchaId, const QString &response) {
    if (!m_servicePlugin) {
        onError(tr("No service plugin found"));
        return;
    }

    m_decaptchaId = captchaId;
    m_captchaResponse = response;
    setStatus(SubmittingCaptchaResponse);

    if (!QMetaObject::invokeMethod(m_servicePlugin, m_callback, Q_ARG(QString,
                    captchaType() == CaptchaType::NoCaptcha ? QString::fromUtf8(captchaData()) : m_captchaChallenge),
                Q_ARG(QString, response))) {
        onError(tr("Invalid captcha callback method"));
    }

    clearCaptchaData();
}

void DownloadRequester::onCaptchaResponseReported(const QString &) {
    if (!m_recaptchaPlugin) {
        onError(tr("No recaptcha plugin found"));
        return;
    }

    setStatus(RetrievingCaptchaChallenge);
    m_recaptchaPlugin->getCaptcha(captchaType(), m_captchaKey, PluginSettings(m_recaptchaPluginId).values());
}

void DownloadRequester::onDecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    emit requestedSettingsTimeoutChanged(CAPTCHA_TIMEOUT);
    setStatus(AwaitingDecaptchaSettingsResponse);
    emit settingsRequest(title, settings);
}

void DownloadRequester::onRecaptchaSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    emit requestedSettingsTimeoutChanged(CAPTCHA_TIMEOUT);
    setStatus(AwaitingRecaptchaSettingsResponse);
    emit settingsRequest(title, settings);
}

void DownloadRequester::onServiceSettingsRequest(const QString &title, const QVariantList &settings,
        const QByteArray &callback) {
    setRequestedSettings(title, settings, callback);
    startWaitTimer(CAPTCHA_TIMEOUT, SLOT(updateRequestedSettingsTimeout()));
    emit requestedSettingsTimeoutChanged(CAPTCHA_TIMEOUT);
    setStatus(AwaitingServiceSettingsResponse);
    emit settingsRequest(title, settings);
}

void DownloadRequester::onDecaptchaError(const QString &errorString) {
    onError(tr("Decaptcha plugin error - %1").arg(errorString));
}

void DownloadRequester::onRecaptchaError(const QString &errorString) {
    onError(tr("Recaptcha plugin error - %1").arg(errorString));
}

void DownloadRequester::onServiceError(const QString &errorString) {
    onError(tr("Service plugin error - %1").arg(errorString));
}

void DownloadRequester::onError(const QString &errorString) {
    Logger::log("DownloadRequester::onError(): " + errorString);
    setStatus(Error);
    emit error(errorString);
}

void DownloadRequester::onWaitRequest(int msecs, bool isLongDelay) {
    startWaitTimer(msecs, SLOT(updateWaitTime()));
    emit waitTimeChanged(msecs);

    if (isLongDelay) {
        setStatus(WaitingInactive);
    }
    else {
        setStatus(WaitingActive);
    }
}
