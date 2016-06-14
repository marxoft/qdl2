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

#ifndef JAVASCRIPTRECAPTCHAPLUGIN_H
#define JAVASCRIPTRECAPTCHAPLUGIN_H

#include "recaptchaplugin.h"
#include "javascriptpluginglobalobject.h"

class QScriptEngine;

class JavaScriptRecaptchaPlugin : public RecaptchaPlugin
{
    Q_OBJECT

    Q_INTERFACES(RecaptchaPlugin)

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptRecaptchaPlugin(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptcha(const QString &captchaKey);

    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCaptcha(const QString &challenge, const QString &imageData);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    void initEngine();
    
    JavaScriptPluginGlobalObject *m_global;
    QScriptEngine *m_engine;
    QPointer<QNetworkAccessManager> m_nam;

    QString m_fileName;
    QString m_id;

    QScriptValue m_callback;
    
    bool m_evaluated;
};

class JavaScriptRecaptchaPluginGlobalObject : public JavaScriptPluginGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptRecaptchaPluginGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void captcha(const QString &challenge, const QString &imageData);
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
};

#endif // JAVASCRIPTRECAPTCHAPLUGIN_H
