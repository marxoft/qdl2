/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "javascriptsearchplugin.h"
#include "logger.h"
#include <QFile>

JavaScriptSearchPlugin::JavaScriptSearchPlugin(const QScriptValue &plugin, QObject *parent) :
    SearchPlugin(parent),
    m_plugin(plugin),
    m_initted(false)
{
}

JavaScriptSearchPlugin::~JavaScriptSearchPlugin() {
    if (QScriptEngine *engine = m_plugin.engine()) {
        engine->collectGarbage();
    }
}

bool JavaScriptSearchPlugin::init() {
    if (m_initted) {
        return true;
    }

    if (!m_plugin.property("cancelCurrentOperation").isFunction()) {
        Logger::log("JavaScriptSearchPlugin::init(): No cancelCurrentOperation() function found");
        return false;
    }

    if (!m_plugin.property("search").isFunction()) {
        Logger::log("JavaScriptSearchPlugin::init(): No search() function found");
        return false;
    }

    QObject *obj = m_plugin.toQObject();

    if ((!obj) || (!connect(obj, SIGNAL(error(QString)), this, SIGNAL(error(QString))))
            || (!connect(obj, SIGNAL(searchCompleted(SearchResultList)),
                    this, SIGNAL(searchCompleted(SearchResultList))))
            || (!connect(obj, SIGNAL(searchCompleted(SearchResultList, QVariantMap)),
                    this, SIGNAL(searchCompleted(SearchResultList, QVariantMap))))
            || (!connect(obj, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                    this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue))))) {
        Logger::log("JavaScriptSearchPlugin::init(): Not a valid SearchPlugin");
        return false;
    }

    Logger::log("JavaScriptSearchPlugin::init(): SearchPlugin initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}

bool JavaScriptSearchPlugin::cancelCurrentOperation() {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return false;
    }

    return m_plugin.property("cancelCurrentOperation").call(m_plugin).toBool();
}

void JavaScriptSearchPlugin::fetchMore(const QVariantMap &params) {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("fetchMore").call(m_plugin, QScriptValueList()
            << m_plugin.engine()->toScriptValue(params));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptSearchPlugin::search(). Error calling fetchMore(): " + errorString);
        emit error(tr("Error calling fetchMore(): %1").arg(errorString));
    }
}

void JavaScriptSearchPlugin::search(const QVariantMap &settings) {
    if (!init()) {
        error(tr("Plugin not initialized"));
        return;
    }

    const QScriptValue result = m_plugin.property("search").call(m_plugin, QScriptValueList()
            << m_plugin.engine()->toScriptValue(settings));

    if (result.isError()) {
        const QString errorString = result.toString();
        Logger::log("JavaScriptSearchPlugin::search(). Error calling search(): " + errorString);
        emit error(tr("Error calling search(): %1").arg(errorString));
    }
}

void JavaScriptSearchPlugin::submitSettingsResponse(const QVariantMap &settings) {
    if (!init()) {
        emit error(tr("Plugin not initialized"));
        return;
    }

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(m_plugin, QScriptValueList()
                << m_plugin.engine()->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptSearchPlugin::submitSettingsResponse(): Error calling settings callback: "
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
                Logger::log("JavaScriptSearchPlugin::submitSettingsResponse(): Error calling settings callback: "
                        + errorString);
                emit error(tr("Settings callback error: %1").arg(errorString));
            }
        }
        else {
            Logger::log(QString("JavaScriptSearchPlugin::submitSettingsResponse(): %1 function not defined")
                    .arg(funcName));
            emit error(tr("%1 function not defined in the plugin").arg(funcName));
        }
    }
}

void JavaScriptSearchPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
        const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

JavaScriptSearchPluginSignaller::JavaScriptSearchPluginSignaller(QObject *parent) :
    QObject(parent)
{
}

JavaScriptSearchPluginFactory::JavaScriptSearchPluginFactory(const QString &fileName, QScriptEngine *engine,
        QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_fileName(fileName),
    m_initted(false)
{
}

SearchPlugin* JavaScriptSearchPluginFactory::createPlugin(QObject *parent) {
    if (init()) {
        return new JavaScriptSearchPlugin(m_constructor.construct(), parent);
    }

    return 0;
}

bool JavaScriptSearchPluginFactory::init() {
    if (m_initted) {
        return true;
    }

    if (!m_engine) {
        Logger::log("JavaScriptSearchPluginFactory::init(): No JavaScript engine");
        return false;
    }

    QFile file(m_fileName);

    if (!file.open(QFile::ReadOnly)) {
        Logger::log("JavaScriptSearchPluginFactory::init(): Cannot read file: " + m_fileName);
        return false;
    }

    m_constructor = m_engine->evaluate(QString::fromUtf8(file.readAll()), m_fileName);
    file.close();

    if (!m_constructor.isFunction()) {
        if (m_constructor.isError()) {
            Logger::log("JavaScriptSearchPluginFactory::init(): Error evaluating JavaScript file: "
                    + m_constructor.toString());
        }
        else {
            Logger::log("JavaScriptSearchPluginFactory::init(): No constructor function");
        }

        return false;
    }

    Logger::log("JavaScriptSearchPluginFactory::init(): Initialized OK", Logger::HighVerbosity);
    m_initted = true;
    return true;
}
