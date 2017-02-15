/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SEARCHPLUGINMANAGER_H
#define SEARCHPLUGINMANAGER_H

#include "searchplugin.h"
#include "searchpluginconfig.h"
#include <QDateTime>

struct SearchPluginPair
{
    SearchPluginPair(SearchPluginConfig* c, SearchPlugin* p) :
        config(c),
        plugin(p)
    {
    }

    SearchPluginConfig *config;
    SearchPlugin *plugin;
};

typedef QList<SearchPluginPair> SearchPluginList;

class SearchPluginManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    ~SearchPluginManager();

    static SearchPluginManager* instance();
    
    int count() const;

    SearchPluginList plugins() const;

public Q_SLOTS:
    SearchPluginConfig* getConfigById(const QString &id) const;

    SearchPlugin* getPluginById(const QString &id) const;

    SearchPlugin* createPluginById(const QString &id, QObject *parent = 0) const;

    int load();

Q_SIGNALS:
    void countChanged(int count);

private:
    SearchPluginManager();

    SearchPluginConfig* getConfigByFilePath(const QString &filePath) const;

    QNetworkAccessManager* networkAccessManager();

    static SearchPluginManager *self;

    QNetworkAccessManager *m_nam;

    QDateTime m_lastLoaded;

    SearchPluginList m_plugins;
};

#endif // SEARCHPLUGINMANAGER_H
