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
#include <QPointer>
#include <QScriptEngine>

class JavaScriptRecaptchaPlugin : public RecaptchaPlugin
{
    Q_OBJECT

public:
    explicit JavaScriptRecaptchaPlugin(const QScriptValue &plugin, QObject *parent = 0);
    ~JavaScriptRecaptchaPlugin();

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &settings);

    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCaptcha(int captchaType, const QString &captchaData);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    bool init();
    
    QScriptValue m_plugin;
    QScriptValue m_callback;
    
    bool m_initted;
};

class JavaScriptRecaptchaPluginSignaller : public QObject
{
    Q_OBJECT

public:
    explicit JavaScriptRecaptchaPluginSignaller(QObject *parent = 0);

Q_SIGNALS:
    void captcha(int captchaType, const QString &captchaData);
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
};

class JavaScriptRecaptchaPluginFactory : public QObject, public RecaptchaPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaPluginFactory)

public:
    explicit JavaScriptRecaptchaPluginFactory(const QString &fileName, QScriptEngine *engine, QObject *parent = 0);

    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0);

private:
    bool init();

    QPointer<QScriptEngine> m_engine;

    QString m_fileName;

    QScriptValue m_constructor;

    bool m_initted;
};

#endif // JAVASCRIPTRECAPTCHAPLUGIN_H
