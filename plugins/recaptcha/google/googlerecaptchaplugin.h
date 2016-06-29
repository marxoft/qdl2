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

#ifndef GOOGLERECAPTCHAPLUGIN_H
#define GOOGLERECAPTCHAPLUGIN_H

#include "recaptchaplugin.h"
#include <QPointer>

class GoogleRecaptchaPlugin : public RecaptchaPlugin
{
    Q_OBJECT

    Q_INTERFACES(RecaptchaPlugin)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.GoogleRecaptchaPlugin")
#endif

public:
    explicit GoogleRecaptchaPlugin(QObject *parent = 0);
    
    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0);

    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptcha(const QString &captchaKey);

private Q_SLOTS:
    void onCaptchaDownloaded();
    void onCaptchaImageDownloaded();

Q_SIGNALS:
    void currentOperationCanceled();

private:
    QNetworkAccessManager* networkAccessManager();
    
    void downloadCaptchaImage(const QString &challenge);

    QPointer<QNetworkAccessManager> m_nam;

    bool m_ownManager;

    QString m_challenge;
};

#endif // GOOGLERECAPTCHAPLUGIN_H
