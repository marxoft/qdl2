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

#ifndef DEATHBYCAPTCHAPLUGIN_H
#define DEATHBYCAPTCHAPLUGIN_H

#include "decaptchaplugin.h"
#include <QUrl>

class DeathByCaptchaPlugin : public DecaptchaPlugin
{
    Q_OBJECT
    
    Q_INTERFACES(DecaptchaPlugin)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.DeathByCaptchaPlugin")
#endif

public:
    explicit DeathByCaptchaPlugin(QObject *parent = 0);
    
    virtual DecaptchaPlugin* createPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();

    virtual void getCaptchaResponse(const QImage &image);
    virtual void reportCaptchaResponse(const QString &captchaId);

    void setLogin(const QVariantMap &login);

private Q_SLOTS:
    void checkCaptchaResponse();
    void checkCaptchaStatus();
    void checkCaptchaStatusResponse();
    void checkCaptchaReport();

Q_SIGNALS:
    void currentOperationCanceled();

private:
    QNetworkAccessManager* networkAccessManager();

    void fetchCaptchaResponse(const QByteArray &imageData);

    static QString CAPTCHA_URL;
    static QString REPORT_URL;
    static QString CONFIG_FILE;

    QNetworkAccessManager *m_nam;

    QByteArray m_imageData;
    
    QUrl m_statusUrl;
    QString m_captchaId;
    QString m_username;
    QString m_password;

    bool m_ownManager;
};

#endif // DEATHBYCAPTCHAPLUGIN_H
