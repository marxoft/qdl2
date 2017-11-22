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

#include "javascriptrecaptchaplugin.h"
#include "logger.h"
#include <QFile>

JavaScriptRecaptchaPlugin::JavaScriptRecaptchaPlugin(const QScriptValue &plugin, QObject *parent) :
    RecaptchaPlugin(parent),
    m_plugin(plugin),
    m_initted(false)
{
}

JavaScriptRecaptchaPlugin::~JavaScriptRecaptchaPlugin() {
    if (QScriptEngine *engine = m_plugin.engine()) {
        engine->collectGarbage();
    }
}

bool JavaScriptRecaptchaPlugin::init() {
    if (m_initted) {
        return true;
    }

    if (!m_plugin.property("cancelCurrentOperation").isFunction()) {
        Logger::log("JavaScriptRecaptchaPlugin::init(): No cancelCurrentOperation() function found");
        return false;
    }

    if (!m_plugin.property("getCaptcha").isFunction()) {
        Logger::log("JavaScriptRecaptchaPlugin::init(): No getCaptcha() function found");
        return false;
    }

    QObject *obj = m_plugin.toQObject();

    if ((!obj) || (!connect(obj, SIGNAL(captcha(int, QString)), this, SLOT(onCaptcha(int, QString))))
            || (!connect(obj, SIGNAL(error(QString)), this, SIGNAL(error(QString))))
            || (!connect(obj, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                    this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue))))) {
        Logger::log("JavaScriptRecaptchaPlugin::init(): Not a valid RecaptchaPlugin");
        return false;
    }

    Logger::log("JavaScriptRecaptchaPlugin::init(): RecaptchaPlugin initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}

bool JavaScriptRecaptchaPlugin::cancelCurrentOperation() {
    if (!init()) {
        return false;
    }

    return m_plugin.property("cancelCurrentOperation").call(m_plugin).toBool();
}

void JavaScriptRecaptchaPlugin::getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("getCaptcha").call(m_plugin, QScriptValueList() << captchaType
            << captchaKey << m_plugin.engine()->toScriptValue(settings));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptRecaptchaPlugin::getCaptcha(). Error calling getCaptcha(): " + errorString);
        emit error(tr("Error calling getCaptcha(): %1").arg(errorString));
    }
}

void JavaScriptRecaptchaPlugin::submitSettingsResponse(const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(m_plugin, QScriptValueList()
                << m_plugin.engine()->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptRecaptchaPlugin::submitSettingsResponse(): Error calling settings callback: "
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
                Logger::log("JavaScriptRecaptchaPlugin::submitSettingsResponse(): Error calling settings callback: "
                        + errorString);
                emit error(tr("Settings callback error: %1").arg(errorString));
            }
        }
        else {
            Logger::log(QString("JavaScriptRecaptchaPlugin::submitSettingsResponse(): %1 function not defined")
                    .arg(funcName));
            emit error(tr("%1 function not defined in the plugin").arg(funcName));
        }
    }
}

void JavaScriptRecaptchaPlugin::onCaptcha(int captchaType, const QString &captchaData) {
    emit captcha(captchaType, captchaData.toUtf8());
}

void JavaScriptRecaptchaPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
        const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

JavaScriptRecaptchaPluginSignaller::JavaScriptRecaptchaPluginSignaller(QObject *parent) :
    QObject(parent)
{
}

JavaScriptRecaptchaPluginFactory::JavaScriptRecaptchaPluginFactory(const QString &fileName, QScriptEngine *engine,
        QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_fileName(fileName),
    m_initted(false)
{
}

RecaptchaPlugin* JavaScriptRecaptchaPluginFactory::createPlugin(QObject *parent) {
    if (init()) {
        return new JavaScriptRecaptchaPlugin(m_constructor.construct(), parent);
    }

    return 0;
}

bool JavaScriptRecaptchaPluginFactory::init() {
    if (m_initted) {
        return true;
    }
    
    if (!m_engine) {
        Logger::log("JavaScriptRecaptchaPluginFactory::init(): No JavaScript engine");
        return false;
    }

    QFile file(m_fileName);

    if (!file.open(QFile::ReadOnly)) {
        Logger::log("JavaScriptRecaptchaPluginFactory::init(): Cannot read file: " + m_fileName);
        return false;
    }

    m_constructor = m_engine->evaluate(QString::fromUtf8(file.readAll()), m_fileName);
    file.close();

    if (!m_constructor.isFunction()) {
        if (m_constructor.isError()) {
            Logger::log("JavaScriptRecaptchaPluginFactory::init(): Error evaluating JavaScript file: "
                    + m_constructor.toString());
        }
        else {
            Logger::log("JavaScriptRecaptchaPluginFactory::init(): No constructor function");
        }

        return false;
    }

    Logger::log("JavaScriptRecaptchaPluginFactory::init(): Initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}
