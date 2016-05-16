/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SERVICEPLUGIN_H
#define SERVICEPLUGIN_H

#include "urlresult.h"
#include <QObject>
#include <QVariantList>

class QNetworkAccessManager;
class QNetworkRequest;

/*!
    \class ServicePlugin
    \brief The base class for service plugins.
*/
class ServicePlugin : public QObject
{
    Q_OBJECT

public:
    explicit ServicePlugin(QObject *parent = 0) :
        QObject(parent)
    {
    }

    /*!
        \brief Creates an instance of the plugin with the specified parent.
    */
    virtual ServicePlugin* createPlugin(QObject *parent = 0) = 0;

    /*!
        \brief Allows the plugin to share an application QNetworkNetworkManager instance.
        
        The base implementation does nothing. If you choose to re-implement this function,
        the plugin should not take ownership of the QNetworkAccessManager instance.
        
        The QNetworkAccessManager is guaranteed to remain valid for the lifetime of the 
        plugin.
    */
    virtual void setNetworkAccessManager(QNetworkAccessManager*) {}
    
public Q_SLOTS:
    /*!
        \brief Cancels the current operation.
        
        Should return \c true if successful.
    */
    virtual bool cancelCurrentOperation() = 0;

    /*!
        \brief Checks if the specified url is valid.
        
        When the check is completed, the urlChecked() signal should be emitted, regardless of the result.
        
        \sa urlChecked
    */
    virtual void checkUrl(const QString &url) = 0;

    /*!
       \brief Retrieves a download request for the specified url.
        
        If the request is retrieved sucessfully, the downloadRequestReady() signal should be emitted. 
        Otherwise, the error() signal should be emitted.
        
        \sa downloadRequestReady, error
    */
    virtual void getDownloadRequest(const QString &url) = 0;

Q_SIGNALS:
    /*!
        \brief This signal should be emitted when a captcha response is required to complete an operation.
        
        The \a callback method will be called when the captcha is completed and should take two strings 
        (the challenge and the response) as arguments.
    */
    void captchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey, const QByteArray &callback);

    /*!
        \brief This signal should be emitted when a download request is successfully retrieved.
    */
    void downloadRequest(const QNetworkRequest &request, const QByteArray &method = QByteArray("GET"),
                         const QByteArray &data = QByteArray());

    /*!
        \brief This signal should be emitted when the plugin is unable to retrieve a download request
        or complete a url check.
    */
    void error(const QString &errorString);

    /*!
        \brief This signal should be emitted when some additional data is required to complete an operation.
                
        The \a callback method will be called when the user submits the settings and should take a QVariantMap
        as its sole argument.
    */
    void settingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);

    /*!
        \brief This signal should be emitted when a url has been successfully checked.
    */
    void urlChecked(const UrlResult &result);

    /*!
        \brief This signal should be emitted when a url has been successfully checked.
    */
    void urlChecked(const UrlResultList &results, const QString &packageName);

    /*!
        \brief This signal should be emitted when a wait is required before a download request can be 
        retrieved.
        
        If isLongDelay is \c true, the application will commence with other downloads. Otherwise, 
        the application will wait for further signals from the plugin.
    */
    void waitRequest(int msecs, bool isLongDelay);
};

Q_DECLARE_INTERFACE(ServicePlugin, "org.qdl2.ServicePlugin")

#endif // SERVICEPLUGIN_H
