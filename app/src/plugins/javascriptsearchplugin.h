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
#include "javascriptpluginglobalobject.h"
#include <QScriptable>

class JavaScriptSearchPlugin : public SearchPlugin
{
    Q_OBJECT

    Q_INTERFACES(SearchPlugin)

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptSearchPlugin(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual SearchPlugin* createPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);
    
public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void fetchMore(const QVariantMap &params);
    virtual void search();
    
    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
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

class JavaScriptSearchPluginGlobalObject : public JavaScriptPluginGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptSearchPluginGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void error(const QString &errorString);
    void searchCompleted(const SearchResultList &results);
    void searchCompleted(const SearchResultList &results, const QVariantMap &nextParams);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    static QScriptValue newSearchResult(QScriptContext *context, QScriptEngine *engine);
};

class JavaScriptSearchResult : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString url READ url WRITE setUrl)

public:
    explicit JavaScriptSearchResult(QObject *parent = 0);
    
    QString name() const;
    void setName(const QString &n);
    
    QString description() const;
    void setDescription(const QString &d);
    
    QString url() const;
    void setUrl(const QString &u);
};

Q_DECLARE_METATYPE(SearchResult*)

#endif // JAVASCRIPTSEARCHPLUGIN_H
