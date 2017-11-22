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

#include "javascriptdecaptchaplugin.h"
#include "logger.h"
#include <QFile>

JavaScriptDecaptchaPlugin::JavaScriptDecaptchaPlugin(const QScriptValue &plugin, QObject *parent) :
    DecaptchaPlugin(parent),
    m_plugin(plugin),
    m_initted(false)
{
}

JavaScriptDecaptchaPlugin::~JavaScriptDecaptchaPlugin() {
    if (QScriptEngine *engine = m_plugin.engine()) {
        engine->collectGarbage();
    }
}

bool JavaScriptDecaptchaPlugin::init() {
    if (m_initted) {
        return true;
    }

    if (!m_plugin.property("cancelCurrentOperation").isFunction()) {
        Logger::log("JavaScriptDecaptchaPlugin::init(): No cancelCurrentOperation() function found");
        return false;
    }

    if (!m_plugin.property("getCaptchaResponse").isFunction()) {
        Logger::log("JavaScriptDecaptchaPlugin::init(): No getCaptchaResponse() function found");
        return false;
    }

    if (!m_plugin.property("reportCaptchaResponse").isFunction()) {
        Logger::log("JavaScriptDecaptchaPlugin::init(): No reportCaptchaResponse() function found");
        return false;
    }

    QObject *obj = m_plugin.toQObject();

    if ((!obj) || (!connect(obj, SIGNAL(captchaResponse(QString, QString)),
                    this, SIGNAL(captchaResponse(QString, QString))))
            || (!connect(obj, SIGNAL(captchaResponseReported(QString)),
                    this, SIGNAL(captchaResponseReported(QString))))
            || (!connect(obj, SIGNAL(error(QString)), this, SIGNAL(error(QString))))
            || (!connect(obj, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                    this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue))))) {
        Logger::log("JavaScriptDecaptchaPlugin::init(): Not a valid DecaptchaPlugin");
        return false;
    }

    Logger::log("JavaScriptDecaptchaPlugin::init(): DecaptchaPlugin initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}

bool JavaScriptDecaptchaPlugin::cancelCurrentOperation() {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return false;
    }

    return m_plugin.property("cancelCurrentOperation").call(m_plugin).toBool();
}

void JavaScriptDecaptchaPlugin::getCaptchaResponse(int captchaType, const QByteArray &captchaData,
        const QVariantMap &settings) {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return;
    }
    
    const QScriptValue result = m_plugin.property("getCaptchaResponse").call(m_plugin, QScriptValueList()
            << captchaType << QString::fromUtf8(captchaData) << m_plugin.engine()->toScriptValue(settings));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptDecaptchaPlugin::getCaptchaResponse(). Error calling getCaptchaResponse(): "
                    + errorString);
        emit error(tr("Error calling getCaptchaResponse(): %1").arg(errorString));
    }
}

void JavaScriptDecaptchaPlugin::reportCaptchaResponse(const QString &captchaId) {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("reportCaptchaResponse").call(m_plugin, QScriptValueList()
            << captchaId);

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptDecaptchaPlugin::reportCaptchaResponse(). Error calling reportCaptchaResponse(): "
                    + errorString);
        emit error(tr("Error calling reportCaptchaResponse(): %1").arg(errorString));
    }
}

void JavaScriptDecaptchaPlugin::submitSettingsResponse(const QVariantMap &settings) {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return;
    }

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(m_plugin, QScriptValueList()
                << m_plugin.engine()->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptDecaptchaPlugin::submitSettingsResponse(): Error calling settings callback: "
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
                Logger::log("JavaScriptDecaptchaPlugin::submitSettingsResponse(): Error calling settings callback: "
                        + errorString);
                emit error(tr("Settings callback error: %1").arg(errorString));
            }
        }
        else {
            Logger::log(QString("JavaScriptDecaptchaPlugin::submitSettingsResponse(): %1 function not defined")
                    .arg(funcName));
            emit error(tr("%1 function not defined in the plugin").arg(funcName));
        }
    }
}

void JavaScriptDecaptchaPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
        const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsRepsonse");
}

JavaScriptDecaptchaPluginSignaller::JavaScriptDecaptchaPluginSignaller(QObject *parent) :
    QObject(parent)
{
}

JavaScriptDecaptchaPluginFactory::JavaScriptDecaptchaPluginFactory(const QString &fileName, QScriptEngine *engine,
        QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_fileName(fileName),
    m_initted(false)
{
}

DecaptchaPlugin* JavaScriptDecaptchaPluginFactory::createPlugin(QObject *parent) {
    if (init()) {
        return new JavaScriptDecaptchaPlugin(m_constructor.construct(), parent);
    }

    return 0;
}

bool JavaScriptDecaptchaPluginFactory::init() {
    if (m_initted) {
        return true;
    }
    
    if (!m_engine) {
        Logger::log("JavaScriptDecaptchaPluginFactory::init(): No JavaScript engine");
        return false;
    }

    QFile file(m_fileName);

    if (!file.open(QFile::ReadOnly)) {
        Logger::log("JavaScriptDecaptchaPluginFactory::init(): Cannot read file: " + m_fileName);
        return false;
    }

    m_constructor = m_engine->evaluate(QString::fromUtf8(file.readAll()), m_fileName);
    file.close();

    if (!m_constructor.isFunction()) {
        if (m_constructor.isError()) {
            Logger::log("JavaScriptDecaptchaPluginFactory::init(): Error evaluating JavaScript file: "
                    + m_constructor.toString());
        }
        else {
            Logger::log("JavaScriptDecaptchaPluginFactory::init(): No constructor function");
        }

        return false;
    }

    Logger::log("JavaScriptDecaptchaPluginFactory::init(): Initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}
