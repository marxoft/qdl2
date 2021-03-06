/*!
 * \file searchplugin.h
 *
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

#ifndef SEARCHPLUGIN_H
#define SEARCHPLUGIN_H

#include "searchresult.h"
#include <QObject>
#include <QVariantList>

class QNetworkAccessManager;

/*!
 * The base class for search plugins.
 */
class SearchPlugin : public QObject
{
    Q_OBJECT

public:
    explicit SearchPlugin(QObject *parent = 0) :
        QObject(parent)
    {
    }

    /*!
     * Allows the plugin to share an application QNetworkNetworkManager instance.
     * 
     * The base implementation does nothing. If you choose to re-implement this function,
     * the plugin should not take ownership of the QNetworkAccessManager instance.
     * 
     * The QNetworkAccessManager is guaranteed to remain valid for the lifetime of the plugin.
     */
    virtual void setNetworkAccessManager(QNetworkAccessManager*) {}
    
public Q_SLOTS:
    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return \c true if the current operation (if any) has been successfully 
     * canceled.
     */
    virtual bool cancelCurrentOperation() = 0;
    
    /*!
     * This method can be re-implemented to fetch more results using the 'nextParams' parameter passed in
     * searchCompleted().
     *
     * The base implementation does nothing.
     *
     * \sa search(), searchCompleted()
     */
    virtual void fetchMore(const QVariantMap &) {}
    
    /*!
     * Pure virtual method.
     * 
     * This method must be re-implemented to perform a search. If the search is sucessful, 
     * the searchCompleted() signal should be emitted. Otherwise, the error() signal should be emitted.
     * 
     * \sa searchCompleted(), error()
     */
    virtual void search(const QVariantMap &settings) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when the plugin is unable to complete a search.
     */
    void error(const QString &errorString);
    
    /*!
     * This signal should be emitted when a search has been successfully completed.
     *
     * \sa search()
     */
    void searchCompleted(const SearchResultList &results);
    
    /*!
     * This signal should be emitted when a search has been successfully completed.
     *
     * The \a nextParams parameter is used to retrieve further results by calling fetchMore().
     *
     * \sa search(), fetchMore()
     */
    void searchCompleted(const SearchResultList &results, const QVariantMap &nextParams);

    /*!
     * This signal should be emitted when some additional data is required to complete an operation.
     *         
     * The \a callback method will be called when the user submits the settings and should take a QVariantMap
     * as its sole argument.
     */
    void settingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);    
};

/*!
 * Interface for creating instances of SearchPlugin.
 *
 * \sa SearchPlugin
 */
class SearchPluginFactory
{

public:
    virtual ~SearchPluginFactory() {}

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return an instance of SearchPlugin with the parent set to \a parent.
     */
    virtual SearchPlugin* createPlugin(QObject *parent = 0) = 0;
};

Q_DECLARE_INTERFACE(SearchPluginFactory, "org.qdl2.SearchPluginFactory")

#endif // SEARCHPLUGIN_H
