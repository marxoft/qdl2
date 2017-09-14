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

#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H

#include "searchplugin.h"

class QHttpRequest;
class QHttpResponse;

class SearchServer : public QObject
{
    Q_OBJECT

public:
    explicit SearchServer(QObject *parent = 0);

    bool handleRequest(QHttpRequest *request, QHttpResponse *response);

private Q_SLOTS:
    void onSearchCompleted(const SearchResultList &results);
    void onSearchCompleted(const SearchResultList &results, const QVariantMap &nextParams);
    void onSearchError(const QString &errorString);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
    void onResponseDone();

private:
    void addResponse(SearchPlugin *plugin, QHttpResponse *response);
    QHttpResponse* getResponse(SearchPlugin *plugin);
    void removeResponse(QHttpResponse *response);

    QHash<SearchPlugin*, QHttpResponse*> m_hash;
};

#endif // SEARCHSERVER_H
