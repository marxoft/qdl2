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

#include "urlcheckmodel.h"
#include "definitions.h"
#include "logger.h"
#include "servicepluginmanager.h"
#include "transfermodel.h"
#include "utils.h"
#include <QIcon>
#include <QTimer>

UrlCheckModel* UrlCheckModel::self = 0;

UrlCheckModel::UrlCheckModel() :
    QAbstractListModel(),
    m_timer(0),
    m_requestedSettingsTimeout(0),
    m_status(Idle),
    m_index(-1)
{
    m_roles[UrlRole] = "url";
    m_roles[FileNameRole] = "fileName";
    m_roles[IsCheckedRole] = "checked";
    m_roles[IsOkRole] = "ok";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
}

UrlCheckModel::~UrlCheckModel() {
    self = 0;
}

UrlCheckModel* UrlCheckModel::instance() {
    return self ? self : self = new UrlCheckModel;
}

int UrlCheckModel::progress() const {
    return (!m_items.isEmpty()) && (m_index > 0) ? m_index * 100 / m_items.size() : 0;
}

QVariantList UrlCheckModel::requestedSettings() const {
    return m_requestedSettings;
}

void UrlCheckModel::setRequestedSettings(const QString &title, const QVariantList &settings, const QByteArray &callback) {
    m_requestedSettingsTitle = title;
    m_requestedSettings = settings;
    m_requestedSettingsCallback = callback;
}

void UrlCheckModel::clearRequestedSettings() {
    if (!m_requestedSettings.isEmpty()) {
        m_requestedSettingsTitle.clear();
        m_requestedSettings.clear();
        m_requestedSettingsCallback.clear();
        m_requestedSettingsTimeout = 0;
    }
}

int UrlCheckModel::requestedSettingsTimeout() const {
    return m_requestedSettingsTimeout;
}

QString UrlCheckModel::requestedSettingsTimeoutString() const {
    return Utils::formatMSecs(requestedSettingsTimeout());
}

QString UrlCheckModel::requestedSettingsTitle() const {
    return m_requestedSettingsTitle;
}

void UrlCheckModel::startRequestedSettingsTimer() {
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(updateRequestedSettingsTimeout()));
    }
    
    m_requestedSettingsTimeout = CAPTCHA_TIMEOUT;
    m_timer->start();
}

void UrlCheckModel::stopRequestedSettingsTimer() {
    if (m_timer) {
        m_timer->stop();
    }
}

void UrlCheckModel::updateRequestedSettingsTimeout() {
    switch (status()) {
    case AwaitingSettingsResponse:
        m_requestedSettingsTimeout -= m_timer->interval();
        emit requestedSettingsTimeoutChanged(m_requestedSettingsTimeout);
        
        if (m_requestedSettingsTimeout <= 0) {
            stopRequestedSettingsTimer();
            submitSettingsResponse(QVariantMap());
        }
        
        break;
    default:
        stopRequestedSettingsTimer();
        break;
    }
}

UrlCheckModel::Status UrlCheckModel::status() const {
    return m_status;
}

void UrlCheckModel::setStatus(UrlCheckModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
        
        switch (s) {
        case Idle:
        case Completed:
        case Canceled:
            m_index = -1;
            break;
        default:
            break;
        }
    }
}

QString UrlCheckModel::statusString() const {
    switch (status()) {
    case Active:
    case AwaitingSettingsResponse:
        return tr("Checking URLs");
    case Completed:
        return tr("Completed");
    case Canceled:
        return tr("Canceled");
    default:
        return QString();
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> UrlCheckModel::roleNames() const {
    return m_roles;
}
#endif

int UrlCheckModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int UrlCheckModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant UrlCheckModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("URL");
    case 1:
        return tr("Ok?");
    default:
        return QVariant();
    }
}

QVariant UrlCheckModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_items.at(index.row()).url;
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case 1:
            if (m_items.at(index.row()).checked) {
                if (m_items.at(index.row()).ok) {
                    return QIcon::fromTheme("widgets_tickmark_list");
                }
                
                return QIcon::fromTheme("general_stop");
            }

            return QVariant();
        default:
            return QVariant();
        }
    case UrlRole:
        return m_items.at(index.row()).url;
    case FileNameRole:
        return m_items.at(index.row()).fileName;
    case IsCheckedRole:
        return m_items.at(index.row()).checked;
    case IsOkRole:
        return m_items.at(index.row()).ok;
    default:
        return QVariant();
    }
}

QMap<int, QVariant> UrlCheckModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant UrlCheckModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap UrlCheckModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList UrlCheckModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                      Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int UrlCheckModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void UrlCheckModel::append(const QString &url) {
    Logger::log("UrlCheckModel::append(): " + url, Logger::LowVerbosity);
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items << UrlCheck(url);
    endInsertRows();
    emit countChanged(rowCount());
    emit progressChanged(progress());

    if (status() != Active) {
        next();
    }
}

void UrlCheckModel::append(const QStringList &urls) {
    foreach (const QString &url, urls) {
        append(url);
    }
}

bool UrlCheckModel::remove(int row) {
    if ((row > m_index) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
        emit countChanged(rowCount());
        emit progressChanged(progress());
        return true;
    }
    
    return false;
}

void UrlCheckModel::cancel() {
    if (ServicePlugin *plugin = getCurrentPlugin()) {
        plugin->cancelCurrentOperation();
        disconnect(plugin, 0, this, 0);
    }

    setStatus(Canceled);
}

void UrlCheckModel::clear() {
    if (!m_items.isEmpty()) {
        cancel();
        setStatus(Idle);
        m_index = -1;
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

bool UrlCheckModel::submitSettingsResponse(const QVariantMap &settings) {
    if (status() != AwaitingSettingsResponse) {
        return false;
    }

    if (settings.isEmpty()) {
        onUrlCheckError(tr("No settings response"));
        return false;
    }

    ServicePlugin *plugin = getCurrentPlugin();

    if (!plugin) {
        onUrlCheckError(tr("No plugin found"));
        return false;
    }

    if (!QMetaObject::invokeMethod(plugin, m_requestedSettingsCallback, Q_ARG(QVariantMap, settings))) {
        stopRequestedSettingsTimer();
        clearRequestedSettings();
        onUrlCheckError(tr("Invalid settings callback method"));
        return false;
    }

    return true;
}

ServicePlugin* UrlCheckModel::getCurrentPlugin() const {
    return ServicePluginManager::instance()->getPluginByUrl(data(index(m_index, 0), UrlRole).toString());
}

void UrlCheckModel::next() {
    m_index++;
    emit progressChanged(progress());

    if (m_index >= m_items.size()) {
        setStatus(Completed);
        return;
    }

    setStatus(Active);
    const QModelIndex idx = index(m_index, 0);
    const QString url = data(idx, UrlRole).toString();

    if (ServicePlugin *plugin = getCurrentPlugin()) {
        Logger::log("UrlCheckModel::next(). Checking URL: " + url, Logger::MediumVerbosity);
        connect(plugin, SIGNAL(urlChecked(UrlResult)), this, SLOT(onUrlChecked(UrlResult)));
        connect(plugin, SIGNAL(urlChecked(UrlResultList, QString)), this, SLOT(onUrlChecked(UrlResultList, QString)));
        connect(plugin, SIGNAL(error(QString)), this, SLOT(onUrlCheckError(QString)));
        connect(plugin, SIGNAL(settingsRequest(QString, QVariantList, QByteArray)),
                this, SLOT(onUrlCheckSettingsRequest(QString, QVariantList, QByteArray)));
        plugin->checkUrl(url);
        return;
    }

    onUrlCheckError(tr("No plugin found"));
}

void UrlCheckModel::onUrlChecked(const UrlResult &result) {
    if (QObject *plugin = sender()) {
        disconnect(plugin, 0, this, 0);
    }
    
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1 1 URL found").arg(m_items[m_index].url),
                Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = true;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    TransferModel::instance()->append(result);
    next();
}

void UrlCheckModel::onUrlChecked(const UrlResultList &results, const QString &packageName) {
    if (QObject *plugin = sender()) {
        disconnect(plugin, 0, this, 0);
    }
    
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlChecked(): %1. %2 URLs found")
                .arg(m_items[m_index].url).arg(results.size()), Logger::MediumVerbosity);
    m_items[m_index].checked = true;
    m_items[m_index].ok = !results.isEmpty();
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);

    if (!results.isEmpty()) {
        TransferModel::instance()->append(results, packageName);
    }

    next();
}

void UrlCheckModel::onUrlCheckError(const QString &errorString) {
    if (QObject *plugin = sender()) {
        disconnect(plugin, 0, this, 0);
    }
    
    if ((m_index < 0) || (m_index >= m_items.size())) {
        return;
    }

    Logger::log(QString("UrlCheckModel::onUrlCheckError(): %1. Error: %2").arg(m_items[m_index].url)
                                                                          .arg(errorString));
    m_items[m_index].checked = true;
    m_items[m_index].ok = false;
    const QModelIndex idx = index(m_index, 1);
    emit dataChanged(idx, idx);
    next();
}

void UrlCheckModel::onUrlCheckSettingsRequest(const QString &title, const QVariantList &settings,
                                              const QByteArray &callback) {
    Logger::log("UrlCheckModel::onUrlCheckSettingsRequest()", Logger::MediumVerbosity);
    setRequestedSettings(title, settings, callback);
    startRequestedSettingsTimer();
    setStatus(AwaitingSettingsResponse);
    emit settingsRequest(title, settings);
}
