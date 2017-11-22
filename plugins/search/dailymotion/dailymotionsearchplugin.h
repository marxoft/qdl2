/**
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

#ifndef DAILYMOTIONSEARCHPLUGIN_H
#define DAILYMOTIONSEARCHPLUGIN_H

#include "searchplugin.h"

namespace QDailymotion {
    class ResourcesRequest;
}

class DailymotionSearchPlugin : public SearchPlugin
{
    Q_OBJECT
    
public:
    explicit DailymotionSearchPlugin(QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void fetchMore(const QVariantMap &params);
    virtual void search(const QVariantMap &settings);

    void submitSettings(const QVariantMap &settings);

private Q_SLOTS:
    void onRequestFinished();
    
private:
    static const QString HTML;
    static const QString PLAYLIST_FIELDS;
    static const QString VIDEO_FIELDS;
        
    QDailymotion::ResourcesRequest* request();
    
    QDailymotion::ResourcesRequest *m_request;
    
    QVariantMap m_filters;
};

class DailymotionSearchPluginFactory : public QObject, public SearchPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SearchPluginFactory)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl2.DailymotionSearchPluginFactory")
#endif

public:
    virtual SearchPlugin* createPlugin(QObject *parent = 0);
};

#endif // DAILYMOTIONSEARCHPLUGIN_H
