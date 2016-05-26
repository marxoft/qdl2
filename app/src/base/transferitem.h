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

#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include <QObject>
#include <QPersistentModelIndex>

class QSettings;
class QVariant;

class TransferItem : public QObject
{

    Q_OBJECT

    Q_PROPERTY(ItemType itemType READ itemType)
    Q_PROPERTY(bool canStart READ canStart)
    Q_PROPERTY(bool canPause READ canPause)
    Q_PROPERTY(bool canCancel READ canCancel)
    Q_PROPERTY(int row READ row)
    Q_PROPERTY(int count READ rowCount)

    Q_ENUMS(ItemType Priority Roles Status)
    
public:
    enum ItemType {
        ListType = 0,
        PackageType,
        TransferType
    };

    enum Priority {
        HighestPriority = 0,
        HighPriority,
        NormalPriority,
        LowPriority,
        LowestPriority
    };
    
    enum Roles {
        BytesTransferredRole = Qt::UserRole + 1,
        CanCancelRole,
        CanPauseRole,
        CanStartRole,
        CaptchaImageRole,
        CaptchaTimeoutRole,
        CaptchaTimeoutStringRole,
        CategoryRole,
        CreateSubfolderRole,
        CustomCommandRole,
        DownloadPathRole,
        ErrorStringRole,
        FileNameRole,
        FilePathRole,
        IdRole,
        ItemTypeRole,
        ItemTypeStringRole,
        NameRole,
        PluginIconPathRole,
        PluginIdRole,
        PluginNameRole,
        PriorityRole,
        PriorityStringRole,
        ProgressRole,
        ProgressStringRole,
        RequestedSettingsRole,
        RequestedSettingsTimeoutRole,
        RequestedSettingsTimeoutStringRole,
        RequestedSettingsTitleRole,
        RowRole,
        RowCountRole,
        SizeRole,
        SpeedRole,
        SpeedStringRole,
        StatusRole,
        StatusStringRole,
        SuffixRole,
        UrlRole,
        WaitTimeRole,
        WaitTimeStringRole
    };

    enum Status {
        // Inactive
        Null = 0,
        Paused,
        Canceled,
        CanceledAndDeleted,
        Failed,
        Completed,
        WaitingInactive,
        // Active
        Queued,
        Connecting,
        WaitingActive,
        RetrievingCaptchaChallenge,
        AwaitingCaptchaResponse,
        RetrievingCaptchaResponse,
        SubmittingCaptchaResponse,
        ReportingCaptchaResponse,
        AwaitingDecaptchaSettingsResponse,
        AwaitingRecaptchaSettingsResponse,
        AwaitingServiceSettingsResponse,
        SubmittingDecaptchaSettingsResponse,
        SubmittingRecaptchaSettingsResponse,
        SubmittingServiceSettingsResponse,
        Downloading,
        Canceling,
        ExtractingArchive,
        MovingFiles,
        ExecutingCustomCommand
    };

    explicit TransferItem(QObject *parent = 0);

    static QHash<int, QByteArray> roleNames();

    static QString priorityString(Priority priority);
    static QString statusString(Status status);
    
    virtual QVariant data(int role) const;
    Q_INVOKABLE virtual QVariant data(const QByteArray &roleName) const;
    virtual bool setData(int role, const QVariant &value);
    Q_INVOKABLE virtual bool setData(const QByteArray &roleName, const QVariant &value);

    virtual QMap<int, QVariant> itemData() const;
    Q_INVOKABLE virtual QVariantMap itemDataWithRoleNames() const;
    virtual bool setItemData(const QMap<int, QVariant> &data);
    Q_INVOKABLE virtual bool setItemData(const QVariantMap &data);

    virtual ItemType itemType() const;
    QString itemTypeString() const;

    virtual bool canStart() const;
    virtual bool canPause() const;
    virtual bool canCancel() const;

    int row() const;
    int rowCount() const;

    TransferItem *parentItem() const;

    TransferItem* childItem(int i) const;
    void appendRow(TransferItem *item);
    bool insertRow(int i, TransferItem *item);
    bool moveRow(int from, int to);
    bool removeRow(int i);
    TransferItem* takeRow(int i);

public Q_SLOTS:
    virtual bool queue();
    virtual bool start();
    virtual bool pause();
    virtual bool cancel(bool deleteFiles = false);

    virtual void restore(const QSettings &settings);
    virtual void save(QSettings &settings);

protected Q_SLOTS:
    virtual void childItemFinished(TransferItem *item);
    
Q_SIGNALS:
    void dataChanged(TransferItem *item, int role);
    void finished(TransferItem *item);

private:
    void connectItemSignals(TransferItem *item);
    void disconnectItemSignals(TransferItem *item);

protected:
    void setRow(int r);
    
    void setParentItem(TransferItem *item);
    
    static QHash<int, QByteArray> roles;

    int m_row;

    TransferItem *m_parentItem;

    QList<TransferItem*> m_childItems;
};

#endif // TRANSFERITEM_H
