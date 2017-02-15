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
#include "pluginsettings.h"
#include <QFile>
#include <QNetworkAccessManager>

JavaScriptSearchPlugin::JavaScriptSearchPlugin(const QString &id, const QString &fileName, QObject *parent) :
    SearchPlugin(parent),
    m_global(0),
    m_engine(0),
    m_nam(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptSearchPlugin::fileName() const {
    return m_fileName;
}

QString JavaScriptSearchPlugin::id() const {
    return m_id;
}      

SearchPlugin* JavaScriptSearchPlugin::createPlugin(QObject *parent) {
    return new JavaScriptSearchPlugin(id(), fileName(), parent);
}

void JavaScriptSearchPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (manager) {
        m_nam = manager;
        
        if (m_global) {
            m_global->setNetworkAccessManager(manager);
        }
    }
}

void JavaScriptSearchPlugin::initEngine() {
    if (m_evaluated) {
        return;
    }
    
    if (!m_engine) {
        m_engine = new QScriptEngine(this);
    }

    QFile file(fileName());
    
    if (file.open(QFile::ReadOnly)) {
        const QScriptValue result = m_engine->evaluate(file.readAll(), fileName());
        file.close();
        
        if (result.isError()) {
            Logger::log("JavaScriptSearchPlugin::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptSearchPlugin::initEngine(): JavaScript file evaluated OK", Logger::HighVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptSearchPluginGlobalObject(m_engine);
        
        if (m_nam) {
            m_global->setNetworkAccessManager(m_nam);
        }
        
        connect(m_global, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
        connect(m_global, SIGNAL(searchCompleted(SearchResultList)), this, SIGNAL(searchCompleted(SearchResultList)));
        connect(m_global, SIGNAL(searchCompleted(SearchResultList, QVariantMap)),
                this, SIGNAL(searchCompleted(SearchResultList, QVariantMap)));
        connect(m_global, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue)));
        
        m_engine->installTranslatorFunctions();
        m_engine->globalObject().setProperty("settings", m_engine->newQObject(new PluginSettings(id(), m_engine)));
    }
    else {
        Logger::log("JavaScriptSearchPlugin::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptSearchPlugin::cancelCurrentOperation() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancelCurrentOperation").call(QScriptValue()).toBool();
}

void JavaScriptSearchPlugin::fetchMore(const QVariantMap &params) {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("fetchMore");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << m_engine->toScriptValue(params));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptSearchPlugin::search(). Error calling fetchMore(): " + errorString);
            emit error(tr("Error calling fetchMore(): %1").arg(errorString));
        }
    }
    else {
        Logger::log("JavaScriptSearchPlugin::fetchMore(). fetchMore() function not defined");
        emit error(tr("fetchMore() function not defined"));
    }
}

void JavaScriptSearchPlugin::search() {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("search");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue());

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptSearchPlugin::search(). Error calling search(): " + errorString);
            emit error(tr("Error calling search(): %1").arg(errorString));
        }
    }
    else {
        Logger::log("JavaScriptSearchPlugin::search(). search() function not defined");
        emit error(tr("search() function not defined"));
    }
}

void JavaScriptSearchPlugin::submitSettingsResponse(const QVariantMap &settings) {
    initEngine();

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(QScriptValue(), QScriptValueList()
                                                    << m_engine->toScriptValue(settings));

        if (result.isError()) {
            emit error(tr("Settings callback error: %1").arg(result.toString()));
        }
    }
    else {
        QScriptValue func = m_engine->globalObject().property(m_callback.toString());

        if (func.isFunction()) {
            const QScriptValue result = func.call(QScriptValue(), QScriptValueList()
                                                  << m_engine->toScriptValue(settings));

            if (result.isError()) {
                emit error(tr("Error calling %1: %1").arg(m_callback.toString()).arg(result.toString()));
            }
        }
        else {
            emit error(tr("%1 function not defined in the plugin").arg(m_callback.toString()));
        }
    }
}

void JavaScriptSearchPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
                                                const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

JavaScriptSearchPluginGlobalObject::JavaScriptSearchPluginGlobalObject(QScriptEngine *engine) :
    JavaScriptPluginGlobalObject(engine)
{
    QScriptValue result = engine->newQObject(new JavaScriptSearchResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<SearchResult>(), result);
    engine->setDefaultPrototype(qMetaTypeId<SearchResult*>(), result);
    engine->globalObject().setProperty("SearchResult", engine->newFunction(newSearchResult));
    qScriptRegisterSequenceMetaType<SearchResultList>(engine);
}

QScriptValue JavaScriptSearchPluginGlobalObject::newSearchResult(QScriptContext *context, QScriptEngine *engine) {
    switch (context->argumentCount()) {
    case 0:
        return engine->toScriptValue(SearchResult());
    case 3:
        return engine->toScriptValue(SearchResult(context->argument(0).toString(), context->argument(1).toString(),
                                                  context->argument(2).toString()));
    default:
        return context->throwError(QScriptContext::SyntaxError,
                                   QObject::tr("SearchResult constructor requires either 0 or 3 arguments."));
    }
}

JavaScriptSearchResult::JavaScriptSearchResult(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptSearchResult::name() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->name;
    }
    
    return QString();
}

void JavaScriptSearchResult::setName(const QString &n) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->name = n;
    }
}

QString JavaScriptSearchResult::description() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->description;
    }
    
    return QString();
}

void JavaScriptSearchResult::setDescription(const QString &d) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->description = d;
    }
}

QString JavaScriptSearchResult::url() const {
    if (const SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        return result->url;
    }
    
    return QString();
}

void JavaScriptSearchResult::setUrl(const QString &u) {
    if (SearchResult* result = qscriptvalue_cast<SearchResult*>(thisObject())) {
        result->url = u;
    }
}
