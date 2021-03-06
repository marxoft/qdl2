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

#ifndef TRANSFER_H
#define TRANSFER_H

#include "downloadrequester.h"
#include "transferitem.h"
#include <QNetworkRequest>
#include <QTime>
#include <QUrl>

class QFile;
class QNetworkAccessManager;
class QNetworkReply;

class Transfer : public TransferItem
{
    Q_OBJECT

    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand)
    Q_PROPERTY(bool customCommandOverrideEnabled READ customCommandOverrideEnabled
               WRITE setCustomCommandOverrideEnabled)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
    Q_PROPERTY(QString fileSuffix READ fileSuffix)
    Q_PROPERTY(int captchaType READ captchaType)
    Q_PROPERTY(QString captchaTypeString READ captchaTypeString)
    Q_PROPERTY(QByteArray captchaData READ captchaData)
    Q_PROPERTY(QString captchaResponse READ captchaResponse WRITE submitCaptchaResponse)
    Q_PROPERTY(int captchaTimeout READ captchaTimeout)
    Q_PROPERTY(QString captchaTimeoutString READ captchaTimeoutString)
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString pluginIconPath READ pluginIconPath)
    Q_PROPERTY(QString pluginId READ pluginId)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(bool usePlugins READ usePlugins WRITE setUsePlugins)
    Q_PROPERTY(Priority priority READ priority WRITE setPriority)
    Q_PROPERTY(QString priorityString READ priorityString)
    Q_PROPERTY(int progress READ progress)
    Q_PROPERTY(QString progressString READ progressString)
    Q_PROPERTY(qint64 bytesTransferred READ bytesTransferred)
    Q_PROPERTY(qint64 size READ size)
    Q_PROPERTY(int speed READ speed)
    Q_PROPERTY(QString speedString READ speedString)
    Q_PROPERTY(QString postData READ postData WRITE setPostData)
    Q_PROPERTY(QVariantMap requestHeaders READ requestHeaders WRITE setRequestHeaders)
    Q_PROPERTY(QString requestMethod READ requestMethod WRITE setRequestMethod)
    Q_PROPERTY(QVariantList requestedSettings READ requestedSettings)
    Q_PROPERTY(int requestedSettingsTimeout READ requestedSettingsTimeout)
    Q_PROPERTY(QString requestedSettingsTimeoutString READ requestedSettingsTimeoutString)
    Q_PROPERTY(QString requestedSettingsTitle READ requestedSettingsTitle)
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(QString statusString READ statusString)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(int waitTime READ waitTime)
    Q_PROPERTY(QString waitTimeString READ waitTimeString)

public:
    explicit Transfer(QObject *parent = 0);

    virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);

    virtual QMap<int, QVariant> itemData() const;
    Q_INVOKABLE virtual QVariantMap itemDataWithRoleNames() const;

    virtual ItemType itemType() const;

    virtual bool canStart() const;
    virtual bool canPause() const;
    virtual bool canCancel() const;

    QString customCommand() const;
    void setCustomCommand(const QString &c);
    bool customCommandOverrideEnabled() const;
    void setCustomCommandOverrideEnabled(bool enabled);
    
    QString downloadPath() const;
    void setDownloadPath(const QString &p);
    
    QString fileName() const;
    void setFileName(const QString &f);
    QString filePath() const;
    void setFilePath(const QString &fp);
    QString fileSuffix() const;

    int captchaType() const;
    QString captchaTypeString() const;
    QByteArray captchaData() const;
    QString captchaResponse() const;
    int captchaTimeout() const;
    QString captchaTimeoutString() const;
    
    QString iconPath() const;

    QString id() const;
    void setId(const QString &i);
    
    QString pluginIconPath() const;
    QString pluginId() const;
    QString pluginName() const;
    bool usePlugins() const;
    void setUsePlugins(bool enabled);
    
    Priority priority() const;
    void setPriority(Priority p);
    QString priorityString() const;

    int progress() const;
    QString progressString() const;
    qint64 bytesTransferred() const;
    qint64 size() const;
    int speed() const;
    QString speedString() const;
    
    QString postData() const;
    void setPostData(const QString &data);
    
    QVariantMap requestHeaders() const;
    void setRequestHeaders(const QVariantMap &headers);
    QString requestMethod() const;
    void setRequestMethod(const QString &method);

    QVariantList requestedSettings() const;
    int requestedSettingsTimeout() const;
    QString requestedSettingsTimeoutString() const;
    QString requestedSettingsTitle() const;

    Status status() const;
    QString statusString() const;
    QString errorString() const;

    QString url() const;
    void setUrl(const QString &u);

    int waitTime() const;
    QString waitTimeString() const;

public Q_SLOTS:
    virtual bool queue();
    virtual bool start();
    virtual bool pause();
    virtual bool cancel(bool deleteFiles = false);

    virtual void restore(const QSettings &settings);
    virtual void save(QSettings &settings);

    bool submitCaptchaResponse(const QString &response);
    bool submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onDownloadRequest(QNetworkRequest request, const QByteArray &method, const QByteArray &data);
    void onDownloadRequestCaptchaTimeoutChanged();
    void onDownloadRequestRequestedSettingsTimeoutChanged();
    void onDownloadRequestWaitTimeChanged();
    void onDownloadRequestStatusChanged(DownloadRequester::Status s);
    void onDownloadRequestError(const QString &errorString);

    void onReplyMetaDataChanged();
    void onReplyReadyRead();
    void onReplyFinished();

private:
    void setPluginIconPath(const QString &p);
    void setPluginId(const QString &i);
    void setPluginName(const QString &n);
    void updatePluginInfo();
    
    void setSize(qint64 s);
    void setSpeed(int s);

    void setStatus(Status s);
    void setErrorString(const QString &e);

    void cleanup();

    void initNetworkAccessManager();
    void initRequester();

    bool openBuffer(const QByteArray &data);
    bool openFile();

    void startDownload();
    void followRedirect(const QUrl &url);

    void deleteFile();
    
    static const QRegExp CONTENT_DISPOSITION_REGEXP;
    
    DownloadRequester *m_requester;
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply;
    QFile *m_file;

    QString m_customCommand;
    QString m_downloadPath;
    QString m_fileName;
    QString m_id;
    QString m_errorString;

    Priority m_priority;

    qint64 m_lastBytesTransferred;
    qint64 m_size;
    int m_speed;

    QTime m_speedTime;

    Status m_status;

    QString m_postData;
    QString m_requestMethod;
    
    QVariantMap m_requestHeaders;
    
    QString m_url;

    QString m_servicePluginId;
    QString m_servicePluginName;
    QString m_servicePluginIcon;

    bool m_customCommandOverrideEnabled;
    bool m_usePlugins;
    bool m_metadataSet;
    bool m_deleteFiles;

    int m_redirects;
};
    
#endif // TRANSFER_H
