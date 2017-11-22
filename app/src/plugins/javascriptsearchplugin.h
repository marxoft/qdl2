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

#ifndef JAVASCRIPTSEARCHPLUGIN_H
#define JAVASCRIPTSEARCHPLUGIN_H

#include "searchplugin.h"
#include <QPointer>
#include <QScriptEngine>

class JavaScriptSearchPlugin : public SearchPlugin
{
    Q_OBJECT

public:
    explicit JavaScriptSearchPlugin(const QScriptValue &plugin, QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void fetchMore(const QVariantMap &params);
    virtual void search(const QVariantMap &settings);
    
    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    bool init();
    
    QScriptValue m_plugin;
    QScriptValue m_callback;
    
    bool m_initted;
};

class JavaScriptSearchPluginSignaller : public QObject
{
    Q_OBJECT

public:
    explicit JavaScriptSearchPluginSignaller(QObject *parent = 0);

Q_SIGNALS:
    void error(const QString &errorString);
    void searchCompleted(const SearchResultList &results);
    void searchCompleted(const SearchResultList &results, const QVariantMap &nextParams);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
};

class JavaScriptSearchPluginFactory : public QObject, public SearchPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SearchPluginFactory)

public:
    explicit JavaScriptSearchPluginFactory(const QString &fileName, QScriptEngine *engine, QObject *parent = 0);

    virtual SearchPlugin* createPlugin(QObject *parent = 0);

private:
    bool init();

    QPointer<QScriptEngine> m_engine;

    QString m_fileName;

    QScriptValue m_constructor;

    bool m_initted;
};

#endif // JAVASCRIPTSEARCHPLUGIN_H
