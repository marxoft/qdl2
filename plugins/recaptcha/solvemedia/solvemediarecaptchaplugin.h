/**
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

#ifndef SOLVEMEDIARECAPTCHAPLUGIN_H
#define SOLVEMEDIARECAPTCHAPLUGIN_H

#include "recaptchaplugin.h"
#include <QPointer>

class SolveMediaRecaptchaPlugin : public RecaptchaPlugin
{
    Q_OBJECT

public:
    explicit SolveMediaRecaptchaPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &settings);

private Q_SLOTS:
    void onCaptchaDownloaded();
    void onCaptchaImageDownloaded();

Q_SIGNALS:
    void currentOperationCanceled();

private:
    QNetworkAccessManager* networkAccessManager();
    
    void downloadCaptchaImage(const QString &challenge);

    static const QString CAPTCHA_CHALLENGE_URL;
    static const QString CAPTCHA_IMAGE_URL;

    QPointer<QNetworkAccessManager> m_nam;

    bool m_ownManager;

    QString m_challenge;
};

class SolveMediaRecaptchaPluginFactory : public QObject, public RecaptchaPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaPluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.SolveMediaRecaptchaPluginFactory")
#endif

public:
    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0);
};

#endif // SOLVEMEDIARECAPTCHAPLUGIN_H
