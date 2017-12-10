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

#ifndef DOWNLOADREQUESTER_H
#define DOWNLOADREQUESTER_H

#include "urlresult.h"
#include <QObject>
#include <QVariantList>

class DecaptchaPlugin;
class RecaptchaPlugin;
class ServicePlugin;
class QNetworkRequest;
class QTimer;

class DownloadRequester : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int captchaType READ captchaType NOTIFY statusChanged)
    Q_PROPERTY(QByteArray captchaData READ captchaData NOTIFY statusChanged)
    Q_PROPERTY(int captchaTimeout READ captchaTimeout NOTIFY captchaTimeoutChanged)
    Q_PROPERTY(QString captchaTimeoutString READ captchaTimeoutString NOTIFY captchaTimeoutChanged)
    Q_PROPERTY(QVariantList requestedSettings READ requestedSettings NOTIFY statusChanged)
    Q_PROPERTY(int requestedSettingsTimeout READ requestedSettingsTimeout NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTimeoutString READ requestedSettingsTimeoutString
               NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTitle READ requestedSettingsTitle NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(int waitTime READ waitTime NOTIFY waitTimeChanged)
    Q_PROPERTY(QString waitTimeString READ waitTimeString NOTIFY waitTimeChanged)

    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Connecting,
        RetrievingCaptchaChallenge,
        AwaitingCaptchaResponse,
        RetrievingCaptchaResponse,
        SubmittingCaptchaResponse,
        ReportingCaptchaResponse,
        AwaitingDecaptchaSettingsResponse,
        AwaitingRecaptchaSettingsResponse,
        AwaitingServiceSettingsResponse,
        SubmittingDecaptchaSettingsResponse,
        SubmittingRecaptchaSettingsResponse,
        SubmittingServiceSettingsResponse,
        WaitingActive,
        WaitingInactive,
        Completed,
        Canceled,
        Error
    };

    explicit DownloadRequester(QObject *parent = 0);
    ~DownloadRequester();

    int captchaType() const;
    QString captchaTypeString() const;
    QByteArray captchaData() const;
    int captchaTimeout() const;
    QString captchaTimeoutString() const;

    QVariantList requestedSettings() const;
    int requestedSettingsTimeout() const;
    QString requestedSettingsTimeoutString() const;
    QString requestedSettingsTitle() const;

    Status status() const;
    QString statusString() const;

    QString url() const;

    int waitTime() const;
    QString waitTimeString() const;

public Q_SLOTS:
    bool getDownloadRequest(const QString &url);

    bool submitCaptchaResponse(const QString &response);
    bool submitSettingsResponse(const QVariantMap &settings);

    void cancel();

private Q_SLOTS:
    void getDownloadRequest();

    void updateCaptchaTimeout();
    void updateRequestedSettingsTimeout();
    void updateWaitTime();
    
    void onDownloadRequest(const QNetworkRequest &request, const QByteArray &method, const QByteArray &data);

    void onCaptchaReady(int captchaType, const QByteArray &captchaData);
    void onCaptchaRequest(const QString &recaptchaPluginId, int captchaType, const QString &captchaKey,
            const QByteArray &callback); 
    void onCaptchaResponse(const QString &captchaId, const QString &response);
    void onCaptchaResponseReported(const QString &captchaId);

    void onDecaptchaSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
    void onRecaptchaSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
    void onServiceSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);

    void onDecaptchaError(const QString &errorString);
    void onRecaptchaError(const QString &errorString);
    void onServiceError(const QString &errorString);
    void onError(const QString &errorString);

    void onWaitRequest(int msecs, bool isLongDelay);

Q_SIGNALS:
    void captchaRequest(int captchaType, const QByteArray &captchaData);
    void captchaTimeoutChanged(int timeout);
    void downloadRequest(const QNetworkRequest &request, const QByteArray &method, const QByteArray &data);
    void error(const QString &errorString);
    void requestedSettingsTimeoutChanged(int timeout);
    void settingsRequest(const QString &title, const QVariantList &settings);
    void statusChanged(DownloadRequester::Status status);
    void waitTimeChanged(int wait);

private:
    void setCaptchaData(int captchaType, const QByteArray &captchaData);
    void clearCaptchaData();
    
    void setRequestedSettings(const QString &title, const QVariantList &settings, const QByteArray &callback);
    void clearRequestedSettings();
    
    void startWaitTimer(int msecs, const char *slot);
    void stopWaitTimer();

    void setStatus(Status s);

    DecaptchaPlugin* decaptchaPlugin(const QString &pluginId);
    RecaptchaPlugin* recaptchaPlugin(const QString &pluginId);
    ServicePlugin* servicePlugin(const QString &url);

    DecaptchaPlugin *m_decaptchaPlugin;
    RecaptchaPlugin *m_recaptchaPlugin;
    ServicePlugin *m_servicePlugin;
    QTimer *m_timer;

    QString m_servicePluginId;
    QString m_recaptchaPluginId;
    QString m_decaptchaPluginId;
    
    int m_captchaType;
    QByteArray m_captchaData;
    QString m_captchaChallenge;
    QString m_captchaResponse;
    QString m_captchaKey;
    QString m_decaptchaId;
    bool m_reportCaptchaError;

    QString m_requestedSettingsTitle;
    QVariantList m_requestedSettings;

    QByteArray m_callback;

    int m_timeRemaining;

    Status m_status;

    QString m_url;
};

#endif // DOWNLOADREQUESTER_H
