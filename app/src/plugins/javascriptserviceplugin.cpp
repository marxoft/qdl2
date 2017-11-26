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

#include "javascriptserviceplugin.h"
#include "logger.h"
#include <QFile>

JavaScriptServicePlugin::JavaScriptServicePlugin(const QScriptValue &plugin, QObject *parent) :
    ServicePlugin(parent),
    m_plugin(plugin),
    m_initted(false)
{
}

JavaScriptServicePlugin::~JavaScriptServicePlugin() {
    if (QScriptEngine *engine = m_plugin.engine()) {
        engine->collectGarbage();
    }
}

bool JavaScriptServicePlugin::init() {
    if (m_initted) {
        return true;
    }

    if (!m_plugin.property("cancelCurrentOperation").isFunction()) {
        Logger::log("JavaScriptServicePlugin::init(): No cancelCurrentOperation() function found");
        return false;
    }

    if (!m_plugin.property("checkUrl").isFunction()) {
        Logger::log("JavaScriptServicePlugin::init(): No checkUrl() function found");
        return false;
    }

    QObject *obj = m_plugin.toQObject();

    if ((!obj) || (!connect(obj, SIGNAL(captchaRequest(QString, int, QString, QScriptValue)),
                    this, SLOT(onCaptchaRequest(QString, int, QString, QScriptValue))))
            || (!connect(obj, SIGNAL(downloadRequest(QNetworkRequest, QString, QString)),
                    this, SLOT(onDownloadRequest(QNetworkRequest, QString, QString))))
            || (!connect(obj, SIGNAL(error(QString)), this, SIGNAL(error(QString))))
            || (!connect(obj, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                    this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue))))
            || (!connect(obj, SIGNAL(urlChecked(UrlResult)), this, SIGNAL(urlChecked(UrlResult))))
            || (!connect(obj, SIGNAL(urlChecked(UrlResultList, QString)),
                    this, SIGNAL(urlChecked(UrlResultList, QString))))
            || (!connect(obj, SIGNAL(waitRequest(int, bool)), this, SIGNAL(waitRequest(int, bool))))) {
        Logger::log("JavaScriptServicePlugin::init(): Not a valid ServicePlugin");
        return false;
    }

    Logger::log("JavaScriptServicePlugin::init(): ServicePlugin initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}

bool JavaScriptServicePlugin::cancelCurrentOperation() {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return false;
    }

    return m_plugin.property("cancelCurrentOperation").call(m_plugin).toBool();
}

void JavaScriptServicePlugin::checkUrl(const QString &url, const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("checkUrl").call(m_plugin, QScriptValueList() << url
            << m_plugin.engine()->toScriptValue(settings));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptServicePlugin::checkUrl(): Error calling checkUrl(): " + errorString);
        emit error(tr("Error calling checkUrl(): %1").arg(errorString));
    }
}

void JavaScriptServicePlugin::getDownloadRequest(const QString &url, const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("getDownloadRequest").call(m_plugin, QScriptValueList() << url
        << m_plugin.engine()->toScriptValue(settings));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptServicePlugin::getDownloadRequest(): Error calling getDownloadRequest(): "
                + errorString);
        emit error(tr("Error calling getDownloadRequest(): %1").arg(errorString));
    }
}

void JavaScriptServicePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(m_plugin, QScriptValueList() << challenge << response);

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptServicePlugin::submitCaptchaResponse(): Error calling captcha callback: "
                    + errorString);
            emit error(tr("Captcha callback error: %1").arg(errorString));
        }
    }
    else {
        const QString funcName = m_callback.toString();
        QScriptValue func = m_plugin.property(funcName);
        
        if (func.isFunction()) {
            const QScriptValue result = func.call(m_plugin, QScriptValueList() << challenge << response);
            
            if (result.isError()) {
                const QString errorString = result.toString();
                Logger::log("JavaScriptServicePlugin::submitCaptchaResponse(): Error calling captcha callback: "
                        + errorString);
                emit error(tr("Captcha callback error: %1").arg(errorString));
            }
        }
        else {
            Logger::log(QString("JavaScriptServicePlugin::submitCaptchaResponse(): %1 function not defined")
                    .arg(funcName));
            emit error(tr("%1 function not defined in the plugin").arg(funcName));
        }
    }
}

void JavaScriptServicePlugin::submitSettingsResponse(const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(m_plugin, QScriptValueList()
                << m_plugin.engine()->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptServicePlugin::submitSettingsResponse(): Error calling settings callback: "
                    + errorString);
            emit error(tr("Settings callback error: %1").arg(errorString));
        }
    }
    else {
        const QString funcName = m_callback.toString();
        QScriptValue func = m_plugin.property(funcName);

        if (func.isFunction()) {
            const QScriptValue result = func.call(m_plugin, QScriptValueList()
                    << m_plugin.engine()->toScriptValue(settings));

            if (result.isError()) {
                const QString errorString = result.toString();
                Logger::log("JavaScriptServicePlugin::submitSettingsResponse(): Error calling settings callback: "
                        + errorString);
                emit error(tr("Settings callback error: %1").arg(errorString));
            }
        }
        else {
            Logger::log(QString("JavaScriptServicePlugin::submitSettingsResponse(): %1 function not defined")
                    .arg(funcName));
            emit error(tr("%1 function not defined in the plugin").arg(funcName));
        }
    }
}

void JavaScriptServicePlugin::onCaptchaRequest(const QString &recaptchaPluginId, int captchaType,
        const QString &captchaKey, const QScriptValue &callback) {
    m_callback = callback;
    emit captchaRequest(recaptchaPluginId, captchaType, captchaKey, "submitCaptchaResponse");
}

void JavaScriptServicePlugin::onDownloadRequest(const QNetworkRequest &request, const QString &method,
        const QString &data) {
    emit downloadRequest(request, method.toUtf8(), data.toUtf8());
}

void JavaScriptServicePlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
        const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

JavaScriptServicePluginSignaller::JavaScriptServicePluginSignaller(QObject *parent) :
    QObject(parent)
{
}

JavaScriptServicePluginFactory::JavaScriptServicePluginFactory(const QString &fileName, QScriptEngine *engine,
        QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_fileName(fileName),
    m_initted(false)
{
}

ServicePlugin* JavaScriptServicePluginFactory::createPlugin(QObject *parent) {
    if (init()) {
        return new JavaScriptServicePlugin(m_constructor.construct(), parent);
    }

    return 0;
}

bool JavaScriptServicePluginFactory::init() {
    if (m_initted) {
        return true;
    }

    if (!m_engine) {
        Logger::log("JavaScriptServicePluginFactory::init(): No JavaScript engine");
        return false;
    }

    QFile file(m_fileName);

    if (!file.open(QFile::ReadOnly)) {
        Logger::log("JavaScriptServicePluginFactory::init(): Cannot read file: " + m_fileName);
        return false;
    }

    m_constructor = m_engine->evaluate(QString::fromUtf8(file.readAll()), m_fileName);
    file.close();

    if (!m_constructor.isFunction()) {
        if (m_constructor.isError()) {
            Logger::log("JavaScriptServicePluginFactory::init(): Error evaluating JavaScript file: "
                    + m_constructor.toString());
        }
        else {
            Logger::log("JavaScriptServicePluginFactory::init(): No constructor function");
        }

        return false;
    }

    Logger::log("JavaScriptServicePluginFactory::init(): Initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}
